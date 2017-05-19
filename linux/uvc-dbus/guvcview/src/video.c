/*******************************************************************************#
#           guvcview              http://guvcview.berlios.de                    #
#                                                                               #
#           Paulo Assis <pj.assis@gmail.com>                                    #
#           Nobuhiro Iwamatsu <iwamatsu@nigauri.org>                            #
#                             Add UYVY color support(Macbook iSight)            #
#           rock <shirock.tw@gmail.com>                                         #
#               Add scaling, foreign window, window position.                   #
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

#include <SDL/SDL.h>
#include <glib.h>
#include <glib/gprintf.h>
// #ifdef USE_GDK
#include <gtk/gtk.h>
// #else
#include <SDL/SDL.h>
// #endif
#include <X11/X.h>

#include "defs.h"
#include "video.h"
#include "guvcview.h"
#include "v4l2uvc.h"
#include "colorspaces.h"
#include "video_filters.h"
#include "jpgenc.h"
#include "autofocus.h"
#include "picture.h"
#include "ms_time.h"
#include "string_utils.h"
#include "create_image.h"

#define __VMUTEX &videoIn->mutex
#define __GMUTEX &global->mutex

// call gdk_window_get_geometry() to reflush state.
// #ifdef USE_GDK
#define WINDOW_OUT_OF_SYNC  (gdk_window_get_geometry(global->inner_window, NULL, NULL, NULL, NULL, NULL))
// #else
// #define WINDOW_OUT_OF_SYNC  (NULL)
// #endif

static bool xlib_get_xid(struct GLOBAL *global, XID *xid)
{
    gchar *wm_argv[] = {
        "/usr/bin/xwininfo",
        "-name",
        global->WVcaption,
        NULL
    };
    gchar *wm_out;
    gchar *wm_err;
    gint wm_status;
    gchar *xid_str = NULL;

    if (global->debug)
        printf("xwininfo -int -name \"%s\"\n", global->WVcaption);

    *xid = 0;

    int i;
    for (i = 0; i < 5; ++i) {
        wm_out = wm_err = NULL;
        wm_status = 0;
        if (!g_spawn_sync("/",
            wm_argv,
            NULL,
            0, // G_SPAWN_DEFAULT
            NULL, NULL,
            &wm_out, &wm_err, &wm_status, NULL)
        ) {
            printf("xwininfo false\n");
            continue;
        }

        if (wm_status == 0) {
            // get xid from string: "xwininfo: Window id: 0x123455 "uvcvideo - /dev/video0""
            if (global->debug)
                printf("%s\n", wm_out);

            if (!(xid_str = strstr(wm_out, "id: 0x"))) {
                printf("XID not found\n");
                break;
            }

            xid_str += 4; // seek to the head of xid string.
            *xid = strtol(xid_str, NULL, 16);
            printf("XID: %lu\n", *xid);
            break;
        }
        else {
            printf("xwininfo retry\n");
        }

        if (wm_out)
            g_free(wm_out);
        if (wm_err)
            g_free(wm_err);
        sleep_ms(200);
    }

    if (wm_out)
        g_free(wm_out);
    if (wm_err)
        g_free(wm_err);

    if (*xid < 1)
        return false;
    return true;
}

static Uint32 get_sdl_video_flags(const struct GLOBAL *global)
{
    Uint32 flags =
        SDL_ANYFORMAT | SDL_DOUBLEBUF | SDL_RESIZABLE | SDL_NOFRAME;

    const SDL_VideoInfo *info;
    info = SDL_GetVideoInfo();

    if (info->hw_available) {
        if (global->debug)
            g_printf("Hardware surfaces are available (%dK video memory)\n", info->video_mem);

        flags |= SDL_HWSURFACE;
    }

    if (info->blit_hw) {
        if (global->debug)
            g_printf("Copy blits between hardware surfaces are accelerated\n");

        flags |= SDL_ASYNCBLIT;
    }

    if (!(flags & SDL_HWSURFACE)) {
        flags |= SDL_SWSURFACE;
    }

    if (global->debug) {
        //if (info->wm_available) g_printf("A window manager is available\n");
        if (info->blit_hw_CC) g_printf ("Colorkey blits between hardware surfaces are accelerated\n");
        if (info->blit_hw_A) g_printf("Alpha blits between hardware surfaces are accelerated\n");
        if (info->blit_sw) g_printf ("Copy blits from software surfaces to hardware surfaces are accelerated\n");
        if (info->blit_sw_CC) g_printf ("Colorkey blits from software surfaces to hardware surfaces are accelerated\n");
        if (info->blit_sw_A) g_printf("Alpha blits from software surfaces to hardware surfaces are accelerated\n");
        if (info->blit_fill) g_printf("Color fills on hardware surfaces are accelerated\n");
    }

    return flags;
}

static SDL_Overlay * video_init(void *data, SDL_Surface **pscreen)
{
    struct ALL_DATA *all_data = (struct ALL_DATA *) data;
    struct GLOBAL *global = all_data->global;

    static Uint32 SDL_VIDEO_Flags =
        SDL_ANYFORMAT | SDL_DOUBLEBUF | SDL_RESIZABLE | SDL_NOFRAME;

    if (*pscreen == NULL) //init SDL
    {
        char driver[128];
        /*----------------------------- Test SDL capabilities ---------------------*/
        if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0)
        {
            g_printerr("Couldn't initialize SDL: %s\n", SDL_GetError());
            exit(1);
        }

        /* For this version, we will use hardware acceleration as default*/
        if(global->hwaccel)
        {
            if ( !getenv("SDL_VIDEO_YUV_HWACCEL") ) putenv("SDL_VIDEO_YUV_HWACCEL=1");
            if ( !getenv("SDL_VIDEO_YUV_DIRECT") ) putenv("SDL_VIDEO_YUV_DIRECT=1");
        }
        else
        {
            if ( !getenv("SDL_VIDEO_YUV_HWACCEL") ) putenv("SDL_VIDEO_YUV_HWACCEL=0");
            if ( !getenv("SDL_VIDEO_YUV_DIRECT") ) putenv("SDL_VIDEO_YUV_DIRECT=0");
        }

        if (SDL_VideoDriverName(driver, sizeof(driver)) && global->debug)
        {
            g_printf("Video driver: %s\n", driver);
        }

        SDL_VIDEO_Flags = get_sdl_video_flags(global);

        SDL_WM_SetCaption(global->WVcaption, NULL);

        /* enable key repeat */
        SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);
    }
    /*------------------------------ SDL init video ---------------------*/

    if (!global->foreign_window)
    {
        char pos_buf[128];
        snprintf(pos_buf, sizeof(pos_buf),
            "SDL_VIDEO_WINDOW_POS=%d,%d",
            global->geometry.x,
            global->geometry.y
        );
        if (global->debug)
            printf("Set window position: %s\n", pos_buf);
        //SDL_putenv("SDL_VIDEO_CENTERED=center");
        SDL_putenv(pos_buf); // if I use an allocated buff, this putenv will not work.
    }

    *pscreen = SDL_SetVideoMode(
        global->geometry.width, //global->width,
        global->geometry.height, //global->height,
        global->bpp,
        SDL_VIDEO_Flags);

    SDL_Overlay* overlay=NULL;
    overlay = SDL_CreateYUVOverlay(global->width, global->height,
        SDL_YUY2_OVERLAY, *pscreen);

// #if USE_GDK
    if (!global->foreign_window) {
        //wmctrl(global, "-b", "add,above"); // set window to keepabove
        XID xid;
        if (xlib_get_xid(global, &xid)) {
            global->inner_window = gdk_window_foreign_new(xid);
            gdk_window_set_keep_above(global->inner_window, true);
            gdk_window_move(global->inner_window, global->geometry.x, global->geometry.y);

            WINDOW_OUT_OF_SYNC;
        }
        else {
            global->inner_window = NULL;
        }
    }
// #endif

    global->live_overlay = overlay;
    return (overlay);
}

void video_resize(struct GLOBAL *global, int width, int height)
{
    //video_set_geometry(global, -1, -1, width, height);
    if (!global->inner_window)
        return;
    gdk_window_resize(global->inner_window, width, height);
    global->geometry.width = width;
    global->geometry.height = height;
    WINDOW_OUT_OF_SYNC;
}

void video_move(struct GLOBAL *global, int x, int y)
{
    //video_set_geometry(global, -1, -1, width, height);
    if (!global->inner_window)
        return;
    gdk_window_move(global->inner_window, x, y);
    global->geometry.x = x;
    global->geometry.y = y;
    WINDOW_OUT_OF_SYNC;
}

void video_hide(struct GLOBAL *global)
{
    if (!global->inner_window)
        return;
    gdk_window_hide(global->inner_window);
    WINDOW_OUT_OF_SYNC;
}

void video_show(struct GLOBAL *global)
{
    if (!global->inner_window)
        return;
    gdk_window_show(global->inner_window);
    gdk_window_set_keep_above(global->inner_window, true);
    video_move(global, global->geometry.x, global->geometry.y);
}

/*-------------------------------- Main Video Loop ---------------------------*/
/* run in a thread (SDL overlay)*/
void *main_loop(void *data)
{
    struct ALL_DATA *all_data = (struct ALL_DATA *) data;

    struct VidState *s = all_data->s;
    struct GLOBAL *global = all_data->global;
    struct focusData *AFdata = all_data->AFdata;
    struct vdIn *videoIn = all_data->videoIn;

    struct particle* particles = NULL; //for the particles video effect

    SDL_Event event;
    /*the main SDL surface*/
    SDL_Surface *pscreen = NULL;
    SDL_Overlay *overlay = NULL;
    SDL_Rect drect;

    int width = global->width;
    int height = global->height;
    int format = global->format;

    if (global->geometry.width <= 0)
        global->geometry.width = width;
    if (global->geometry.height <= 0)
        global->geometry.height = height;

    BYTE *p = NULL;

    Control *focus_control = NULL;
    int last_focus = 0;

    int size_renew_count = 0;

    // autofocus
    if (global->AFcontrol)
    {
        focus_control = get_ctrl_by_id(s->control_list, AFdata->id);
        get_ctrl(videoIn->fd, s->control_list, AFdata->id, all_data);
        last_focus = focus_control->value;
        /*make sure we wait for focus to settle on first check*/
        if (last_focus < 0) last_focus = AFdata->f_max;
    }

    /*------------------------------ SDL init video ---------------------*/
    if(!global->no_display)
    {
        overlay = video_init(data, &(pscreen));
        p = (unsigned char *) overlay->pixels[0];

        drect.x = 0;
        drect.y = 0;
        drect.w = global->geometry.width;
        drect.h = global->geometry.height;
    }

    //gboolean capVid = FALSE;
    gboolean signalquit = FALSE;

    while (!signalquit)
    {
        __LOCK_MUTEX(__VMUTEX);
            //capVid = videoIn->capVid;
            signalquit = videoIn->signalquit;
        __UNLOCK_MUTEX(__VMUTEX);

        if (videoIn->signalquit)
            break;

        /*-------------------------- Grab Frame ----------------------------------*/
        if (uvcGrab(videoIn, format, width, height, &global->fps, &global->fps_num) < 0)
        {
            //g_printerr("Error grabbing image \n");
            continue;
        }
        else
        {
            if(!videoIn->timestamp)
            {
                global->skip_n++; //skip this frame
            }

            /*---------------- autofocus control ------------------*/

            if (global->AFcontrol && (global->autofocus || AFdata->setFocus))
            { /*AFdata = NULL if no focus control*/
                if (AFdata->focus < 0)
                {
                    /*starting autofocus*/
                    AFdata->focus = AFdata->left; /*start left*/
                    focus_control->value = AFdata->focus;
                    // autofocus
                    if (set_ctrl (videoIn->fd, s->control_list, AFdata->id) != 0) { // couldn't to set focus
                        global->autofocus = FALSE;
                        AFdata->setFocus = 0;
                    }
                    /*number of frames until focus is stable*/
                    /*1.4 ms focus time - every 1 step*/
                    AFdata->focus_wait = (int) abs(AFdata->focus-last_focus)*1.4/(1000/global->fps)+1;
                    last_focus = AFdata->focus;
                }
                else
                {
                    if (AFdata->focus_wait == 0)
                    {
                        AFdata->sharpness=getSharpness (videoIn->framebuffer, width, height, 5);
                        if (global->debug)
                            g_printf("sharp=%d focus_sharp=%d foc=%d right=%d left=%d ind=%d flag=%d\n",
                                AFdata->sharpness,AFdata->focus_sharpness,
                                AFdata->focus, AFdata->right, AFdata->left,
                                AFdata->ind, AFdata->flag);
                        AFdata->focus=getFocusVal (AFdata);
                        if ((AFdata->focus != last_focus))
                        {
                            focus_control->value = AFdata->focus;
                            // autofocus
                            if (set_ctrl (videoIn->fd, s->control_list, AFdata->id) != 0) {
                                //g_printerr("ERROR: couldn't set focus to %d\n", AFdata->focus);
                                global->autofocus = FALSE;
                                AFdata->setFocus = 0;
                            }
                            /*number of frames until focus is stable*/
                            /*1.4 ms focus time - every 1 step*/
                            AFdata->focus_wait = (int) abs(AFdata->focus-last_focus)*1.4/(1000/global->fps)+1;
                        }
                        last_focus = AFdata->focus;
                    }
                    else
                    {
                        AFdata->focus_wait--;
                        //if (global->debug) g_printf("Wait Frame: %d\n",AFdata->focus_wait);
                    }
                }
            }
        }
        /*------------------------- Filter Frame ---------------------------------*/
        __LOCK_MUTEX(__GMUTEX);
        if(global->Frame_Flags>0)
        {
            if((global->Frame_Flags & YUV_PARTICLES)==YUV_PARTICLES)
                particles = particles_effect(videoIn->framebuffer, width, height, 20, 4, particles);

            if((global->Frame_Flags & YUV_MIRROR)==YUV_MIRROR)
                yuyv_mirror(videoIn->framebuffer, width, height);

            if((global->Frame_Flags & YUV_UPTURN)==YUV_UPTURN)
                yuyv_upturn(videoIn->framebuffer, width, height);

            if((global->Frame_Flags & YUV_NEGATE)==YUV_NEGATE)
                yuyv_negative (videoIn->framebuffer, width, height);

            if((global->Frame_Flags & YUV_MONOCR)==YUV_MONOCR)
                yuyv_monochrome (videoIn->framebuffer, width, height);

            if((global->Frame_Flags & YUV_PIECES)==YUV_PIECES)
                pieces (videoIn->framebuffer, width, height, 16 );

        }
        __UNLOCK_MUTEX(__GMUTEX);
        /*-------------------------capture Image----------------------------------*/
        if (videoIn->capImage)
        {
            /*
            * format and resolution can change(enabled) while capturing the frame
            * but you would need to be speedy gonzalez to press two buttons
            * at almost the same time :D
            */
            int ret = 0;
            if((ret=store_picture(all_data)) < 0)
                g_printerr("saved image to:%s ...Failed \n",videoIn->ImageFName);
            else if (!ret && global->debug) g_printf("saved image to:%s ...OK \n",videoIn->ImageFName);

            videoIn->capImage=FALSE;
        }


        /* decrease skip frame count */
        if (global->skip_n > 0)
        {
            //if (global->debug && capVid) g_printf("skiping frame %d...\n", global->skip_n);
            global->skip_n--;
        }

        /*------------------------- Display Frame --------------------------------*/
        if(!global->no_display)
        {
            SDL_LockYUVOverlay(overlay);
            memcpy(p, videoIn->framebuffer, width * height * 2);
            SDL_UnlockYUVOverlay(overlay);

            if (global->foreign_window) {
                // #1417 resize scaling
                // 每4張檢查一次外部視窗的尺寸
                if (size_renew_count >= 3) {
                    gdk_window_get_geometry(global->foreign_window,
                        &global->geometry.x,
                        &global->geometry.y,
                        &global->geometry.width,
                        &global->geometry.height,
                        &global->geometry.depth
                    );

                    if (drect.w != global->geometry.width ||
                        drect.h != global->geometry.height
                    ) { // size changed
                        drect.w = global->geometry.width;
                        drect.h = global->geometry.height;

                        pscreen = SDL_SetVideoMode(
                                global->geometry.width,
                                global->geometry.height,
                                global->bpp,
                                get_sdl_video_flags(global));
                    }
                    size_renew_count = 0;
                }
                else {
                    ++size_renew_count;
                }
            }

            SDL_DisplayYUVOverlay(overlay, &drect);

            /*------------------------- Read events ------------------------------*/
            /* Poll for events */
            while( SDL_PollEvent(&event) )
            {
                //printf("event type:%i  event key:%i\n", event.type, event.key.keysym.scancode);
                if(event.type==SDL_KEYDOWN)
                {
                    if (event.key.keysym.sym == SDLK_ESCAPE)
                    {
                        SDL_Quit();
                        exit(0);
                    }

                    if (videoIn->PanTilt)
                    {
                        switch( event.key.keysym.sym )
                        {
                            /* Keyboard event */
                            /* Pass the event data onto PrintKeyInfo() */
                            /*
                            Notice. SDL事件只有在SDL獨立視窗時可處理。
                            如果指定了SDL_WINDOWID，則事件轉交 GTK 負責，此處不作用。
                            */
                            case SDLK_DOWN:
                                /*Tilt Down*/
                                //uvcPanTilt (videoIn->fd, s->control_list, 0, 1);
                                break;

                            case SDLK_UP:
                                /*Tilt UP*/
                                //uvcPanTilt (videoIn->fd, s->control_list, 0, -1);
                                break;

                            case SDLK_LEFT:
                                /*Pan Left*/
                                //uvcPanTilt (videoIn->fd, s->control_list, 1, 1);
                                break;

                            case SDLK_RIGHT:
                                /*Pan Right*/
                                //uvcPanTilt (videoIn->fd, s->control_list, 1, -1);
                                break;
                            default:
                                break;
                        }
                    }
                    switch( event.key.keysym.scancode )
                    {
                        case 220: /*webcam button*/
                            // TODO how to do?
                            break;
                    }
                } // end event.type==SDL_KEYDOWN
                else if (event.type == SDL_VIDEORESIZE)
                {
                    //g_print("SDL resize: %d, %d\n", event.resize.w, event.resize.h);
                    // NOTICE: This will not raise when environ SDL_WINDOWID set (foreign_window).
                    //if (!global->foreign_window) {
// #ifdef USE_GDK
                    if (global->inner_window) {
                        // change overlay rectangle.
                        drect.w = event.resize.w;
                        drect.h = event.resize.h;
                        // change video rectangle
                        pscreen = SDL_SetVideoMode(
                                event.resize.w,
                                event.resize.h,
                                global->bpp,
                                get_sdl_video_flags(global));
                    }
// #else
//                     drect.w = event.resize.w;
//                     drect.h = event.resize.h;
//                     SDL_SetVideoMode(
//                         event.resize.w,
//                         event.resize.h,
//                         global->bpp,
//                         get_sdl_video_flags(global));
// #endif
                }
            }
        } // !no_display
        /* if set make the thread sleep - default no sleep (full throttle)*/
        if(global->vid_sleep) sleep_ms(global->vid_sleep);

    }/*loop end*/

    #if 0
    __LOCK_MUTEX(__VMUTEX);
        capVid = videoIn->capVid;
    __UNLOCK_MUTEX(__VMUTEX);
    #endif

    if (global->debug) g_printf("Thread terminated...\n");
    p = NULL;
    if(particles) g_free(particles);
    particles=NULL;

    if (global->debug) g_printf("cleaning Thread allocations: 100%%\n");
    fflush(NULL);//flush all output buffers

    if(!global->no_display)
    {
        SDL_FreeYUVOverlay(overlay);
        // Manual said: The returned surface is freed by SDL_Quit and must not be freed by the caller.
        //SDL_FreeSurface(pscreen);
        SDL_Quit();
    }

    if (global->debug) g_printf("SDL Quit\n");

    global = NULL;
    AFdata = NULL;
    videoIn = NULL;
    return NULL;
}
