/*******************************************************************************#
#           guvcview              http://guvcview.berlios.de                    #
#                                                                               #
#           Paulo Assis <pj.assis@gmail.com>                                    #
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

#ifndef GLOBALS_H
#define GLOBALS_H

#include <glib.h>
// #ifdef USE_GDK
#include <gtk/gtk.h>
// #else
#include <SDL/SDL.h>
// #endif

#include "defs.h"
#include "utils.h"

typedef struct _VidBuff
{
	gboolean used;
	QWORD time_stamp;
	BYTE *frame;
	int bytes_used;
} VidBuff;

// 0-"jpg", 1-"bmp", 2-"png" (see create_image.c)
typedef enum {
    IMG_FORMAT_JPG = 0,
    IMG_FORMAT_BMP,
    IMG_FORMAT_PNG,
    IMG_FORMAT_RAW
} IMG_FORMAT;

/*global variables used in guvcview*/
struct GLOBAL
{
	__MUTEX_TYPE    mutex;         //global structure mutex
	__COND_TYPE     IO_cond;      //IO thread semaphore

	VidBuff *videoBuff;    //video Buffer

	char *videodevice;     // video device (def. /dev/video0)
	char *confPath;        //configuration file path
	char *WVcaption;       //video preview title bar caption
	char *mode;            //mjpg (default)

	//streaming
    char *streamingPort;
    char *streamingName;
    char *streamingPassword;

	QWORD v_ts;            //video time stamp
	DWORD framecount;      //video frame count

	int stack_size;        //thread stack size
	int vid_sleep;         //video thread sleep time (0 by default)
	int cap_meth;          //capture method: 1-mmap 2-read
	IMG_FORMAT imgFormat;  //image format
	int VidCodec;          //0-"MJPG"  1-"YUY2" 2-"DIB "(rgb32) 3-....
	int VidFormat;         //0-AVI 1-MKV ....
	int fps;               //fps denominator
	int fps_num;           //fps numerator (usually 1)
	int bpp;               //current bytes per pixel
	int hwaccel;           //use hardware acceleration
	int width;             //frame width
	int height;            //frame height
	int format;            //v4l2 pixel format
	int Frame_Flags;       //frame filter flags
	int skip_n;            //initial frames to skip
	int w_ind;             //write frame index
	int r_ind;             //read  frame index

	gboolean no_display;   //display on window or not.
	gboolean AFcontrol;    //Autofocus control flag (exists or not)
	gboolean autofocus;    //autofocus flag (enable/disable)
	gboolean debug;        //debug mode flag (--verbose)
	gboolean add_ctrls;    //flag for exiting after adding extension controls

    struct {
        gint x;
        gint y;
        gint width;
        gint height;
        gint depth;
    } geometry;

// #ifdef USE_GDK
    GdkWindow *inner_window;
    GdkWindow *foreign_window;
// #else
    SDL_Surface *liveview;
    SDL_Overlay *live_overlay;
    gboolean is_embed_view;
// #endif
};

/*----------------------------- prototypes ------------------------------------*/
int initGlobals(struct GLOBAL *global);

int closeGlobals(struct GLOBAL *global);

#endif

