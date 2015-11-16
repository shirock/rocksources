/*******************************************************************************#
#           guvcview              http://guvcview.berlios.de                    #
#                                                                               #
#           Paulo Assis <pj.assis@gmail.com>                                    #
#           Nobuhiro Iwamatsu <iwamatsu@nigauri.org>                            #
#                             Add UYVY color support(Macbook iSight)            #
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

#ifndef VIDEO_H
#define VIDEO_H

#include <glib.h>
#include "jpgenc.h"
#include "globals.h"

#ifdef HAS_AVCODEC_H
  #include <avcodec.h>
#else
  #ifdef HAS_LIBAVCODEC_AVCODEC_H
    #include <libavcodec/avcodec.h>
  #else
    #ifdef HAS_FFMPEG_AVCODEC_H
      #include <ffmpeg/avcodec.h>
    #else
      #include <libavcodec/avcodec.h>
    #endif
  #endif
#endif

/* FIX BUG
The following items may not be defined in different version of avcodec.
The followings copies from ffmpeg's libavcodec/avcodec.h.
http://cekirdek.pardus.org.tr/~ismail/ffmpeg-docs/avcodec_8h.html
*/
#define LIBAVCODEC_VER_AT_LEAST(major,minor)  (LIBAVCODEC_VERSION_MAJOR > major || \
                                              (LIBAVCODEC_VERSION_MAJOR == major && \
                                               LIBAVCODEC_VERSION_MINOR >= minor))

#if !LIBAVCODEC_VER_AT_LEAST(53,0)
  #define AV_SAMPLE_FMT_S16 SAMPLE_FMT_S16
  #define AV_SAMPLE_FMT_FLT SAMPLE_FMT_FLT
#endif


#if !LIBAVCODEC_VER_AT_LEAST(54,25)
    #define AV_CODEC_ID_NONE CODEC_ID_NONE
    #define AV_CODEC_ID_MJPEG CODEC_ID_MJPEG
    #define AV_CODEC_ID_MPEG1VIDEO CODEC_ID_MPEG1VIDEO
    #define AV_CODEC_ID_FLV1 CODEC_ID_FLV1
    #define AV_CODEC_ID_WMV1 CODEC_ID_WMV1
    #define AV_CODEC_ID_MPEG2VIDEO CODEC_ID_MPEG2VIDEO
    #define AV_CODEC_ID_MSMPEG4V3 CODEC_ID_MSMPEG4V3
    #define AV_CODEC_ID_MPEG4 CODEC_ID_MPEG4
    #define AV_CODEC_ID_H264 CODEC_ID_H264
    #define AV_CODEC_ID_VP8 CODEC_ID_VP8
    #define AV_CODEC_ID_THEORA CODEC_ID_THEORA

    #define AV_CODEC_ID_PCM_S16LE CODEC_ID_PCM_S16LE
    #define AV_CODEC_ID_MP2 CODEC_ID_MP2
    #define AV_CODEC_ID_MP3 CODEC_ID_MP3
    #define AV_CODEC_ID_AC3 CODEC_ID_AC3
    #define AV_CODEC_ID_AAC CODEC_ID_AAC
    #define AV_CODEC_ID_VORBIS CODEC_ID_VORBIS
#endif

#if 0
#ifdef CODEC_TYPE_VIDEO
#define CODEC_TYPE_VIDEO AVMEDIA_TYPE_VIDEO
#endif
#ifdef CODEC_TYPE_AUDIO
#define CODEC_TYPE_AUDIO AVMEDIA_TYPE_AUDIO
#endif
#endif

#ifndef CODEC_FLAG2_BPYRAMID
#define CODEC_FLAG2_BPYRAMID   0x00000010
#endif
#ifndef CODEC_FLAG2_WPRED
#define CODEC_FLAG2_WPRED   0x00000020
#endif
#ifndef CODEC_FLAG2_FASTPSKIP
#define CODEC_FLAG2_FASTPSKIP   0x00000100
#endif

// http://lists.mplayerhq.hu/pipermail/ffmpeg-devel/2012-April/123454.html
#ifndef avcodec_init
#define avcodec_init avcodec_register_all
#endif
// end FIX BUG

void video_resize(struct GLOBAL *global, int width, int height);
void video_move(struct GLOBAL *global, int x, int y);
void video_show(struct GLOBAL *global);
void video_hide(struct GLOBAL *global);

/* run in a thread (SDL overlay)*/
void *main_loop(void *data);

#endif
