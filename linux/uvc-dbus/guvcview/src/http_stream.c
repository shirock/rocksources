/*******************************************************************************
#                                                                              #
#      MJPG-streamer allows to stream JPG frames from an input-plugin          #
#      to several output plugins                                               #
#                                                                              #
#      Copyright (C) 2007 busybox-project (base64 function)                    #
#      Copyright (C) 2007 Tom Stöveken                                         #
#                                                                              #
# This program is free software; you can redistribute it and/or modify         #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation; version 2 of the License.                      #
#                                                                              #
# This program is distributed in the hope that it will be useful,              #
# but WITHOUT ANY WARRANTY; without even the implied warranty of               #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                #
# GNU General Public License for more details.                                 #
#                                                                              #
# You should have received a copy of the GNU General Public License            #
# along with this program; if not, write to the Free Software                  #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA    #
#                                                                              #
*******************************************************************************/
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <limits.h>

#include <linux/version.h>
#include <linux/types.h>          /* for videodev2.h */
#include <linux/videodev2.h>

#define ALLOW_MULTI_STREAMING_CLIENT  (0)
// 多個客戶端開啟串流時，串流內容將明顯延遲。故預設只允許一個客戶端開啟串流。
#if ALLOW_MULTI_STREAMING_CLIENT
#include <pthread.h>
#endif

#include "defs.h"
#include "guvcview.h"
#include "jpgenc.h"

#include "http_stream.h"
#define IO_BUFFER 256
#define BUFFER_SIZE 1024

/* the boundary is used for the M-JPEG stream, it separates the multipart stream of pictures */
#define BOUNDARY "boundarydonotcross"

/*
 * Standard header to be send along with other header information like mimetype.
 *
 * The parameters should ensure the browser does not cache our answer.
 * A browser should connect for each file and not serve files from his cache.
 * Using cached pictures would lead to showing old/outdated pictures
 * Many browser seem to ignore, or at least not always obey those headers
 * since i observed caching of files from time to time.
 */
#define STD_HEADER "Connection: close\r\n" \
    "Server: MJPG-Streamer/0.2\r\n" \
    "Cache-Control: no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0\r\n" \
    "Pragma: no-cache\r\n" \
    "Expires: Mon, 3 Jan 2000 12:34:56 GMT\r\n"

/* the webserver determines between these values for an answer */
typedef enum {
    A_UNKNOWN,
    A_SNAPSHOT,
    A_SNAPSHOT_WXP,
    A_STREAM,
    A_STREAM_WXP,
    A_COMMAND,
    A_FILE,
    A_CGI,
    A_TAKE,
    A_INPUT_JSON,
    A_OUTPUT_JSON,
    A_PROGRAM_JSON,
} answer_t;

/*
 * the client sends information with each request
 * this structure is used to store the important parts
 */
typedef struct {
    answer_t type;
    char *parameter;
    char *client;
    char *credentials;
    char *query_string;
} request;

/* the iobuffer structure is used to read from the HTTP-client */
typedef struct {
    int level;              /* how full is the buffer */
    char buffer[IO_BUFFER]; /* the data */
} iobuffer;

/*
 * this struct is just defined to allow passing all necessary details to a worker thread
 * "cfd" is for connected/accepted filedescriptor
 */
typedef struct {
    int fd;
    struct ALL_DATA *all_data;
} cfd;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
#define V4L2_CTRL_TYPE_STRING_SUPPORTED
#endif

#define DBG     printf

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define LENGTH_OF(x) (sizeof(x)/sizeof(x[0]))

/******************************************************************************
Description.: initializes the iobuffer structure properly
Input Value.: pointer to already allocated iobuffer
Return Value: iobuf
******************************************************************************/
static void init_iobuffer(iobuffer *iobuf)
{
    memset(iobuf->buffer, 0, sizeof(iobuf->buffer));
    iobuf->level = 0;
}

/******************************************************************************
Description.: initializes the request structure properly
Input Value.: pointer to already allocated req
Return Value: req
******************************************************************************/
static void init_request(request *req)
{
    req->type        = A_UNKNOWN;
    req->parameter   = NULL;
    req->client      = NULL;
    req->credentials = NULL;
}

/******************************************************************************
Description.: If strings were assigned to the different members free them
              This will fail if strings are static, so always use strdup().
Input Value.: req: pointer to request structure
Return Value: -
******************************************************************************/
static void free_request(request *req)
{
    if(req->parameter != NULL) free(req->parameter);
    if(req->client != NULL) free(req->client);
    if(req->credentials != NULL) free(req->credentials);
    if(req->query_string != NULL) free(req->query_string);
}

/******************************************************************************
Description.: read with timeout, implemented without using signals
              tries to read len bytes and returns if enough bytes were read
              or the timeout was triggered. In case of timeout the return
              value may differ from the requested bytes "len".
Input Value.: * fd.....: fildescriptor to read from
              * iobuf..: iobuffer that allows to use this functions from multiple
                         threads because the complete context is the iobuffer.
              * buffer.: The buffer to store values at, will be set to zero
                         before storing values.
              * len....: the length of buffer
              * timeout: seconds to wait for an answer
Return Value: * buffer.: will become filled with bytes read
              * iobuf..: May get altered to save the context for future calls.
              * func().: bytes copied to buffer or -1 in case of error
******************************************************************************/
static int _read(int fd, iobuffer *iobuf, void *buffer, size_t len, int timeout)
{
    int copied = 0, rc, i;
    fd_set fds;
    struct timeval tv;

    memset(buffer, 0, len);

    while((copied < len)) {
        i = MIN(iobuf->level, len - copied);
        memcpy(buffer + copied, iobuf->buffer + IO_BUFFER - iobuf->level, i);

        iobuf->level -= i;
        copied += i;
        if(copied >= len)
            return copied;

        /* select will return in case of timeout or new data arrived */
        tv.tv_sec = timeout;
        tv.tv_usec = 0;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        if((rc = select(fd + 1, &fds, NULL, NULL, &tv)) <= 0) {
            if(rc < 0)
                exit(EXIT_FAILURE);

            /* this must be a timeout */
            return copied;
        }

        init_iobuffer(iobuf);

        /*
         * there should be at least one byte, because select signalled it.
         * But: It may happen (very seldomly), that the socket gets closed remotly between
         * the select() and the following read. That is the reason for not relying
         * on reading at least one byte.
         */
        if((iobuf->level = read(fd, &iobuf->buffer, IO_BUFFER)) <= 0) {
            /* an error occured */
            return -1;
        }

        /* align data to the end of the buffer if less than IO_BUFFER bytes were read */
        memmove(iobuf->buffer + (IO_BUFFER - iobuf->level), iobuf->buffer, iobuf->level);
    }

    return 0;
}

/******************************************************************************
Description.: Read a single line from the provided fildescriptor.
              This funtion will return under two conditions:
              * line end was reached
              * timeout occured
Input Value.: * fd.....: fildescriptor to read from
              * iobuf..: iobuffer that allows to use this functions from multiple
                         threads because the complete context is the iobuffer.
              * buffer.: The buffer to store values at, will be set to zero
                         before storing values.
              * len....: the length of buffer
              * timeout: seconds to wait for an answer
Return Value: * buffer.: will become filled with bytes read
              * iobuf..: May get altered to save the context for future calls.
              * func().: bytes copied to buffer or -1 in case of error
******************************************************************************/
/* read just a single line or timeout */
static int _readline(int fd, iobuffer *iobuf, void *buffer, size_t len, int timeout)
{
    char c = '\0', *out = buffer;
    int i;

    memset(buffer, 0, len);

    for(i = 0; i < len && c != '\n'; i++) {
        if(_read(fd, iobuf, &c, 1, timeout) <= 0) {
            /* timeout or error occured */
            return -1;
        }
        *out++ = c;
    }

    return i;
}

/******************************************************************************
Description.: Decodes the data and stores the result to the same buffer.
              The buffer will be large enough, because base64 requires more
              space then plain text.
Hints.......: taken from busybox, but it is GPL code
Input Value.: base64 encoded data
Return Value: plain decoded data
******************************************************************************/
static void decodeBase64(char *data)
{
    const unsigned char *in = (const unsigned char *)data;
    /* The decoded size will be at most 3/4 the size of the encoded */
    unsigned ch = 0;
    int i = 0;

    while(*in) {
        int t = *in++;

        if(t >= '0' && t <= '9')
            t = t - '0' + 52;
        else if(t >= 'A' && t <= 'Z')
            t = t - 'A';
        else if(t >= 'a' && t <= 'z')
            t = t - 'a' + 26;
        else if(t == '+')
            t = 62;
        else if(t == '/')
            t = 63;
        else if(t == '=')
            t = 0;
        else
            continue;

        ch = (ch << 6) | t;
        i++;
        if(i == 4) {
            *data++ = (char)(ch >> 16);
            *data++ = (char)(ch >> 8);
            *data++ = (char) ch;
            i = 0;
        }
    }
    *data = '\0';
}

/******************************************************************************
Description.: Send a complete HTTP response and a stream of JPG-frames.
Input Value.: fildescriptor fd to send the answer to
Return Value: -
******************************************************************************/
static void send_stream(cfd *context_fd, int input_number)
{
    unsigned char *frame = NULL;
    int frame_size = 0;
    char buffer[BUFFER_SIZE] = {0};
    struct timeval timestamp;

    struct ALL_DATA *all_data = context_fd->all_data;
    struct GLOBAL *global = all_data->global;
    struct vdIn *videoIn = all_data->videoIn;
    struct JPEG_ENCODER_STRUCTURE *jpeg_struct = NULL;

    sprintf(buffer, "HTTP/1.0 200 OK\r\n" \
            "Access-Control-Allow-Origin: *\r\n" \
            STD_HEADER \
            "Content-Type: multipart/x-mixed-replace;boundary=" BOUNDARY "\r\n" \
            "\r\n" \
            "--" BOUNDARY "\r\n");

    if(write(context_fd->fd, buffer, strlen(buffer)) < 0) {
        return;
    }

//     DBG("Headers send, sending stream now\n");

    /* Initialization of JPEG control structure */
    jpeg_struct = g_new0(struct JPEG_ENCODER_STRUCTURE, 1);
    initialization (jpeg_struct,global->width,global->height);
    initialize_quantization_tables (jpeg_struct);

    while (videoIn->streaming) {

        // __LOCK_MUTEX(__GMUTEX); // FIXME is needed?

        frame = g_new0(BYTE, ((global->width)*(global->height))>>1);
        frame_size = encode_image(videoIn->snapshot, frame,
            jpeg_struct, 1, global->width, global->height);

        // __UNLOCK_MUTEX(__GMUTEX); // FIXME is needed?
        /* allow others to access the global buffer again */

        sprintf(buffer, "Content-Type: image/jpeg\r\n" \
                "Content-Length: %d\r\n" \
                "X-Timestamp: %d.%06d\r\n" \
                "\r\n", frame_size, (int)timestamp.tv_sec, (int)timestamp.tv_usec);
        if(write(context_fd->fd, buffer, strlen(buffer)) < 0) break;

        if(write(context_fd->fd, frame, frame_size) < 0) break;

        sprintf(buffer, "\r\n--" BOUNDARY "\r\n");
        if(write(context_fd->fd, buffer, strlen(buffer)) < 0) break;

        free(frame);
    }

    g_free(jpeg_struct);
}



/******************************************************************************
Description.: Send error messages and headers.
Input Value.: * fd.....: is the filedescriptor to send the message to
              * which..: HTTP error code, most popular is 404
              * message: append this string to the displayed response
Return Value: -
******************************************************************************/
static void send_error(int fd, int which, char *message)
{
    char buffer[BUFFER_SIZE] = {0};

    if(which == 401) {
        sprintf(buffer, "HTTP/1.0 401 Unauthorized\r\n" \
                "Content-type: text/plain\r\n" \
                STD_HEADER \
                "WWW-Authenticate: Basic realm=\"MJPG-Streamer\"\r\n" \
                "\r\n" \
                "401: Not Authenticated!\r\n" \
                "%s", message);
    } else if(which == 404) {
        sprintf(buffer, "HTTP/1.0 404 Not Found\r\n" \
                "Content-type: text/plain\r\n" \
                STD_HEADER \
                "\r\n" \
                "404: Not Found!\r\n" \
                "%s", message);
    } else if(which == 500) {
        sprintf(buffer, "HTTP/1.0 500 Internal Server Error\r\n" \
                "Content-type: text/plain\r\n" \
                STD_HEADER \
                "\r\n" \
                "500: Internal Server Error!\r\n" \
                "%s", message);
    } else if(which == 400) {
        sprintf(buffer, "HTTP/1.0 400 Bad Request\r\n" \
                "Content-type: text/plain\r\n" \
                STD_HEADER \
                "\r\n" \
                "400: Not Found!\r\n" \
                "%s", message);
    } else if (which == 403) {
        sprintf(buffer, "HTTP/1.0 403 Forbidden\r\n" \
                "Content-type: text/plain\r\n" \
                STD_HEADER \
                "\r\n" \
                "403: Forbidden!\r\n" \
                "%s", message);
    } else {
        sprintf(buffer, "HTTP/1.0 501 Not Implemented\r\n" \
                "Content-type: text/plain\r\n" \
                STD_HEADER \
                "\r\n" \
                "501: Not Implemented!\r\n" \
                "%s", message);
    }

    if(write(fd, buffer, strlen(buffer)) < 0) {
        DBG("write failed, done anyway\n");
    }
}


/******************************************************************************
Description.: Serve a connected TCP-client. This thread function is called
              for each connect of a HTTP client like a webbrowser. It determines
              if it is a valid HTTP request and dispatches between the different
              response options.
Input Value.: arg is the filedescriptor and server-context of the connected TCP
              socket. It must have been allocated so it is freeable by this
              thread function.
Return Value: always NULL
******************************************************************************/
/* thread for clients that connected to this server */
static void *client_thread(void *arg)
{

    int cnt;
    int input_number = 0;
    char buffer[BUFFER_SIZE] = {0};
    iobuffer iobuf;
    request req;
    cfd lcfd; /* local-connected-file-descriptor */
    memcpy(&lcfd, arg, sizeof(cfd));
    /* we really need the fildescriptor and it must be freeable by us */

    struct ALL_DATA *all_data = lcfd.all_data;
    struct GLOBAL *global = all_data->global;

    /* initializes the structures */
    init_iobuffer(&iobuf);
    init_request(&req);

    /* What does the client want to receive? Read the request. */
    memset(buffer, 0, sizeof(buffer));
    if((cnt = _readline(lcfd.fd, &iobuf, buffer, sizeof(buffer) - 1, 5)) == -1) {
        close(lcfd.fd);
        return NULL;
    }

    req.query_string = NULL;
    req.type = A_STREAM;

    /*
     * parse the rest of the HTTP-request
     * the end of the request-header is marked by a single, empty line with "\r\n"
     */
    do {
        memset(buffer, 0, sizeof(buffer));

        if((cnt = _readline(lcfd.fd, &iobuf, buffer, sizeof(buffer) - 1, 5)) == -1) {
            free_request(&req);
            close(lcfd.fd);
            return NULL;
        }

        if(strcasestr(buffer, "User-Agent: ") != NULL) {
            req.client = strdup(buffer + strlen("User-Agent: "));
        } else if(strcasestr(buffer, "Authorization: Basic ") != NULL) {
            req.credentials = strdup(buffer + strlen("Authorization: Basic "));
            decodeBase64(req.credentials);
            DBG("username:password: %s\n", req.credentials);
        }

    } while(cnt > 2 && !(buffer[0] == '\r' && buffer[1] == '\n'));

    /* check for username and password if parameter was given */
    if (global->streamingName != NULL &&
        global->streamingPassword != NULL)
    {
        char *credentials = NULL;
        credentials = g_strdup_printf("%s:%s", global->streamingName, global->streamingPassword);
        if(req.credentials == NULL || strcmp(credentials, req.credentials) != 0) {
            DBG("access denied\n");
            send_error(lcfd.fd, 401, "username and password do not match to configuration");
            close(lcfd.fd);
            free_request(&req);
            free(credentials);
            return NULL;
        }
        DBG("access granted\n");
    }

    switch(req.type) {
    case A_STREAM:
        DBG("Request for stream from input: %d\n", input_number);
        send_stream(&lcfd, input_number);
        break;
    default:
        DBG("unknown request\n");
    }

    close(lcfd.fd);
    free_request(&req);

    DBG("leaving HTTP streaming client thread\n");
    return NULL;
}

/******************************************************************************
Description.: Open a TCP socket and wait for clients to connect. If clients
              connect, start a new thread for each accepted connection.
Input Value.: arg is a pointer to the globals struct
Return Value: always NULL, will only return on exit
******************************************************************************/
// http://127.0.0.1:8080/?action=stream
void *http_server_thread(void *data)
{
    struct ALL_DATA *all_data = (struct ALL_DATA *) data;
    struct GLOBAL *global = all_data->global;
    struct vdIn *videoIn = all_data->videoIn;

    int on;
    struct addrinfo *aip;
    struct addrinfo hints;
    struct sockaddr_storage client_addr;
    socklen_t addr_len = sizeof(struct sockaddr_storage);
    fd_set selectfds;
    int max_fds = 0;
    char name[NI_MAXHOST];
    int err;
#if ALLOW_MULTI_STREAMING_CLIENT
    pthread_t client;
#endif

    int sd = -1;

    struct timeval select_tv;

    bzero(&hints, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;

    if((err = getaddrinfo(NULL, global->streamingPort, &hints, &aip)) != 0) {
        perror(gai_strerror(err));
        videoIn->streaming = FALSE;
        return NULL;
    }

    /* open sockets for server (1 socket / address family) */
    sd = socket(aip->ai_family, aip->ai_socktype, 0);
    if (sd < 0) {
        perror("open socket for server failed\n");
        videoIn->streaming = FALSE;
        return NULL;
    }

    /* ignore "socket already in use" errors */
    on = 1;
    if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed\n");
    }

    /* IPv6 socket should listen to IPv6 only, otherwise we will get "socket already in use" */
    on = 1;
    if(aip->ai_family == AF_INET6 && setsockopt(sd, IPPROTO_IPV6, IPV6_V6ONLY,
            (const void *)&on , sizeof(on)) < 0) {
        perror("setsockopt(IPV6_V6ONLY) failed\n");
    }

    /* setsockopt(sd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on)); */

    if(bind(sd, aip->ai_addr, aip->ai_addrlen) < 0) {
        perror("bind");
        videoIn->streaming = FALSE;
        close(sd);
        return NULL;
    }

    if(listen(sd, 3) < 0) {
        perror("listen");
        videoIn->streaming = FALSE;
        close(sd);
        return NULL;
    }

    max_fds = sd;
    /* create a child for every client that connects */
    while (videoIn->streaming) {
        cfd *pcfd = malloc(sizeof(cfd));

        if(pcfd == NULL) {
            fprintf(stderr, "failed to allocate (a very small amount of) memory\n");
            exit(EXIT_FAILURE);
        }

        DBG("waiting for clients to connect. listen port %s. \n", global->streamingPort);

        do {
            FD_ZERO(&selectfds);
            FD_SET(sd, &selectfds);

            select_tv.tv_sec = 1L;
            select_tv.tv_usec = 0L;
            //err = select(max_fds + 1, &selectfds, NULL, NULL, NULL);
            err = select(max_fds + 1, &selectfds, NULL, NULL, &select_tv);

            if(err < 0 && errno != EINTR) {
                perror("select");
                videoIn->streaming = FALSE;
            }
            if (videoIn->streaming == FALSE) {
                break;
            }
        } while(err <= 0);

        if(FD_ISSET(sd, &selectfds)) {
            pcfd->fd = accept(sd, (struct sockaddr *)&client_addr, &addr_len);
            pcfd->all_data = all_data;

            if(getnameinfo((struct sockaddr *)&client_addr, addr_len, name, sizeof(name), NULL, 0, NI_NUMERICHOST) == 0) {
                DBG("serving client: %s\n", name);
            }

#if ALLOW_MULTI_STREAMING_CLIENT
            if(pthread_create(&client, NULL, &client_thread, pcfd) != 0) {
                DBG("could not launch another client thread\n");
                close(pcfd->fd);
                free(pcfd);
                continue;
            }
            pthread_detach(client);
#else
            client_thread(pcfd);
            close(pcfd->fd);
            free(pcfd);
#endif
        }
        //printf("loop...\n");
    }

    close(sd);
    DBG("leaving HTTP streaming server thread\n");
    return NULL;
}
