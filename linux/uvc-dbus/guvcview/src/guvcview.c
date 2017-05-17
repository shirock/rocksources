/*******************************************************************************#
#           guvcview              http://guvcview.berlios.de                    #
#                                                                               #
#           Paulo Assis <pj.assis@gmail.com>                                    #
#           Nobuhiro Iwamatsu <iwamatsu@nigauri.org>                            #
#           Dr. Alexander K. Seewald <alex@seewald.at>                          #
#                             Autofocus algorithm                               #
#           rock <shirock.tw@gmail.com>                                         #
#               For my dbus jobs.                                               #
#                                                                               #
# This program is free software; you can redistribute it and/or modify          #
# it under the terms of the GNU General Public License as published by          #
# the Free Software Foundation; either version 2 of the License, or             #
# (at your option) any later version.                                           #
#                                                                               #
# This program is distributed in the hope that it will be useful,               #
# but WITHOUT ANY WARRANTY; without even the implied warranty of                #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 #
# GNU General Public License for more details.                                  #
#                                                                               #
# You should have received a copy of the GNU General Public License             #
# along with this program; if not, write to the Free Software                   #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA     #
#                                                                               #
********************************************************************************/


#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <unistd.h>

#include "colorspaces.h"
#include "jpgenc.h"
#include "autofocus.h"
#include "picture.h"
#include "ms_time.h"
#include "string_utils.h"
#include "options.h"
#include "video.h"
#include "create_image.h"
#include "v4l2uvc.h"
#include "guvcview.h"
#include "globals.h"
#include "v4l2_controls.h"

/*----------------------------- globals --------------------------------------*/
/**
THIS LIBRARY IS NOT REENTRANT AND NOT THREAD-SAFED!
    by rock.

API defines.
*/
gchar *lib_get_controls_values();
gchar *lib_get_control_value(const char*name);
gboolean lib_set_control_value(const char*name, const char*json_value);
void lib_require_focus();
void lib_continue_focus(gboolean value);
void lib_auto_focus(gboolean value);

gboolean lib_get_video_size(int *width, int *height);
void lib_close_camera();
gboolean lib_capture_frame(const char*img_fmt, guint8 **data, guint *data_len);
gboolean lib_save_frame(const char*img_fmt, const char*img_filepath);
int lib_open_camera(const char*device_name,
                    const char*window_id,
                    const char*options);

struct ALL_DATA all_data;

struct GLOBAL *global = NULL;
struct focusData *AFdata = NULL;
struct vdIn *videoIn = NULL;

/*controls data*/
struct VidState *s = NULL;

/*thread definitions*/
//GThread *video_thread = NULL;


void init_controls (struct ALL_DATA *);

void *socket_command_loop(void *alldata);
void *http_server_thread(void *alldata);


//==== export functions ====
/**
return: NULL or a JSON string.
 */
gchar *lib_get_controls_values()
{
	struct VidState *s = all_data.s;
	struct vdIn *videoIn = all_data.videoIn;

    int row=0;

    get_ctrl_values (videoIn->fd, s->control_list, s->num_controls, NULL);

    Control *current = s->control_list;
    Control *next = current->next;
    gchar **str_array = NULL;
    str_array = g_new0(gchar*, s->num_controls + 1);

    while (current)
    {
        //printf("control name: %s; type: %d;\n", current->control.name, current->control.type);
        switch (current->control.type)
        {
#ifndef DISABLE_STRING_CONTROLS
            case V4L2_CTRL_TYPE_STRING: // 7
                //printf("\tstring: %s\n", current->string);
                str_array[row] = g_strdup_printf("\"%s\": \"%s\"",
                    current->control.name,
                    current->string);
                break;
#endif
            case V4L2_CTRL_TYPE_INTEGER64: // 5
                //printf("\tint64: %ld\n", current->value64);
                str_array[row] = g_strdup_printf("\"%s\": %ld",
                    current->control.name,
                    (long int) current->value64);
                break;
            default:
                //printf("\tint32: %d\n", current->value);
                str_array[row] = g_strdup_printf("\"%s\": %d",
                    current->control.name,
                    current->value);
                break;
        }

        row++;
        current = next;
        if (current)
            next = current->next;
    }

    int sz = 0;
    gchar *json_str = NULL, *ret_str;

    gchar *joined_str = g_strjoinv(",", str_array);
    json_str = g_strconcat("{", joined_str, "}", NULL);

    g_strfreev(str_array);
    //g_free(str_array);
    g_free(joined_str);

    sz = strlen(json_str) + 1;
    ret_str = malloc(sz);
    memcpy(ret_str, json_str, sz);
    g_free(json_str);

    return ret_str;
}

/**
return: NULL or a JSON string. Using free() to release.
 */
gchar *lib_get_control_value(const char*name)
{
	struct VidState *s = all_data.s;
	struct vdIn *videoIn = all_data.videoIn;
	Control *ctrl = NULL;
    gchar *json_str = NULL, *ret_str = NULL;
    int sz = 0;

	ctrl = get_ctrl_value_by_name(videoIn->fd, s->control_list, name);
    if (ctrl == NULL)
        return NULL;

    switch (ctrl->control.type)
    {
#ifndef DISABLE_STRING_CONTROLS
        case V4L2_CTRL_TYPE_STRING: // 7
            //printf("\tstring: %s\n", current->string);
            json_str = g_strdup_printf("\"%s\"",
                ctrl->string);
            break;
#endif
        case V4L2_CTRL_TYPE_INTEGER64: // 5
            //printf("\tint64: %ld\n", current->value64);
            json_str = g_strdup_printf("%ld",
                (long int) ctrl->value64);
            break;
        default:
            //printf("\tint32: %d\n", current->value);
            json_str = g_strdup_printf("%d",
                ctrl->value);
            break;
    }

    sz = strlen(json_str) + 1;
    ret_str = malloc(sz);
    memcpy(ret_str, json_str, sz);
    g_free(json_str);

    return ret_str;
}

gboolean lib_set_control_value(const char*name, const char*json_value)
{
	struct VidState *s = all_data.s;
	struct vdIn *videoIn = all_data.videoIn;
	int ret = 0;
	Control *ctrl = NULL;
    //gchar *tmp_str = NULL;

	ctrl = get_ctrl_value_by_name(videoIn->fd, s->control_list, name);
    if (ctrl == NULL)
        return FALSE;

    if (ctrl->control.flags & V4L2_CTRL_FLAG_READ_ONLY)
        return FALSE;

    if( ctrl->class == V4L2_CTRL_CLASS_USER)
    {
        struct v4l2_control v4l2_ctrl;
        //printf("   using VIDIOC_G_CTRL for user class controls\n");
        v4l2_ctrl.id = ctrl->control.id;
        v4l2_ctrl.value = atoi(json_value); //ctrl->value;
        ret = xioctl(videoIn->fd, VIDIOC_S_CTRL, &v4l2_ctrl);
    }
    else
    {
        //printf("   using VIDIOC_G_EXT_CTRLS on single controls for class: 0x%08x\n",
        //    current->class);
        struct v4l2_ext_controls v4l2_ctrls = {0};
        struct v4l2_ext_control v4l2_ctrl = {0};
        v4l2_ctrl.id = ctrl->control.id;
        switch (ctrl->control.type)
        {
#ifndef DISABLE_STRING_CONTROLS
            case V4L2_CTRL_TYPE_STRING:
                if (json_value[0] == '"') {
                    tmp_str = g_strdup(json_value+1);
                    tmp_str[strlen(tmp_str)-1] = '\0';
                }
                else {
                    tmp_str = g_strdup(json_value);
                }
                v4l2_ctrl.size = strlen(tmp_str); //ctrl->value;
                v4l2_ctrl.string = tmp_str; //ctrl->string;
                g_free(tmp_str);
                break;
#endif
            case V4L2_CTRL_TYPE_INTEGER64:
                v4l2_ctrl.value64 = atol(json_value); //ctrl->value64;
                break;
            default:
                v4l2_ctrl.value = atoi(json_value); //ctrl->value;
                break;
        }
        v4l2_ctrls.count = 1;
        v4l2_ctrls.controls = &v4l2_ctrl;
        ret = xioctl(videoIn->fd, VIDIOC_S_EXT_CTRLS, &v4l2_ctrls);
        if (ret)
            printf("control id: 0x%08x failed to set (error %i)\n",
                v4l2_ctrl.id, ret);
    }

    //update real value
    get_ctrl(videoIn->fd, s->control_list, ctrl->control.id, NULL);

    return (ret == 0 ? TRUE : FALSE);
}

void lib_require_focus()
{
    // printf("invoke require focus\n");
    if (global->AFcontrol < 1 || all_data.AFdata == NULL) {
        printf("This camera does not support Auto Focus\n");
        return;
    }

    if (!all_data.global->autofocus) {
        printf("manual autofocus\n");
        all_data.AFdata->setFocus = 1;
    }
}

void lib_continue_focus(gboolean value)
{
    // printf("invoke continue focus\n");
    all_data.global->autofocus = value;
}

/**
開關 webcam 硬體的自動對焦模組(若它內建的話)。
只有少數 webcam 內建此功能。
ps. require_focus(), continue_focus() 是軟體自動對焦。
 */
void lib_auto_focus(gboolean state)
{
    // printf("invoke auto focus\n");
    int rc;
    struct v4l2_ext_control ctrl = {0};
    struct v4l2_ext_controls ctrls = {0};

    ctrl.id = V4L2_CID_FOCUS_AUTO;
    ctrl.size = 0;
    ctrl.value = state;
    ctrls.controls = &ctrl;
    ctrls.count = 1;

    rc = xioctl(videoIn->fd, VIDIOC_S_EXT_CTRLS, &ctrls);
    if (rc == 0) {
        if (state == TRUE && all_data.global->autofocus == TRUE)
            lib_continue_focus(FALSE);
            // 兩者衝突，故開啟硬體自動對焦時，必須關閉軟體自動對焦
    }
    else {
        printf("auto focus failed\n");
    }
}

/**
這會回傳實際使用的影像尺寸。
選項參數 v4l2-width, v4l2-height 可指定想使用的影像尺寸。
但每種 webcam 實際可選用的影像尺寸並不一定相符，此時會選擇最接近的尺寸使用。
因此 get_video_size() 回傳的尺寸並不一定等於 v4l2-width, v4l2-height 。
 */
gboolean lib_get_video_size(int *width, int *height)
{
    *width  = all_data.global->width;
    *height = all_data.global->height;
    return TRUE;
}

void lib_video_window_show()
{
    // printf("invoke video show\n");
    video_show(all_data.global);
}

void lib_video_window_hide()
{
    // printf("invoke video hide\n");
    video_hide(all_data.global);
}

void lib_video_window_move(int x, int y)
{
    // printf("invoke video move\n");
    video_move(all_data.global, x, y);
}

void lib_video_window_resize(int width, int height)
{
    // printf("invoke video resize\n");
    video_resize(all_data.global, width, height);
}


void lib_close_camera()
{
    gdk_threads_leave();
    all_data.videoIn->signalquit = TRUE;
    close_v4l2(all_data.videoIn, FALSE);
    exit(0);
}

gboolean lib_capture_frame(const char*img_fmt, guint8 **data, guint *data_len)
{
    if (*data)
        g_free(*data);
    *data = NULL;
    *data_len = 0;

    switch (img_fmt[0])
    {
    case 'b':
    case 'B':
        all_data.global->imgFormat = IMG_FORMAT_BMP;
        break;
    case 'p':
    case 'P':
        all_data.global->imgFormat = IMG_FORMAT_PNG;
        break;
    case 'j':
    case 'J':
    default:
        all_data.global->imgFormat = IMG_FORMAT_JPG;
        break;
    }

    // store_picture
    store_picture_to_buffer(data, data_len);

    return (*data_len > 0 ? TRUE : FALSE);
}

gboolean lib_save_frame(const char*img_fmt, const char*img_filepath)
{
    // printf("invoke save frame\n");
    int ret = 0;

    switch (img_fmt[0])
    {
    case 'b':
    case 'B':
        all_data.global->imgFormat = IMG_FORMAT_BMP;
        break;
    case 'p':
    case 'P':
        all_data.global->imgFormat = IMG_FORMAT_PNG;
        break;
    case 'j':
    case 'J':
    default:
        all_data.global->imgFormat = IMG_FORMAT_JPG;
        break;
    }

    if (all_data.videoIn->ImageFName)
        g_free(all_data.videoIn->ImageFName);
    all_data.videoIn->ImageFName = g_strdup(img_filepath);

    ret=store_picture(&all_data);

    return (ret == 0 ? TRUE : FALSE);
}

gboolean lib_start_streaming(const char *port, const char *username, const char *password)
{
    if (all_data.videoIn->streaming)
        return TRUE;

    if (port && port[0] != '\0') {
        if (all_data.global->streamingPort)
            free(all_data.global->streamingPort);
        all_data.global->streamingPort = strdup(port);
    }

    if (username && username[0] != '\0') {
        if (all_data.global->streamingName)
            free(all_data.global->streamingName);
        all_data.global->streamingName = strdup(username);
    }

    if (password && password[0] != '\0') {
        if (all_data.global->streamingPassword)
            free(all_data.global->streamingPassword);
        all_data.global->streamingPassword = strdup(password);
    }

    all_data.videoIn->streaming = TRUE;
    printf("Streaming port:%s. Username:%s, Password:%s\n", all_data.global->streamingPort, all_data.global->streamingName, all_data.global->streamingPassword);

    if( __THREAD_CREATE(&all_data.IO_thread, http_server_thread, (void *) &all_data))
    {
        g_printerr("IO thread creation failed\n");
        all_data.videoIn->streaming = FALSE;
        return FALSE;
    }

    return TRUE;
}

void lib_stop_streaming()
{
    all_data.videoIn->streaming = FALSE;
}

//==== end export functions ====

// options
static void _lib_option_set_size(struct GLOBAL *global,
            const char*k, const void*_v)
{
    int v = *((int*)_v);
    if (strcmp(k, "v4l2-width") == 0) {
        global->width = v;
    }
    else {
        global->height = v;
    }
}

static void _lib_option_set_window_geometry(struct GLOBAL *global,
            const char*k, const void*_v)
{
    int v = *((int*)_v);
    if (strcmp(k, "window-x") == 0) {
        global->geometry.x = v;
    }
    else if (strcmp(k, "window-y") == 0) {
        global->geometry.y = v;
    }
    else if (strcmp(k, "window-width") == 0) {
        global->geometry.width = v;
    }
    else if (strcmp(k, "window-height") == 0) {
        global->geometry.height = v;
    }
}

static void _lib_option_set_hflip(struct GLOBAL *global,
            const char*k, const void*_v)
{
    global->Frame_Flags |= YUV_MIRROR;
}

static void _lib_option_set_debug(struct GLOBAL *global,
            const char*k, const void*_v)
{
    global->debug = TRUE;
}

static void _lib_option_set_mode(struct GLOBAL *global,
            const char*k, const void*_v)
{
    //v4l2-chroma
    const char *v = (const char*)_v;
    char modebuf[16];
    global->format = get_PixFormat(v);
    get_PixMode(global->format, modebuf);
    global->mode = g_strdup(modebuf);
}

static void _lib_parse_options(struct GLOBAL *global, const char*options)
{
    gchar **tokens = NULL;
    gchar *token = NULL;
    gchar *pkey = NULL, *pvalue = NULL;
    int i, int_value;

    struct {
        char *key;
        enum {
            STR,
            INT,
            BOOL
        } value_type;
        void (*set_func)(struct GLOBAL *, const char*k, const void*v);
    }
    tbl_options[] = {
        {"v4l2-width",      INT,    _lib_option_set_size},
        {"v4l2-height",     INT,    _lib_option_set_size},
        {"v4l2-hflip",      BOOL,   _lib_option_set_hflip},
        {"v4l2-chroma",     STR,    _lib_option_set_mode},
        {"window-x",        INT,    _lib_option_set_window_geometry},
        {"window-y",        INT,    _lib_option_set_window_geometry},
        {"window-width",    INT,    _lib_option_set_window_geometry},
        {"window-height",   INT,    _lib_option_set_window_geometry},
        {"debug",           BOOL,   _lib_option_set_debug},
        {NULL}
    },
    *tbl_options_idx = NULL;

    tokens = g_strsplit_set(options, " ,;:", 10/*max_tokens*/);

    if (!tokens)
        return;

    for (i = 0, token = tokens[i]; token; ++i, token = tokens[i])
    {
        if (*token == 0)
            continue;
        //printf("option: [%s]\n", token);

        pkey = token;
        pvalue = strchr(pkey, '=');
        if (pvalue) {
            *pvalue = '\0';
            ++pvalue;
        }
        else {
            continue;
        }

        for (tbl_options_idx = tbl_options;
            tbl_options_idx->key;
             ++tbl_options_idx)
         {
            if (strcmp(tbl_options_idx->key, pkey) != 0)
                continue;
            if (tbl_options_idx->value_type == INT) {
                int_value = (pvalue ? atoi(pvalue) : 0);
                tbl_options_idx->set_func(global, pkey, &int_value);
            }
            else if (tbl_options_idx->value_type == BOOL) {
                tbl_options_idx->set_func(global, pkey, NULL);
            }
            else {
                tbl_options_idx->set_func(global, pkey, pvalue);
            }
         }

    }
    g_strfreev(tokens);

    return;
}

/*--------------------------------- MAIN -------------------------------------*/
/** lib_open_camera()

THIS LIBRARY IS NOT REENTRANT AND NOT THREAD-SAFED!
IT MEANS ONE PROCESS COULD ONLY OPEN ONE CAMERA!

arguments:
+ device_name
+ window_id
+ options

options:
+ v4l2-width: int, -1
+ v4l2-height: int, -1
+ v4l2-chroma: string, "YUYV", "MJPG"
+ v4l2-hflip: enable horizon fliter.
+ debug: enable debug mode.

return:
+ VDIN_OK                    0
+ VDIN_DEVICE_ERR           -1
+ VDIN_FORMAT_ERR           -2
+ VDIN_REQBUFS_ERR          -3
+ VDIN_ALLOC_ERR            -4
+ VDIN_RESOL_ERR            -5
+ VDIN_FBALLOC_ERR          -6
+ VDIN_UNKNOWN_ERR          -7
+ VDIN_DEQBUFS_ERR          -8
+ VDIN_DECODE_ERR           -9
+ VDIN_QUERYCAP_ERR        -10
+ VDIN_QUERYBUF_ERR        -11
+ VDIN_QBUF_ERR            -12
+ VDIN_MMAP_ERR            -13
+ VDIN_READ_ERR            -14
+ VDIN_STREAMON_ERR        -15
+ VDIN_STREAMOFF_ERR       -16
+ VDIN_DYNCTRL_ERR         -17

 */
int lib_open_camera(const char*device_name,
                    const char*window_id,
                    const char*options)
{
	int ret=0;
	gboolean control_only = FALSE;

	/* initialize glib threads - make glib thread safe*/
#if !defined(GLIB_DEPRECATED_IN_2_32)
	g_thread_init(NULL);
#endif
	gdk_threads_init ();

	/*structure containing all shared data - passed in callbacks*/
//	struct ALL_DATA all_data;
	memset(&all_data,0,sizeof(struct ALL_DATA));

	/*allocate global variables*/
	global = g_new0(struct GLOBAL, 1);
	initGlobals(global);

	/*------------------------- reads configuration file ---------------------*/
	g_print("Read configuration: %s\n", global->confPath);
	readConf(global);

	/*------------------------ reads command line options --------------------*/
    _lib_parse_options(global, options);

    if (global->videodevice)
        g_free(global->videodevice);
    global->videodevice = g_strdup(device_name);
    global->WVcaption = g_strdup_printf("uvcvideo - %s", device_name);

	//sets local control_only flag - prevents several initializations/allocations
	control_only = (global->control_only || global->add_ctrls) ;

	/*---------------------------------- Allocations -------------------------*/

	s = g_new0(struct VidState, 1);

	if(!control_only) /*control_only exclusion (video and Audio) */
	{

		// must be called before using avcodec lib
		avcodec_init();

		// register all the codecs (you can also register only the codec
		//you wish to have smaller code
		avcodec_register_all();

	}

	/*---------------------------- GTK init ----------------------------------*/

    int argc = 1;
    char *_argv1 = "libguvcview";
    char **argv = {&_argv1};
	gtk_init(&argc, &argv);
//	g_set_application_name("Guvcview Video Capture");

    printf("Window id: %s\n", window_id);
    if (window_id[0] == 'N' || window_id[0] == 'n' ) {
        global->no_display = TRUE;
    }
    else if (window_id[0] == '0') {
        // create an inner window
        // let width and height to be 0.
        // after creating window, it will set width and height as size of video.
        global->foreign_window = NULL;
    }
    else {
        setenv("SDL_WINDOWID", window_id, TRUE);
        GdkNativeWindow xid = atoi(window_id);

        //all_data.foreign_window = gdk_window_foreign_new(xid);
        global->foreign_window = gdk_window_foreign_new(xid);
        gdk_window_get_geometry(global->foreign_window,
            &global->geometry.x,
            &global->geometry.y,
            &global->geometry.width,
            &global->geometry.height,
            &global->geometry.depth
            );
        printf("Window size: width: %d; height: %d;\n", global->geometry.width, global->geometry.height);
    }

	/*----------------------- init videoIn structure --------------------------*/
	videoIn = g_new0(struct vdIn, 1);

	/*set structure with all global allocations*/
	all_data.global = global;
	all_data.AFdata = AFdata; /*not allocated yet*/
	all_data.videoIn = videoIn;
	all_data.s = s;

	/*get format from selected mode*/
	global->format = get_PixFormat(global->mode);
	if(global->debug) g_printf("%s: setting format to %i\n", global->mode, global->format);

	if ( ( ret=init_videoIn (videoIn, global) ) != 0)
	{
		g_printerr("Init video returned %i\n",ret);
		switch (ret)
		{
			case VDIN_DEVICE_ERR://can't open device
				ERR_DIALOG ("Guvcview error:\n\nUnable to open device",
					"Please make sure the camera is connected\nand that the correct driver is installed.",
					&all_data);
				break;

			#if 0
			case VDIN_DYNCTRL_OK: //uvc extension controls OK, give warning and shutdown (called with --add_ctrls)
				WARN_DIALOG ("Guvcview:\n\nUVC Extension controls",
					"Extension controls were added to the UVC driver",
					&all_data);
				//clean_struct(&all_data);
				close_v4l2(all_data.videoIn, control_only);
				exit(0);
				break;
            #endif

			case VDIN_DYNCTRL_ERR: //uvc extension controls error - EACCES (needs root user)
				ERR_DIALOG ("Guvcview error:\n\nUVC Extension controls",
					"An error ocurred while adding extension\ncontrols to the UVC driver\nMake sure you run guvcview as root (or sudo).",
					&all_data);
				break;

			case VDIN_UNKNOWN_ERR: //unknown error (treat as invalid format)
			case VDIN_FORMAT_ERR://invalid format
			case VDIN_RESOL_ERR: //invalid resolution
				g_printf("trying minimum setup ...\n");
				if (videoIn->listFormats->numb_formats > 0) //check for supported formats
				{
					VidFormats *listVidFormats;
					videoIn->listFormats->current_format = 0; //get the first supported format
					global->format = videoIn->listFormats->listVidFormats[0].format;
					if(get_PixMode(global->format, global->mode) < 0)
						g_printerr("IMPOSSIBLE: format has no supported mode !?\n");
					listVidFormats = &videoIn->listFormats->listVidFormats[0];
					global->width = listVidFormats->listVidCap[0].width;
					global->width = listVidFormats->listVidCap[0].height;
					if (listVidFormats->listVidCap[0].framerate_num != NULL)
						global->fps_num = listVidFormats->listVidCap[0].framerate_num[0];
					if (listVidFormats->listVidCap[0].framerate_denom != NULL)
						global->fps = listVidFormats->listVidCap[0].framerate_denom[0];
				}
				else
				{
					g_printerr("ERROR: Can't set video stream. No supported format found\nExiting...\n");
					ERR_DIALOG ("Guvcview error:\n\nCan't set a valid video stream for guvcview",
						"Make sure your device driver is v4l2 compliant\nand that it is properly installed.",
						&all_data);
				}

				//try again with new format
				ret = init_videoIn (videoIn, global);

				if ((ret == VDIN_QUERYBUF_ERR) && (global->cap_meth != videoIn->cap_meth))
				{
					//mmap not supported ? try again with read method
					g_printerr("mmap failed trying read method...");
					global->cap_meth = videoIn->cap_meth;
					ret = init_videoIn (videoIn, global);
					if (ret == VDIN_OK)
						g_printerr("OK\n");
					else
						g_printerr("FAILED\n");
				}

				if (ret < 0)
				{
					g_printerr("ERROR: Minimum Setup Failed.\n Exiting...\n");
					ERR_DIALOG ("Guvcview error:\n\nUnable to start with minimum setup",
						"Please reconnect your camera.",
						&all_data);
				}

				break;

			case VDIN_QUERYBUF_ERR:
				if (global->cap_meth != videoIn->cap_meth)
				{
					//mmap not supported ? try again with read method
					g_printerr("mmap failed trying read method...");
					global->cap_meth = videoIn->cap_meth;
					ret = init_videoIn (videoIn, global);
					if (ret == VDIN_OK)
						g_printerr("OK\n");
					else
					{
						g_printerr("FAILED\n");
						//return to default method(mmap)
						global->cap_meth = IO_MMAP;
						g_printerr("ERROR: Minimum Setup Failed.\n Exiting...\n");
						ERR_DIALOG (N_("Guvcview error:\n\nUnable to start with minimum setup"),
							N_("Please reconnect your camera."),
							&all_data);
					}
				}
				break;

			case VDIN_QUERYCAP_ERR:
				ERR_DIALOG (N_("Guvcview error:\n\nCouldn't query device capabilities"),
					N_("Make sure the device driver suports v4l2."),
					&all_data);
				break;
			case VDIN_READ_ERR:
				ERR_DIALOG (N_("Guvcview error:\n\nRead method error"),
					N_("Please try mmap instead (--capture_method=1)."),
					&all_data);
				break;

			case VDIN_REQBUFS_ERR:/*unable to allocate dequeue buffers or mem*/
			case VDIN_ALLOC_ERR:
			case VDIN_FBALLOC_ERR:
			default:
				ERR_DIALOG (N_("Guvcview error:\n\nUnable to allocate Buffers"),
					N_("Please try restarting your system."),
					&all_data);
				break;
		}
	}

	if (ret < 0) {
	    // ERROR!
	    return ret;
    }

	videoIn->listFormats->current_format = get_FormatIndex(videoIn->listFormats, global->format);
	if(videoIn->listFormats->current_format < 0)
	{
		g_printerr("ERROR: Can't set video stream. No supported format found\nExiting...\n");
		ERR_DIALOG (N_("Guvcview error:\n\nCan't set a valid video stream for guvcview"),
			N_("Make sure your device driver is v4l2 compliant\nand that it is properly installed."),
			&all_data);
	}
	/*-----------------------------GTK widgets---------------------------------*/
	/*----------------------- Image controls Tab ------------------------------*/
	init_controls(&all_data);

	if (!control_only) /*control_only exclusion*/
	{
		/*------------------ Creating the main loop (video) thread ---------------*/
		GError *err1 = NULL;

        if( __THREAD_CREATE(&all_data.video_thread, main_loop, (void *) &all_data))
		{
			g_printerr("Thread create failed: %s!!\n", err1->message );
			g_error_free ( err1 ) ;

			ERR_DIALOG (N_("Guvcview error:\n\nUnable to create Video Thread"),
				N_("Please report it to http://developer.berlios.de/bugs/?group_id=8179"),
				&all_data);
		}
		//all_data.video_thread = video_thread;
	}/*end of control_only exclusion*/

	__THREAD_TYPE socket_command_thread;
    if( __THREAD_CREATE(&socket_command_thread, socket_command_loop, (void *) &all_data))
    {
        //g_printerr("Thread create failed: %s!!\n", err1->message );
        //g_error_free ( err1 ) ;

        //ERR_DIALOG (N_("Guvcview error:\n\nUnable to create Video Thread"),
        //    N_("Please report it to http://developer.berlios.de/bugs/?group_id=8179"),
        //    &all_data);
        printf("Could not create socket command thread. Disable this feature.\n");
    }

    printf("PID: %d\n", getpid());
	/* The last thing to get called */
	gdk_threads_enter();

//	g_printf("Closing GTK... OK\n");
	return 0;
}


#ifdef DEMO
void signal_exit_program(int n)
{
    lib_close_camera(0);
}

// TODO snapshot
void test_capture_image(int n)
{
    printf("capture\n");
	int ret = 0;
	char *img_fmt = "png";
	char *img_filename = "Image.png";
	char *img_filename2 = "Image_buff.png";

	if((ret = lib_save_frame(img_fmt, img_filename)) < 0)
		g_printerr("saved image to:%s ...Failed \n", all_data.videoIn->ImageFName);
	else
	    g_printf("saved image to:%s ...OK \n", all_data.videoIn->ImageFName);

    guint8 *data;
    guint data_len;
    data = NULL;
    data_len = 0;

    lib_capture_frame(img_fmt, &data, &data_len);
    if (g_file_set_contents(img_filename2, (const gchar*)data, data_len, NULL))
	    g_printf("saved image to buffer, size: %d ...OK \n", data_len);
	else
		g_printerr("saved image to buffer, size: %d ...Failed \n", data_len);
    g_free(data);

    int width, height;
    lib_get_video_size(&width, &height);
    printf("video size: %d, %d\n", width, height);

    signal(SIGUSR1, test_capture_image);
}

int main(int argc, char **argv)
{
    int rc = 0;
    char *camera, *wid, *options, *port;

    if (argc < 2)
    {
        printf("\n");
        printf("%s <uvc_name> [window_id] [options] [streaming-port]\n\n", argv[0]);
        printf("  uvc_name: like /dev/video0\n");
        printf("  window_id:\n");
        printf("    0: New monitor window.\n");
        printf("    No: No monitor.\n");
        printf("    a integer: embed monitor.\n");
        printf("  options: camera options.\n");
        printf("  streaming-port: HTTP streaming port.\n");
        printf("\n");
        return 1;
    }

    signal(SIGINT,  signal_exit_program);
    signal(SIGTERM, signal_exit_program);
    // TODO snapshot
    signal(SIGUSR1, test_capture_image);

    camera = argv[1];
    wid = (argc >= 3 ? argv[2] : "0");
    options = (argc >= 4 ? argv[3] : "");
    port = (argc >= 5 ? argv[4] : NULL);

    rc = lib_open_camera(camera, wid, options);

    if (port) {
        lib_start_streaming(port, NULL, NULL);
    }

    char cmd[128];

    printf("capture\n");
    char *img_fmt = NULL;
    char img_filename[PATH_MAX];
    guint8 *data;
    guint data_len;
    int width, height;
    gchar *json_str;

    int snapshot_cnt = 0;

    sleep(1);
    //printf("all_data.global->autofocus: %s\n", all_data.global->autofocus ? "Yes" : "No");
    while(TRUE)
    {
        printf("Command: s (save jpeg, sb: bmp, sp: png), c (capture), v (video size), "
               "Px,y,w,h (change geometry), S (show window), H (hide window), "
               "rPort (start streaming at Port), R (stop streaming), "
               "l (list controls), gName (get control value), VName=Value (set control value), "
               "f (require autofocus), F (continue autofocus), x (exit)\n");
        printf("cmd> ");
        fgets(cmd, sizeof(cmd), stdin);
        cmd[strlen(cmd)-1] = '\0';
        switch (cmd[0])
        {
        case 's':
        case 'c':
            if (strlen(cmd) >= 2) {
                if (cmd[1] == 'b') {
                    img_fmt = "bmp";
                    sprintf(img_filename, "Image_%c%d.bmp", cmd[0], snapshot_cnt++);
                }
                else if (cmd[1] == 'p') {
                    img_fmt = "png";
                    sprintf(img_filename, "Image_%c%d.png", cmd[0], snapshot_cnt++);
                }
                else {
                    img_fmt = "jpg";
                    sprintf(img_filename, "Image_%c%d.jpg", cmd[0], snapshot_cnt++);
                }
            }
            else {
                img_fmt = "jpg";
                sprintf(img_filename, "Image_%c%d.jpg", cmd[0], snapshot_cnt++);
            }

            if (cmd[0] == 's') {
                if(lib_save_frame(img_fmt, img_filename) < 0)
                    g_printerr("saved image to:%s ...Failed \n", img_filename);
                else
                    g_printf("saved image to:%s ...OK \n", img_filename);
            }
            else {
                data = NULL;
                data_len = 0;
                lib_capture_frame(img_fmt, &data, &data_len);
                if (g_file_set_contents(img_filename, (const gchar*)data, data_len, NULL))
                    g_printf("saved image to buffer, size: %d ...OK \n", data_len);
                else
                    g_printerr("saved image to buffer, size: %d ...Failed \n", data_len);
                g_free(data);
            }
            break;
        case 'v':
            lib_get_video_size(&width, &height);
            printf("video size: %d, %d\n", width, height);
            break;
        case 'x':
        case 'q':
            lib_close_camera();
            break;
        case 'l':
            json_str = lib_get_controls_values();
            if (json_str)
            {
                printf("Control values: %s\n", json_str);
                free(json_str);
            }
            break;
        case 'g':
            json_str = lib_get_control_value(cmd+1);
            if (json_str)
            {
                printf("Control values: %s\n", json_str);
                free(json_str);
            }
            break;
        case 'P': // video_set_geometry
            {
                char *p1, *p2;
                int x, y, w, h;
                x = y = w = h = -1;

                p1 = cmd+1;
                if ((p2 = strchr(p1, ',')) == NULL)
                    break;
                *p2 = '\0';
                x = atoi(p1);

                p1 = p2+1;
                if ((p2 = strchr(p1, ',')) == NULL)
                    break;
                *p2 = '\0';
                y = atoi(p1);

                p1 = p2+1;
                if ((p2 = strchr(p1, ',')) == NULL)
                    break;
                *p2 = '\0';
                w = atoi(p1);

                p1 = p2+1;
                h = atoi(p1);

                //video_set_geometry(global, x, y, w, h);
                if (x >=0 && y >= 0)
                    video_move(global, x, y);
                if (w >=0 && h >= 0)
                    video_resize(global, w, h);
            }
            break;
        case 'S':
            video_show(global);
            break;
        case 'H':
            video_hide(global);
            break;
        case 'V':
            json_str = strchr(cmd, '=');
            if (json_str == NULL) {
                break;
            }
            else {
                *json_str = '\0';
                ++json_str;
            }
            lib_set_control_value(cmd+1, json_str);
            break;
        case 'f':
            lib_continue_focus(FALSE);
            lib_require_focus();
            break;
        case 'F':
            lib_continue_focus(TRUE);
            break;
        case 'r':
            {
                char *port = NULL;
                if (cmd[1] == '\0')
                    port = "8080";
                else
                    port = cmd+1;
                lib_start_streaming(port, NULL, NULL);
            }
            break;
        case 'R':
            lib_stop_streaming();
            break;
        }
    }
//	gtk_main();
//	gdk_threads_leave();

    return rc;
}
#endif
