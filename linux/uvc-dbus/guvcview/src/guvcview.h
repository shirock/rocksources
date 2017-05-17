/*******************************************************************************#
#           guvcview              http://guvcview.berlios.de                    #
#                                                                               #
#           Paulo Assis <pj.assis@gmail.com>                                    #
#           Nobuhiro Iwamatsu <iwamatsu@nigauri.org>                            #
#                             Add UYVY color support(Macbook iSight)            #
#           rock <shirock.tw@gmail.com>                                         #
#               Remove GTK codes.                                               #
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

#ifndef GUVCVIEW_H
#define GUVCVIEW_H
#include <SDL/SDL.h>

#include "v4l2uvc.h"

/*
 gettext alter.
*/
#define N_(s) (s)
#define _(s) (s)

/*
 GUI callbacks alter.
*/
#define ERR_DIALOG(a1,a2,a3)  g_printerr("%s. %s", a1, a2)
#define WARN_DIALOG(a1,a2,a3) g_printerr("%s. %s", a1, a2)

/* Must set this as global so they */
/* can be set from any callback.   */

struct ALL_DATA
{
    struct GLOBAL *global;
    struct focusData *AFdata;
    struct vdIn *videoIn;
    struct VidState *s;
    __THREAD_TYPE video_thread;
    __THREAD_TYPE IO_thread;
};

extern struct ALL_DATA all_data;

void lib_video_window_show();
void lib_video_window_hide();
void lib_video_window_move(int x, int y);
void lib_video_window_resize(int width, int height);
gboolean lib_get_video_size(int *width, int *height);
gboolean lib_capture_frame(const char*img_fmt, guint8 **data, guint *data_len);
gboolean lib_save_frame(const char*img_fmt, const char*img_filepath);
gboolean lib_start_streaming(const char *port, const char *username, const char *password);
void lib_stop_streaming();

#endif
