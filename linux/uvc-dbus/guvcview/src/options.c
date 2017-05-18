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

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <glib/gprintf.h>
#include "guvcview.h"
#include "defs.h"
#include "globals.h"
#include "string_utils.h"
#include "v4l2uvc.h"

/*----------------------- read conf (.guvcviewrc(-videoX)) file -----------------------*/
int
readConf(struct GLOBAL *global)
{
    int ret=0;
    GScanner  *scanner;
    GTokenType ttype;
    GScannerConfig config =
    {
        " \t\r\n",                     /* characters to skip */
        G_CSET_a_2_z "_" G_CSET_A_2_Z, /* identifier start */
        G_CSET_a_2_z "_." G_CSET_A_2_Z G_CSET_DIGITS,/* identifier cont. */
        "#\n",                         /* single line comment */
        FALSE,                         /* case_sensitive */
        TRUE,                          /* skip multi-line comments */
        TRUE,                          /* skip single line comments */
        FALSE,                         /* scan multi-line comments */
        TRUE,                          /* scan identifiers */
        TRUE,                          /* scan 1-char identifiers */
        FALSE,                         /* scan NULL identifiers */
        FALSE,                         /* scan symbols */
        FALSE,                         /* scan binary */
        FALSE,                         /* scan octal */
        TRUE,                          /* scan float */
        TRUE,                          /* scan hex */
        FALSE,                         /* scan hex dollar */
        TRUE,                          /* scan single quote strings */
        TRUE,                          /* scan double quote strings */
        TRUE,                          /* numbers to int */
        FALSE,                         /* int to float */
        TRUE,                          /* identifier to string */
        TRUE,                          /* char to token */
        FALSE,                         /* symbol to token */
        FALSE,                         /* scope 0 fallback */
        TRUE                           /* store int64 */
    };

    int fd = g_open (global->confPath, O_RDONLY, 0);

    if (fd < 0 )
        return -1;

    scanner = g_scanner_new (&config);
    g_scanner_input_file (scanner, fd);
    scanner->input_name = global->confPath;

    for (ttype = g_scanner_get_next_token (scanner);
        ttype != G_TOKEN_EOF;
        ttype = g_scanner_get_next_token (scanner))
    {
        if (ttype == G_TOKEN_STRING)
        {
            //printf("reading %s...\n",scanner->value.v_string);
            char *name = g_strdup (scanner->value.v_string);
            ttype = g_scanner_get_next_token (scanner);
            if (ttype != G_TOKEN_EQUAL_SIGN)
            {
                g_scanner_unexp_token (scanner,
                    G_TOKEN_EQUAL_SIGN,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    FALSE);
            }
            else
            {
                ttype = g_scanner_get_next_token (scanner);
                /*check for signed integers*/
                if(ttype == '-')
                {
                    //signal = -1;
                    ttype = g_scanner_get_next_token (scanner);
                }

                if (ttype == G_TOKEN_STRING)
                {
                    /*must check for defaults since ReadOpts runs before ReadConf*/
                    if (g_strcmp0(name,"resolution")==0)
                    {
                        sscanf(scanner->value.v_string,"%ix%i",
                            &(global->width),
                            &(global->height));
                    }
                    else if (g_strcmp0(name,"mode")==0)
                    {
                        /*use fourcc but keep it compatible with luvcview*/
                        if(g_strcmp0(scanner->value.v_string,"yuv") == 0)
                            g_snprintf(global->mode,5,"yuyv");
                        else
                            g_snprintf(global->mode,5,"%s",scanner->value.v_string);
                    }
                    else if (g_strcmp0(name,"fps")==0)
                    {
                        sscanf(scanner->value.v_string,"%i/%i",
                            &(global->fps_num), &(global->fps));
                    }
                    else if (g_strcmp0(name,"streaming_username")==0)
                    {
                        global->streamingName = g_strdup(scanner->value.v_string);
                    }
                    else if (g_strcmp0(name,"streaming_password")==0)
                    {
                        global->streamingPassword = g_strdup(scanner->value.v_string);
                    }
                }
                else if (ttype==G_TOKEN_INT)
                {
                    if (g_strcmp0(name,"stack_size")==0)
                    {
                        global->stack_size = scanner->value.v_int;
                    }
                    else if (g_strcmp0(name,"vid_sleep")==0)
                    {
                        global->vid_sleep = scanner->value.v_int;
                    }
                    else if (g_strcmp0(name,"cap_meth")==0)
                    {
                        global->cap_meth = scanner->value.v_int;
                    }
                    else if (g_strcmp0(name,"fps")==0)
                    {
                        /*parse non-quoted fps values*/
                        int line = g_scanner_cur_line(scanner);

                        global->fps_num = scanner->value.v_int;
                        ttype = g_scanner_peek_next_token (scanner);
                        if(ttype=='/')
                        {
                            /*get '/'*/
                            ttype = g_scanner_get_next_token (scanner);
                            ttype = g_scanner_peek_next_token (scanner);
                            if(ttype==G_TOKEN_INT)
                            {
                                ttype = g_scanner_get_next_token (scanner);
                                global->fps = scanner->value.v_int;
                            }
                            else if (scanner->next_line>line)
                            {
                                /*start new loop*/
                                break;
                            }
                            else
                            {
                                ttype = g_scanner_get_next_token (scanner);
                                g_scanner_unexp_token (scanner,
                                    G_TOKEN_NONE,
                                    NULL,
                                    NULL,
                                    NULL,
                                    "bad value for fps",
                                    FALSE);
                            }
                        }
                    }
                    else if (g_strcmp0(name,"auto_focus")==0)
                    {
                        global->autofocus = scanner->value.v_int;
                    }
                    else if (g_strcmp0(name,"bpp")==0)
                    {
                        global->bpp = scanner->value.v_int;
                    }
                    else if (g_strcmp0(name,"hwaccel")==0)
                    {
                        global->hwaccel = scanner->value.v_int;
                    }
                    else if (g_strcmp0(name,"vid_codec")==0 || (g_strcmp0(name,"avi_format")==0))
                    {
                        global->VidCodec = scanner->value.v_int;
                    }
                    else if (g_strcmp0(name,"frame_flags")==0)
                    {
                        global->Frame_Flags = scanner->value.v_int;
                    }
                }
                else if (ttype==G_TOKEN_FLOAT)
                {
                    printf("unexpected float value (%f) for %s\n", scanner->value.v_float, name);
                }
                else if (ttype==G_TOKEN_CHAR)
                {
                    printf("unexpected char value (%c) for %s\n", scanner->value.v_char, name);
                }
                else
                {
                    g_scanner_unexp_token (scanner,
                        G_TOKEN_NONE,
                        NULL,
                        NULL,
                        NULL,
                        "string values must be quoted - skiping",
                        FALSE);
                    int line = g_scanner_cur_line (scanner);
                    int stp=0;

                    do
                    {
                        ttype = g_scanner_peek_next_token (scanner);
                        if(scanner->next_line > line)
                        {
                            //printf("next line reached\n");
                            stp=1;
                            break;
                        }
                        else
                        {
                            ttype = g_scanner_get_next_token (scanner);
                        }
                    }
                    while (!stp);
                }
            }
            g_free(name);
        }
    }

    g_scanner_destroy (scanner);
    close (fd);

    if (global->debug)
    {
        g_printf("video_device: %s\n",global->videodevice);
        g_printf("cap_meth: %i\n",global->cap_meth);
        g_printf("resolution: %i x %i\n",global->width,global->height);
        g_printf("fps: %i/%i\n",global->fps_num,global->fps);
        g_printf("bpp: %i\n",global->bpp);
        g_printf("hwaccel: %i\n",global->hwaccel);
        g_printf("Video Filter Flags: %i\n",global->Frame_Flags);
    }

    return (ret);
}
