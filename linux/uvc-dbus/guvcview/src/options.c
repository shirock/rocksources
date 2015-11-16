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

/*----------------------- write conf (.guvcviewrc) file ----------------------*/
int
writeConf(struct GLOBAL *global, char *videodevice)
{
	int ret=0;
	FILE *fp;

	// write to tmp file then rename after sucessfull fsync
	// using fsync avois data loss on system crash
	// see: https://bugs.launchpad.net/ubuntu/+source/linux/+bug/317781/comments/54
	char *tmpfile = g_strjoin (".", global->confPath, "tmp", NULL);
	if ((fp = g_fopen(tmpfile,"w"))!=NULL)
	{
		g_fprintf(fp,"# guvcview configuration file for %s\n\n",videodevice);
		g_fprintf(fp,"# Thread stack size: default 128 pages of 64k = 8388608 bytes\n");
		g_fprintf(fp,"stack_size=%d\n",global->stack_size);
		g_fprintf(fp,"# video loop sleep time in ms: 0,1,2,3,...\n");
		g_fprintf(fp,"# increased sleep time -> less cpu load, more droped frames\n");
		g_fprintf(fp,"vid_sleep=%i\n",global->vid_sleep);
		g_fprintf(fp,"# capture method: 1- mmap 2- read\n");
		g_fprintf(fp,"cap_meth=%i\n",global->cap_meth);
		g_fprintf(fp,"# video resolution \n");
		g_fprintf(fp,"resolution='%ix%i'\n",global->width,global->height);
		g_fprintf(fp,"# control window size: default %ix%i\n",WINSIZEX,WINSIZEY);
		g_fprintf(fp,"windowsize='%ix%i'\n",global->winwidth,global->winheight);
		g_fprintf(fp,"#vertical pane size\n");
		g_fprintf(fp,"vpane=%i\n",global->boxvsize);
		g_fprintf(fp,"#spin button behavior: 0-non editable 1-editable\n");
		g_fprintf(fp,"spinbehave=%i\n", global->spinbehave);
		g_fprintf(fp,"# mode video format 'yuvy' 'yvyu' 'uyvy' 'yyuv' 'yu12' 'yv12' 'nv12' 'nv21' 'nv16' 'nv61' 'y41p' 'grey' 's501' 's505' 's508' 'gbrg' 'grbg' 'ba81' 'rggb' 'rgb3' 'bgr3' 'jpeg' 'mjpg'(default)\n");
		g_fprintf(fp,"mode='%s'\n",global->mode);
		g_fprintf(fp,"# frames per sec. - hardware supported - default( %i )\n",DEFAULT_FPS);
		g_fprintf(fp,"fps='%d/%d'\n",global->fps_num,global->fps);
		g_fprintf(fp,"#Display Fps counter: 1- Yes 0- No\n");
		g_fprintf(fp,"fps_display=%i\n",global->FpsCount);
		g_fprintf(fp,"#auto focus (continuous): 1- Yes 0- No\n");
		g_fprintf(fp,"auto_focus=%i\n",global->autofocus);
		g_fprintf(fp,"# bytes per pixel: default (0 - current)\n");
		g_fprintf(fp,"bpp=%i\n",global->bpp);
		g_fprintf(fp,"# hardware accelaration: 0 1 (default - 1)\n");
		g_fprintf(fp,"hwaccel=%i\n",global->hwaccel);
		g_fprintf(fp,"# video compression format: 0-MJPG 1-YUY2/UYVY 2-DIB (BMP 24) 3-MPEG1 4-FLV1 5-MPEG2 6-MS MPEG4 V3(DIV3) 7-MPEG4 (DIV5)\n");
		g_fprintf(fp,"vid_codec=%i\n",global->VidCodec);
		g_fprintf(fp,"# Auto Video naming (ex: filename-n.avi)\n");
		g_fprintf(fp,"vid_inc=%d\n",global->vid_inc);
//		g_fprintf(fp,"# sound 0 - disable 1 - enable\n");
//		g_fprintf(fp,"sound=%i\n",global->Sound_enable);
		g_fprintf(fp,"# video filters: 0 -none 1- flip 2- upturn 4- negate 8- mono (add the ones you want)\n");
		g_fprintf(fp,"frame_flags=%i\n",global->Frame_Flags);
		g_fprintf(fp,"# Image capture Full Path\n");
		g_fprintf(fp,"image_path='%s/%s'\n",global->imgFPath[1],global->imgFPath[0]);
		g_fprintf(fp,"# Auto Image naming (filename-n.ext)\n");
		g_fprintf(fp,"image_inc=%d\n",global->image_inc);
		g_fprintf(fp,"# Video capture Full Path\n");
		g_fprintf(fp,"video_path='%s/%s'\n",global->vidFPath[1],global->vidFPath[0]);
		printf("write %s OK\n",global->confPath);

		//flush stream buffers to filesystem
		fflush(fp);

		//close file after fsync (sync file data to disk)
		if (fsync(fileno(fp)) || fclose(fp))
		{
			perror("error writing configuration data to file");
			ret=-1;
		}
		else
		{
			//rename from tmp to real name
			g_rename (tmpfile, global->confPath);
		}
	}
	else
	{
		g_printerr("Could not write file %s \n Please check file permissions\n",tmpfile);
		ret=-2;
	}
	g_free(tmpfile);

	return ret;
}

/*----------------------- read conf (.guvcviewrc(-videoX)) file -----------------------*/
int
readConf(struct GLOBAL *global)
{
	int ret=0;
//	int signal=1; /*1=>+ or -1=>-*/
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
	{
		printf("Could not open %s for read,\n will try to create it\n",global->confPath);
		ret=writeConf(global, global->videodevice);
	}
	else
	{
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
						//signal=1; /*reset signal*/
						//if (g_strcmp0(name,"video_device")==0)
						//{
						//	g_snprintf(global->videodevice,15,"%s",scanner->value.v_string);
						//}

						/*must check for defaults since ReadOpts runs before ReadConf*/
						if (g_strcmp0(name,"resolution")==0)
						{
							if(global->flg_res < 1)
								sscanf(scanner->value.v_string,"%ix%i",
									&(global->width),
									&(global->height));
						}
						else if (g_strcmp0(name,"windowsize")==0)
						{
							sscanf(scanner->value.v_string,"%ix%i",
								&(global->winwidth), &(global->winheight));
						}
						else if (g_strcmp0(name,"mode")==0)
						{
							if(global->flg_mode < 1)
							{
								/*use fourcc but keep it compatible with luvcview*/
								if(g_strcmp0(scanner->value.v_string,"yuv") == 0)
									g_snprintf(global->mode,5,"yuyv");
								else
									g_snprintf(global->mode,5,"%s",scanner->value.v_string);
							}
						}
						else if (g_strcmp0(name,"fps")==0)
						{
							sscanf(scanner->value.v_string,"%i/%i",
								&(global->fps_num), &(global->fps));
						}
#if 0
						else if (g_strcmp0(name,"image_path")==0)
						{
							if(global->flg_imgFPath < 1)
							{
								global->imgFPath = splitPath(scanner->value.v_string,global->imgFPath);
								/*get the file type*/
								global->imgFormat = check_image_type(global->imgFPath[0]);
							}
						}
						else if ((g_strcmp0(name,"video_path")==0) || (g_strcmp0(name,"avi_path")==0))
						{
							if(global->vidfile == NULL)
							{
								global->vidFPath=splitPath(scanner->value.v_string,global->vidFPath);
								/*get the file type (0-avi 1-matroska)*/
								global->VidFormat = check_video_type(global->vidFPath[0]);
							}
						}
						else
						{
							printf("unexpected string value (%s) for %s\n",
								scanner->value.v_string, name);
						}
#endif
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
							if(!(global->flg_cap_meth))
								global->cap_meth = scanner->value.v_int;
						}
						else if (g_strcmp0(name,"vpane")==0)
						{
							global->boxvsize = scanner->value.v_int;
						}
						else if (g_strcmp0(name,"spinbehave")==0)
						{
							global->spinbehave = scanner->value.v_int;
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
						else if (strcmp(name,"fps_display")==0)
						{
							if(global->flg_FpsCount < 1)
								global->FpsCount = (short) scanner->value.v_int;
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
							if(global->flg_hwaccel < 1)
								global->hwaccel = scanner->value.v_int;
						}
						else if (g_strcmp0(name,"vid_codec")==0 || (g_strcmp0(name,"avi_format")==0))
						{
							global->VidCodec = scanner->value.v_int;
						}
						else if ((g_strcmp0(name,"vid_inc")==0) || (g_strcmp0(name,"avi_inc")==0))
						{
							global->vid_inc = (DWORD) scanner->value.v_int;
							g_snprintf(global->vidinc_str,20,_("File num:%d"),global->vid_inc);
						}
						else if (g_strcmp0(name,"frame_flags")==0)
						{
							global->Frame_Flags = scanner->value.v_int;
						}
						else if (g_strcmp0(name,"image_inc")==0)
						{
							if(global->image_timer <= 0)
							{
								global->image_inc = (DWORD) scanner->value.v_int;
								g_snprintf(global->imageinc_str,20,_("File num:%d"),global->image_inc);
							}
						}
						/*
						else
						{
							printf("unexpected integer value (%lu) for %s\n",
								scanner->value.v_int, name);
							printf("Strings must be quoted\n");
						}
						*/
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
//			g_printf("vid_sleep: %i\n",global->vid_sleep);
			g_printf("cap_meth: %i\n",global->cap_meth);
			g_printf("resolution: %i x %i\n",global->width,global->height);
//			g_printf("windowsize: %i x %i\n",global->winwidth,global->winheight);
//			g_printf("vert pane: %i\n",global->boxvsize);
//			g_printf("spin behavior: %i\n",global->spinbehave);
//			g_printf("mode: %s\n",global->mode);
			g_printf("fps: %i/%i\n",global->fps_num,global->fps);
			g_printf("Display Fps: %i\n",global->FpsCount);
			g_printf("bpp: %i\n",global->bpp);
			g_printf("hwaccel: %i\n",global->hwaccel);
//			g_printf("avi_format: %i\n",global->VidCodec);
			g_printf("Pan Step: %i degrees\n",global->PanStep);
			g_printf("Tilt Step: %i degrees\n",global->TiltStep);
			g_printf("Video Filter Flags: %i\n",global->Frame_Flags);
//			g_printf("image inc: %d\n",global->image_inc);
		}
	}

	return (ret);
}
