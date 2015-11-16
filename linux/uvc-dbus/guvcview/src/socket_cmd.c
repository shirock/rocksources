/*
Copyright 2015 Transtep, Inc.
*/
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <errno.h>
#include <glib.h>
#include "globals.h"
#include "guvcview.h"

#define SOCKET_DIR  "/tmp"

/**
@param out_argc store number of tokens in returned array.
@return a newly-allocated NULL-terminated array of strings. Use g_strfreev() to free it. It will be NULL if empty.
*/
static char **
split_string_to_argv(const char*string, int *out_argc)
{
    if (!string || *string == '\0') {
        if (out_argc)
            *out_argc = 0;
        return NULL;
    }

    char *data = g_strstrip(g_strdup(string));

    if (*data == '\0') { // empty?
        if (out_argc)
            *out_argc = 0;
        return NULL;
    }

    char **argvt;
    char **pstrt, **pstrl;

    argvt = g_strsplit(data, " ", -1);

    for (pstrt = argvt, pstrl = NULL;
        *pstrt != NULL;
        ++pstrt)
    {
        if (**pstrt == '\0') { // empty string.
            if (pstrl == NULL)
                pstrl = pstrt;
            g_free(*pstrt);
        }
        else if (pstrl != NULL) {
            *pstrl = *pstrt;
            *pstrt = NULL;
            ++pstrl;
        }
    }

    g_free(data);

    if (out_argc)
        *out_argc = g_strv_length(argvt);
    return argvt;
}


static ssize_t
response(int fd, const char *fmt, ...)
{
    va_list aplist;
    int msg_len;
    char *buf;

    va_start(aplist, fmt);
    buf = g_strdup_vprintf(fmt, aplist);
    msg_len = strlen(buf);
    va_end(aplist);

    buf[msg_len] = '\n'; // note, it set \0 to \n. so buf will not be a zero-terminated string.
    ++msg_len;

    ssize_t rc = write(fd, buf, msg_len);

    g_free(buf);
    return rc;
}

/*
response
--------
+OK\n
--------
    */
static void
response_ok(int fd)
{
    response(fd, "+OK");
}

/*
response
--------
-ERR error_message\n
--------
    */
static void
response_error(int fd, const char *fmt, ...)
{
    va_list aplist;
    char *err_fmt = g_strconcat("-ERR ", fmt, NULL);

    va_start(aplist, fmt);
    response(fd, err_fmt, aplist);
    va_end(aplist);

    g_free(err_fmt);
}

/**
response
--------
<bytes_of_data>\n
<data>\n
+OK\n
--------
notice: bytes_of_data 不包含 data 最後的 \n 。
 */
static void
response_data(int fd, const char *data, int data_len)
{
    response(fd, "%d", data_len);
    write(fd, data, data_len);
    write(fd, "\n", 1);
    response_ok(fd);
}


/**
argv requires x,y.
*/
static bool
show_display_area(int fd, int argc, char **argv)
{
    int x, y;
    //printf("%d, %s, %s\n", argc, argv[1], argv[2]);
    if (argc < 3) {
        response_error(fd, "too few arguments");
        return true;
    }

    x = atoi(argv[1]);
    y = atoi(argv[2]);
    lib_video_window_show();
    lib_video_window_move(x, y);

    response_ok(fd);
    return true;
}

static bool
hide_display_area(int fd, int argc, char **argv)
{
    lib_video_window_hide();

    response_ok(fd);
    return true;
}

/**
argv requires width, height.
*/
static bool
resize_display_area(int fd, int argc, char **argv)
{
    int w, h;
    //printf("%d, %s, %s\n", argc, argv[1], argv[2]);
    if (argc < 3) {
        response_error(fd, "too few arguments");
        return true;
    }

    w = atoi(argv[1]);
    h = atoi(argv[2]);
    lib_video_window_resize(w, h);

    response_ok(fd);
    return true;
}

/**
response
--------
<Width>x<Height>\n
+OK\n
--------
    */
static bool
get_video_size(int fd, int argc, char **argv)
{
    int w, h;

    lib_get_video_size(&w, &h);

    char buf[1024];
    sprintf(buf, "%dx%d\n", w, h);
    write(fd, buf, strlen(buf));

    response_ok(fd);
    return true;
}

/**
argv requires filepath or file-extension.
絕對路徑(/ 開頭)則直接儲存圖片於指定路徑，圖片格式由副檔名決定。
其他內容一律視為指定圖片格式，由IPC管道回傳圖片內容。

若選擇回傳圖片內容，則可用選用性的第三個參數選擇回傳型式，
目前只提供 binary 和 base64 兩種選擇。
第三參數若為 'base64' 則將資料內容以 base64 編碼字串回傳。
預設回傳 binary 資料。

response
--------
<bytes_of_data>\n
<data>\n
+OK\n
--------
*/
static bool
save_image_as(int fd, int argc, char **argv)
{
    if (argc < 2) {
        response_error(fd, "lose filepath/format argument");
        return true;
    }

    char *filepath = argv[1];
    char *save_format = "jpg";
    bool base64_encode = false;

    bool result;
    char *data = NULL;
    guint data_len = 0;

    if (*filepath == '/') {
        char *dot = strrchr(filepath, '.');
        if (dot == NULL) {
            response_error(fd, "unknown format");
            return true;
        }
        save_format = dot + 1;
    }
    else {
        save_format = filepath;
        if (argc >= 3 && strcmp(argv[2], "base64") == 0) {
            base64_encode = true;
        }
    }

    if (strchr("jJpPbB", save_format[0]) == NULL) {
        // 只支援 jpg, png, bmp
        response_error(fd, "unsupport format");
        return true;
    }

    if (filepath[0] == '/') { // save to file
        result = lib_save_frame(save_format, filepath);
        if (!result) {
            response_error(fd, "failed to save image to %s", filepath);
            return true;
        }

        response_ok(fd);
    }
    else { // response image data
        result = lib_capture_frame(save_format, (guint8 **)&data, &data_len);
        if (!result) {
            response_error(fd, "failed to capture image");
            return true;
        }

        if (base64_encode) {
            char *encoded_data = NULL;
            encoded_data = g_base64_encode((const guchar*)data, data_len);
            g_free(data);
            data_len = strlen(encoded_data);
            data = encoded_data;
        }

        response_data(fd, data, data_len);
        g_free(data);
    }

    return true;
}

static int
create_socket(const char *socket_path)
{
    int fd = -1;

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    //printf("addr sun_path %lu\n", sizeof(addr.sun_path)); // 108

    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

    unlink(addr.sun_path);

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        printf("could not create stream socket\n");
        return -1;
    }

    if (bind(fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) < 0) {
        printf("could not bind addr, %s\n", addr.sun_path);
        return -1;
    }

    listen(fd, 3);

    return fd;
}

static char **
read_command(int fd, int *out_argc)
{
    char buf[1024];
    memset(buf, 0, sizeof(buf));


    if (out_argc)
        *out_argc = 0;

    if (read(fd, buf, sizeof(buf)) < 0) {
        printf("socket error\n");
        return NULL;
    }

    buf[sizeof(buf)-1] = '\0';
    g_strstrip(buf);
    //printf("read [%s]\n", buf);
    if (strlen(buf) <= 1)
        return NULL;

    return split_string_to_argv(buf, out_argc);
}

static bool
command_dispatch(int fd)
{
    struct {
        const char *cmd;
        bool (*func)(int fd, int argc, char **argv);
    }
    cmd_tbl[] = {
        {
            "show", show_display_area
        },
        {
            "hide", hide_display_area
        },
        {
            "resize", resize_display_area
        },
        {
            "save", save_image_as
        },
        {
            "get-video-size", get_video_size
        },
        /*
        {
            "open", open_camera
        },
        {
            "close", close_camera
        },
        {
            "noop", no_operation
        },
        {
            "exit", exit_peer
        },
        */
        {
            NULL, NULL
        }
    },
    *pCmd = NULL;

    char **largv;
    int largc;

    largv = read_command(fd, &largc);
    if (largv == NULL)
        return false;

    bool cmd_ret;
    for (pCmd = cmd_tbl; pCmd->cmd != NULL; ++pCmd) {
        if (strcmp(pCmd->cmd, largv[0]) == 0) {
            //printf("invoke %s \n", pCmd->cmd);
            cmd_ret = pCmd->func(fd, largc, largv);
            g_strfreev(largv);
            return cmd_ret;
        }
    }
    g_strfreev(largv);

    response_error(fd, "unknown command");
    return true;
}

static bool
block_serve(int bfd)
{
    struct sockaddr_un peer_addr;
    socklen_t peer_addr_size = sizeof(struct sockaddr_un);

    int cfd;
    cfd = accept(bfd, (struct sockaddr *) &peer_addr, &peer_addr_size);
    if (cfd < 0) {
        printf("socket accept failed\n");
        return false;
    }

    response_ok(cfd);

    command_dispatch(cfd);

    close(cfd);
    return true;
}

void *
socket_command_loop(void *alldata)
{
    struct ALL_DATA *all_data = (struct ALL_DATA *) alldata;
    struct GLOBAL *global = all_data->global;

    umask(0); // allow others to access sockets.
    if (g_mkdir_with_parents(SOCKET_DIR, 0777) != 0) {
        //printf("Could not create camera status folder: %s\n", config_path);
        return NULL;
    }

    char idc = global->WVcaption[strlen(global->WVcaption)-1];
    if (idc >= '0' && idc <= '9') {
        idc -= '0';
    }
    else {
        return NULL;
    }

    char *socket_path = NULL;
    socket_path = g_strdup_printf(SOCKET_DIR "/uvc-service-socket-%d", idc);

    int bfd = create_socket(socket_path);
    if (bfd < 0) {
        printf("could not create socket\n");
        return NULL;
    }

    while (!all_data->videoIn->signalquit) {
        if (!block_serve(bfd))
            break;
    }

    close(bfd);
    unlink(socket_path);
    g_free(socket_path);

    return NULL;
}
