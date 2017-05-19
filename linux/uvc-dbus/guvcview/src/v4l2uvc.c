/*******************************************************************************#
#           guvcview              http://guvcview.berlios.de                    #
#                                                                               #
#           Paulo Assis <pj.assis@gmail.com>                                    #
#           rock <shirock.tw@gmail.com> Add snapshot and Fix MMAP bug.          #
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

/*******************************************************************************#
#                                                                               #
#  V4L2 interface                                                               #
#                                                                               #
********************************************************************************/

#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <libv4l2.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <errno.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>

#include "guvcview.h"
#include "v4l2uvc.h"
#include "v4l2_dyna_ctrls.h"
#include "utils.h"
#include "picture.h"
#include "colorspaces.h"
#include "ms_time.h"

#define __VMUTEX &vd->mutex

/* needed only for language files (not used)*/

// V4L2 control strings
#define CSTR_USER_CLASS		N_("User Controls")
#define	CSTR_BRIGHT 		N_("Brightness")
#define	CSTR_CONTRAST 		N_("Contrast")
#define	CSTR_HUE 		N_("Hue")
#define	CSTR_SATURAT		N_("Saturation")
#define	CSTR_SHARP		N_("Sharpness")
#define	CSTR_GAMMA		N_("Gamma")
#define	CSTR_BLCOMP		N_("Backlight Compensation")
#define	CSTR_PLFREQ		N_("Power Line Frequency")
#define CSTR_HUEAUTO		N_("Hue, Automatic")
#define	CSTR_FOCUSAUTO		N_("Focus, Auto")
#define CSTR_EXPMENU1		N_("Manual Mode")
#define CSTR_EXPMENU2		N_("Auto Mode")
#define CSTR_EXPMENU3		N_("Shutter Priority Mode")
#define CSTR_EXPMENU4		N_("Aperture Priority Mode")
#define CSTR_BLACK_LEVEL	N_("Black Level")
#define CSTR_AUTO_WB		N_("White Balance, Automatic")
#define CSTR_DO_WB		N_("Do White Balance")
#define CSTR_RB			N_("Red Balance")
#define	CSTR_BB			N_("Blue Balance")
#define CSTR_EXP		N_("Exposure")
#define CSTR_AUTOGAIN		N_("Gain, Automatic")
#define	CSTR_GAIN		N_("Gain")
#define CSTR_HFLIP		N_("Horizontal Flip")
#define CSTR_VFLIP		N_("Vertical Flip")
#define CSTR_HCENTER		N_("Horizontal Center")
#define CSTR_VCENTER		N_("Vertical Center")
#define CSTR_CHR_AGC		N_("Chroma AGC")
#define CSTR_CLR_KILL		N_("Color Killer")
#define CSTR_COLORFX		N_("Color Effects")

// CAMERA CLASS control strings
#define CSTR_CAMERA_CLASS	N_("Camera Controls")
#define CSTR_EXPAUTO		N_("Auto Exposure")
#define	CSTR_EXPABS		    N_("Exposure Time, Absolute")
#define CSTR_EXPAUTOPRI		N_("Exposure, Dynamic Framerate")
#define	CSTR_PAN_REL		N_("Pan, Relative")
#define CSTR_TILT_REL		N_("Tilt, Relative")
#define CSTR_PAN_RESET		N_("Pan, Reset")
#define CSTR_TILT_RESET		N_("Tilt, Reset")
#define CSTR_PAN_ABS		N_("Pan, Absolute")
#define CSTR_TILT_ABS		N_"Tilt, Absolute")
#define CSTR_FOCUS_ABS		N_("Focus, Absolute")
#define CSTR_FOCUS_REL		N_("Focus, Relative")
#define CSTR_FOCUS_AUTO		N_("Focus, Automatic")
#define CSTR_ZOOM_ABS		N_("Zoom, Absolute")
#define CSTR_ZOOM_REL		N_("Zoom, Relative")
#define CSTR_ZOOM_CONT		N_("Zoom, Continuous")
#define CSTR_PRIV		N_("Privacy")

//UVC specific control strings
#define	CSTR_EXPAUTO_UVC	N_("Exposure, Auto")
#define	CSTR_EXPAUTOPRI_UVC	N_("Exposure, Auto Priority")
#define	CSTR_EXPABS_UVC		N_("Exposure (Absolute)")
#define	CSTR_WBTAUTO_UVC	N_("White Balance Temperature, Auto")
#define	CSTR_WBT_UVC		N_("White Balance Temperature")
#define CSTR_WBCAUTO_UVC	N_("White Balance Component, Auto")
#define CSTR_WBCB_UVC		N_("White Balance Blue Component")
#define	CSTR_WBCR_UVC		N_("White Balance Red Component")

//libwebcam specific control strings
#define CSTR_FOCUS_LIBWC	N_("Focus")
#define CSTR_FOCUSABS_LIBWC	N_("Focus (Absolute)")


/* ioctl with a number of retries in the case of failure
* args:
* fd - device descriptor
* IOCTL_X - ioctl reference
* arg - pointer to ioctl data
* returns - ioctl result
*/
int xioctl(int fd, int IOCTL_X, void *arg)
{
	int ret = 0;
	int tries= IOCTL_RETRY;
	do
	{
		ret = v4l2_ioctl(fd, IOCTL_X, arg);
	}
	while (ret && tries-- &&
			((errno == EINTR) || (errno == EAGAIN) || (errno == ETIMEDOUT)));

	if (ret && (tries <= 0)) g_printerr("ioctl (%i) retried %i times - giving up: %s)\n", IOCTL_X, IOCTL_RETRY, strerror(errno));

	return (ret);
}

/* Query video device capabilities and supported formats
 * args:
 * vd: pointer to a VdIn struct ( must be allready allocated )
 *
 * returns: error code  (0- OK)
*/
static int check_videoIn(struct vdIn *vd, int *width, int *height)
{
	if (vd == NULL)
		return VDIN_ALLOC_ERR;

	memset(&vd->cap, 0, sizeof(struct v4l2_capability));

	if ( xioctl(vd->fd, VIDIOC_QUERYCAP, &vd->cap) < 0 )
	{
		perror("VIDIOC_QUERYCAP error");
		return VDIN_QUERYCAP_ERR;
	}

	if ( ( vd->cap.capabilities & V4L2_CAP_VIDEO_CAPTURE ) == 0)
	{
		g_printerr("Error opening device %s: video capture not supported.\n",
				vd->videodevice);
		return VDIN_QUERYCAP_ERR;
	}
	if (!(vd->cap.capabilities & V4L2_CAP_STREAMING))
	{
		g_printerr("%s does not support streaming i/o\n",
			vd->videodevice);
		return VDIN_QUERYCAP_ERR;
	}

	if(vd->cap_meth == IO_READ)
	{

		vd->mem[vd->buf.index] = NULL;
		if (!(vd->cap.capabilities & V4L2_CAP_READWRITE))
		{
			g_printerr("%s does not support read i/o\n",
				vd->videodevice);
			return VDIN_READ_ERR;
		}
	}
	g_printf("Init. %s (location: %s)\n", vd->cap.card, vd->cap.bus_info);

	vd->listFormats = enum_frame_formats( width, height, vd->fd);

	if(!(vd->listFormats->listVidFormats))
		g_printerr("Couldn't detect any supported formats on your device (%i)\n", vd->listFormats->numb_formats);
	return VDIN_OK;
}

static int unmap_buff(struct vdIn *vd)
{
	int i=0;
	int ret=0;

	switch(vd->cap_meth)
	{
		case IO_READ:
			break;

		case IO_MMAP:
			for (i = 0; i < NB_BUFFER; i++)
			{
				// unmap old buffer
				if((vd->mem[i] != MAP_FAILED) && vd->buff_length[i])
					if((ret=v4l2_munmap(vd->mem[i], vd->buff_length[i]))<0)
					{
						perror("couldn't unmap buff");
					}
			}
	}
	return ret;
}

static int map_buff(struct vdIn *vd)
{
	int i = 0;
	// map new buffer
	for (i = 0; i < NB_BUFFER; i++)
	{
		vd->mem[i] = v4l2_mmap( NULL, // start anywhere
			vd->buff_length[i],
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			vd->fd,
			vd->buff_offset[i]);
		if (vd->mem[i] == MAP_FAILED)
		{
			perror("Unable to map buffer");
			return VDIN_MMAP_ERR;
		}
	}

	return (0);
}

/* Query and map buffers
 * args:
 * vd: pointer to a VdIn struct ( must be allready allocated )
 * setUNMAP: ( flag )if set unmap old buffers first
 *
 * returns: error code  (0- OK)
*/
static int query_buff(struct vdIn *vd)
{
	int i=0;
	int ret=0;

	switch(vd->cap_meth)
	{
		case IO_READ:
			break;

		case IO_MMAP:
			for (i = 0; i < NB_BUFFER; i++)
			{
				memset(&vd->buf, 0, sizeof(struct v4l2_buffer));
				vd->buf.index = i;
				vd->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				//vd->buf.flags = V4L2_BUF_FLAG_TIMECODE;
				//vd->buf.timecode = vd->timecode;
				//vd->buf.timestamp.tv_sec = 0;//get frame as soon as possible
				//vd->buf.timestamp.tv_usec = 0;
				vd->buf.memory = V4L2_MEMORY_MMAP;
				ret = xioctl(vd->fd, VIDIOC_QUERYBUF, &vd->buf);
				if (ret < 0)
				{
					perror("VIDIOC_QUERYBUF - Unable to query buffer");
					if(errno == EINVAL)
					{
						g_printerr("trying with read method instead\n");
						vd->cap_meth = IO_READ;
					}
					return VDIN_QUERYBUF_ERR;
				}
				if (vd->buf.length <= 0)
					g_printerr("WARNING VIDIOC_QUERYBUF - buffer length is %d\n",
						vd->buf.length);

				vd->buff_length[i] = vd->buf.length;
				vd->buff_offset[i] = vd->buf.m.offset;
			}
			// map the new buffers
			if(map_buff(vd) != 0)
				return VDIN_MMAP_ERR;
	}
	return VDIN_OK;
}

/* Queue Buffers
 * args:
 * vd: pointer to a VdIn struct ( must be allready allocated )
 *
 * returns: error code  (0- OK)
*/
static int queue_buff(struct vdIn *vd)
{
	int i=0;
	int ret=0;
	switch(vd->cap_meth)
	{
		case IO_READ:
			break;

		case IO_MMAP:
		default:
			for (i = 0; i < NB_BUFFER; ++i)
			{
				memset(&vd->buf, 0, sizeof(struct v4l2_buffer));
				vd->buf.index = i;
				vd->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				//vd->buf.flags = V4L2_BUF_FLAG_TIMECODE;
				//vd->buf.timecode = vd->timecode;
				//vd->buf.timestamp.tv_sec = 0;//get frame as soon as possible
				//vd->buf.timestamp.tv_usec = 0;
				vd->buf.memory = V4L2_MEMORY_MMAP;
				ret = xioctl(vd->fd, VIDIOC_QBUF, &vd->buf);
				if (ret < 0)
				{
					perror("VIDIOC_QBUF - Unable to queue buffer");
					return VDIN_QBUF_ERR;
				}
			}
			vd->buf.index = 0; /*reset index*/
	}
	return VDIN_OK;
}

/* Enable video stream
 * args:
 * vd: pointer to a VdIn struct ( must be allready initiated)
 *
 * returns: VIDIOC_STREAMON ioctl result (0- OK)
*/
int video_enable(struct vdIn *vd)
{
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	int ret=0;
	switch(vd->cap_meth)
	{
		case IO_READ:
			//do nothing
			break;

		case IO_MMAP:
		default:
			ret = xioctl(vd->fd, VIDIOC_STREAMON, &type);
			if (ret < 0)
			{
				perror("VIDIOC_STREAMON - Unable to start capture");
				return VDIN_STREAMON_ERR;
			}
			break;
	}
	vd->isstreaming = 1;
	return 0;
}

/* Disable video stream
 * args:
 * vd: pointer to a VdIn struct ( must be allready initiated)
 *
 * returns: VIDIOC_STREAMOFF ioctl result (0- OK)
*/
int video_disable(struct vdIn *vd)
{
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	int ret=0;
	switch(vd->cap_meth)
	{
		case IO_READ:
			//do nothing
			break;

		case IO_MMAP:
		default:
			ret = xioctl(vd->fd, VIDIOC_STREAMOFF, &type);
			if (ret < 0)
			{
				perror("VIDIOC_STREAMOFF - Unable to stop capture");
				if(errno == 9) vd->isstreaming = 0;/*capture as allready stoped*/
				return VDIN_STREAMOFF_ERR;
			}
			break;
	}
	vd->isstreaming = 0;
	return 0;
}

/* gets video stream jpeg compression parameters
 * args:
 * vd: pointer to a VdIn struct ( must be allready initiated)
 *
 * returns: VIDIOC_G_JPEGCOMP ioctl result value
*/
int get_jpegcomp(struct vdIn *vd)
{
	int ret = xioctl(vd->fd, VIDIOC_G_JPEGCOMP, &vd->jpgcomp);
	if(!ret)
	{
		g_printf("VIDIOC_G_COMP:\n");
		g_printf("    quality:      %i\n", vd->jpgcomp.quality);
		g_printf("    APPn:         %i\n", vd->jpgcomp.APPn);
		g_printf("    APP_len:      %i\n", vd->jpgcomp.APP_len);
		g_printf("    APP_data:     %s\n", vd->jpgcomp.APP_data);
		g_printf("    COM_len:      %i\n", vd->jpgcomp.COM_len);
		g_printf("    COM_data:     %s\n", vd->jpgcomp.COM_data);
		g_printf("    jpeg_markers: 0x%x\n", vd->jpgcomp.jpeg_markers);
	}
	else
	{
		perror("VIDIOC_G_COMP:");
		if(errno == EINVAL)
		{
			vd->jpgcomp.quality = -1; //not supported
			g_printf("   compression control not supported\n");
		}
	}

	return (ret);
}

/* sets video stream jpeg compression parameters
 * args:
 * vd: pointer to a VdIn struct ( must be allready initiated)
 *
 * returns: VIDIOC_S_JPEGCOMP ioctl result value
*/
int set_jpegcomp(struct vdIn *vd)
{
	int ret = xioctl(vd->fd, VIDIOC_S_JPEGCOMP, &vd->jpgcomp);
	if(ret != 0)
	{
		perror("VIDIOC_S_COMP:");
		if(errno == EINVAL)
		{
			vd->jpgcomp.quality = -1; //not supported
			g_printf("   compression control not supported\n");
		}
	}

	return (ret);
}

/* Try/Set device video stream format
 * args:
 * vd: pointer to a VdIn struct ( must be allready allocated )
 *
 * returns: error code ( 0 - VDIN_OK)
*/
static int init_v4l2(struct vdIn *vd, int *format, int *width, int *height, int *fps, int *fps_num)
{
	int ret = 0;

	// make sure we set a valid format
	g_print("checking format: %i\n", *format);
	if ((ret=check_SupPixFormat(*format)) < 0)
	{
		// not available - Fail so we can check other formats (don't bother trying it)
		g_printerr("Format unavailable: %d.\n",*format);
		return VDIN_FORMAT_ERR;
	}

	vd->timestamp = 0;
	// set format
	vd->fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	vd->fmt.fmt.pix.width = *width;
	vd->fmt.fmt.pix.height = *height;
	vd->fmt.fmt.pix.pixelformat = *format;
	vd->fmt.fmt.pix.field = V4L2_FIELD_ANY;

	ret = xioctl(vd->fd, VIDIOC_S_FMT, &vd->fmt);
	if (ret < 0)
	{
		perror("VIDIOC_S_FORMAT - Unable to set format");
		return VDIN_FORMAT_ERR;
	}
	if ((vd->fmt.fmt.pix.width != *width) ||
		(vd->fmt.fmt.pix.height != *height))
	{
		g_printerr("Requested Format unavailable: get width %d height %d \n",
		vd->fmt.fmt.pix.width, vd->fmt.fmt.pix.height);
		*width = vd->fmt.fmt.pix.width;
		*height = vd->fmt.fmt.pix.height;
	}

	//deprecated in v4l2 - still waiting for new API implementation
	if(*format == V4L2_PIX_FMT_MJPEG || *format == V4L2_PIX_FMT_JPEG)
	{
		get_jpegcomp(vd);
	}

	/* ----------- FPS --------------*/
	input_set_framerate(vd, fps, fps_num);

	switch (vd->cap_meth)
	{
		case IO_READ: //allocate buffer for read
			memset(&vd->buf, 0, sizeof(struct v4l2_buffer));
			vd->buf.length = (*width) * (*height) * 3; //worst case (rgb)
			vd->mem[vd->buf.index] = g_new0(BYTE, vd->buf.length);
			break;

		case IO_MMAP:
		default:
			// request buffers
			memset(&vd->rb, 0, sizeof(struct v4l2_requestbuffers));
			vd->rb.count = NB_BUFFER;
			vd->rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			vd->rb.memory = V4L2_MEMORY_MMAP;

			ret = xioctl(vd->fd, VIDIOC_REQBUFS, &vd->rb);
			if (ret < 0)
			{
				perror("VIDIOC_REQBUFS - Unable to allocate buffers");
				return VDIN_REQBUFS_ERR;
			}
			// map the buffers
			if (query_buff(vd))
			{
				//delete requested buffers
				//no need to unmap as mmap failed for sure
				memset(&vd->rb, 0, sizeof(struct v4l2_requestbuffers));
				vd->rb.count = 0;
				vd->rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				vd->rb.memory = V4L2_MEMORY_MMAP;
				if(xioctl(vd->fd, VIDIOC_REQBUFS, &vd->rb)<0)
					perror("VIDIOC_REQBUFS - Unable to delete buffers");
				return VDIN_QUERYBUF_ERR;
			}
			// Queue the buffers
			if (queue_buff(vd))
			{
				//delete requested buffers
				unmap_buff(vd);
				memset(&vd->rb, 0, sizeof(struct v4l2_requestbuffers));
				vd->rb.count = 0;
				vd->rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				vd->rb.memory = V4L2_MEMORY_MMAP;
				if(xioctl(vd->fd, VIDIOC_REQBUFS, &vd->rb)<0)
					perror("VIDIOC_REQBUFS - Unable to delete buffers");
				return VDIN_QBUF_ERR;
			}
	}

	return VDIN_OK;
}

/* Alloc image buffers for decoding video stream
 * args:
 * vd: pointer to a VdIn struct ( must be allready allocated )
 *
 * returns: error code ( 0 - VDIN_OK)
*/
static int videoIn_frame_alloca(struct vdIn *vd, int format, int width, int height)
{
	int ret = VDIN_OK;
	size_t framebuf_size=0;
	size_t tmpbuf_size=0;

	int framesizeIn = (width * height << 1); //2 bytes per pixel
	switch (format)
	{
		case V4L2_PIX_FMT_JPEG:
		case V4L2_PIX_FMT_MJPEG:
			// alloc a temp buffer to reconstruct the pict (MJPEG)
			tmpbuf_size= framesizeIn;
			vd->tmpbuffer = g_new0(unsigned char, tmpbuf_size);

			framebuf_size = width * (height + 8) * 2;
			vd->framebuffer = g_new0(unsigned char, framebuf_size);
			break;

		case V4L2_PIX_FMT_UYVY:
		case V4L2_PIX_FMT_YVYU:
		case V4L2_PIX_FMT_YYUV:
		case V4L2_PIX_FMT_YUV420: // only needs 3/2 bytes per pixel but we alloc 2 bytes per pixel
		case V4L2_PIX_FMT_YVU420: // only needs 3/2 bytes per pixel but we alloc 2 bytes per pixel
		case V4L2_PIX_FMT_Y41P:   // only needs 3/2 bytes per pixel but we alloc 2 bytes per pixel
		case V4L2_PIX_FMT_NV12:
		case V4L2_PIX_FMT_NV21:
		case V4L2_PIX_FMT_NV16:
		case V4L2_PIX_FMT_NV61:
		case V4L2_PIX_FMT_SPCA501:
		case V4L2_PIX_FMT_SPCA505:
		case V4L2_PIX_FMT_SPCA508:
			// alloc a temp buffer for converting to YUYV
			tmpbuf_size= framesizeIn;
			vd->tmpbuffer = g_new0(unsigned char, tmpbuf_size);
			framebuf_size = framesizeIn;
			vd->framebuffer = g_new0(unsigned char, framebuf_size);
			break;

		case V4L2_PIX_FMT_GREY:
			// alloc a temp buffer for converting to YUYV
			tmpbuf_size= width * height; // 1 byte per pixel
			vd->tmpbuffer = g_new0(unsigned char, tmpbuf_size);
			framebuf_size = framesizeIn;
			vd->framebuffer = g_new0(unsigned char, framebuf_size);
			break;

        case V4L2_PIX_FMT_Y16:
            // alloc a temp buffer for converting to YUYV
            tmpbuf_size= width * height * 2; // 2 byte per pixel
            vd->tmpbuffer = g_new0(unsigned char, tmpbuf_size);
            framebuf_size = framesizeIn;
            vd->framebuffer = g_new0(unsigned char, framebuf_size);
            break;

		case V4L2_PIX_FMT_YUYV:
			//  YUYV doesn't need a temp buffer but we will set it if/when
			//  video processing disable control is checked (bayer processing).
			//            (logitech cameras only)
			framebuf_size = framesizeIn;
			vd->framebuffer = g_new0(unsigned char, framebuf_size);
			break;

		case V4L2_PIX_FMT_SGBRG8: //0
		case V4L2_PIX_FMT_SGRBG8: //1
		case V4L2_PIX_FMT_SBGGR8: //2
		case V4L2_PIX_FMT_SRGGB8: //3
			// Raw 8 bit bayer
			// when grabbing use:
			//    bayer_to_rgb24(bayer_data, RGB24_data, width, height, 0..3)
			//    rgb2yuyv(RGB24_data, vd->framebuffer, width, height)

			// alloc a temp buffer for converting to YUYV
			// rgb buffer for decoding bayer data
			tmpbuf_size = width * height * 3;
			vd->tmpbuffer = g_new0(unsigned char, tmpbuf_size);

			framebuf_size = framesizeIn;
			vd->framebuffer = g_new0(unsigned char, framebuf_size);
			break;
		case V4L2_PIX_FMT_RGB24:
		case V4L2_PIX_FMT_BGR24:
			//rgb or bgr (8-8-8)
			// alloc a temp buffer for converting to YUYV
			// rgb buffer
			tmpbuf_size = width * height * 3;
			vd->tmpbuffer = g_new0(unsigned char, tmpbuf_size);

			framebuf_size = framesizeIn;
			vd->framebuffer = g_new0(unsigned char, framebuf_size);
			break;

		default:
			g_printerr("(v4l2uvc.c) should never arrive (1)- exit fatal !!\n");
			ret = VDIN_UNKNOWN_ERR;
            g_free(vd->framebuffer);
            vd->framebuffer = NULL;
            g_free(vd->tmpbuffer);
            vd->tmpbuffer = NULL;
            return (ret);
	}

	if ((!vd->framebuffer) || (framebuf_size <=0))
	{
		g_printerr("couldn't calloc %lu bytes of memory for frame buffer\n",
			(unsigned long) framebuf_size);
		ret = VDIN_FBALLOC_ERR;
        g_free(vd->framebuffer);
        vd->framebuffer = NULL;
        g_free(vd->tmpbuffer);
        vd->tmpbuffer = NULL;
        return (ret);
	}
	else
	{
		int i = 0;
		// set framebuffer to black (y=0x00 u=0x80 v=0x80) by default
		for (i=0; i<(framebuf_size-4); i+=4)
			{
				vd->framebuffer[i]=0x00;  //Y
				vd->framebuffer[i+1]=0x80;//U
				vd->framebuffer[i+2]=0x00;//Y
				vd->framebuffer[i+3]=0x80;//V
			}
	}

	vd->framebuffer_size = framebuf_size;
    if (all_data.global->Frame_Flags != 0) {
        // set filter. copy raw frame to snapshot.
        vd->snapshot = g_new0(unsigned char, vd->framebuffer_size);
    }
    else {
        // no filter. use the same memory block.
        vd->snapshot = vd->framebuffer;
    }
	return (ret);
}

/* Init VdIn struct with default and/or global values
 * args:
 * vd: pointer to a VdIn struct ( must be allready allocated )
 * global: pointer to a GLOBAL struct ( must be allready initiated )
 *
 * returns: error code ( 0 - VDIN_OK)
*/
int init_videoIn(struct vdIn *vd, struct GLOBAL *global)
{
	int ret = VDIN_OK;
	char *device = global->videodevice;

	__INIT_MUTEX( __VMUTEX );
	if (vd == NULL || device == NULL)
		return VDIN_ALLOC_ERR;
	if (global->width == 0 || global->height == 0)
		return VDIN_RESOL_ERR;
	if (global->cap_meth < IO_MMAP || global->cap_meth > IO_READ)
		global->cap_meth = IO_MMAP;		//mmap by default
	vd->cap_meth = global->cap_meth;
	g_printf("capture method = %s\n", vd->cap_meth == IO_READ ? "read" : "mmap");
	vd->videodevice = NULL;
	vd->videodevice = g_strdup(device);
	g_printf("video device: %s \n", vd->videodevice);

	//flag to video thread
	//vd->capVid = FALSE;
	//flag from video thread
	//vd->VidCapStop=TRUE;
    vd->streaming = FALSE;

//	vd->VidFName = g_strdup(DEFAULT_AVI_FNAME);
	vd->signalquit = FALSE;
	vd->PanTilt=0;
	vd->isbayer = 0; //bayer mode off
	vd->pix_order=0; // pix order for bayer mode def: gbgbgb..|rgrgrg..
	vd->setFPS=0;
	vd->capImage=FALSE;
	vd->cap_raw=0;

	vd->ImageFName = g_strdup(DEFAULT_IMAGE_FNAME);

	//timestamps not supported by UVC driver
	//vd->timecode.type = V4L2_TC_TYPE_25FPS;
	//vd->timecode.flags = V4L2_TC_FLAG_DROPFRAME;

	vd->available_exp[0]=-1;
	vd->available_exp[1]=-1;
	vd->available_exp[2]=-1;
	vd->available_exp[3]=-1;

	vd->tmpbuffer = NULL;
	vd->framebuffer = NULL;

	vd->listDevices = enum_devices( vd->videodevice );

	if (vd->listDevices != NULL)
	{
		if(!(vd->listDevices->listVidDevices))
			g_printerr("unable to detect video devices on your system (%i)\n", vd->listDevices->num_devices);
	}
	else
		g_printerr("Unable to detect devices on your system\n");

	if (vd->fd <=0 ) //open device
	{
		if ((vd->fd = v4l2_open(vd->videodevice, O_RDWR | O_NONBLOCK, 0)) < 0)
		{
			perror("ERROR opening V4L interface");
			ret = VDIN_DEVICE_ERR;
			goto error;
		}
	}

	//reset v4l2_format
	memset(&vd->fmt, 0, sizeof(struct v4l2_format));
	// populate video capabilities structure array
	// should only be called after all vdIn struct elements
	// have been initialized
	if((ret = check_videoIn(vd, &global->width, &global->height)) != VDIN_OK)
	{
		goto error;
	}

	//add dynamic controls
	//only for uvc logitech cameras
	//needs admin rights
	if(vd->listDevices->num_devices > 0)
	{
		g_printf("vid:%04x \npid:%04x \ndriver:%s\n",
			vd->listDevices->listVidDevices[vd->listDevices->current_device].vendor,
			vd->listDevices->listVidDevices[vd->listDevices->current_device].product,
			vd->listDevices->listVidDevices[vd->listDevices->current_device].driver);
		if(g_strcmp0(vd->listDevices->listVidDevices[vd->listDevices->current_device].driver,"uvcvideo") == 0)
		{
			if(vd->listDevices->listVidDevices[vd->listDevices->current_device].vendor != 0)
			{
				//check for logitech vid
				if (vd->listDevices->listVidDevices[vd->listDevices->current_device].vendor == 0x046d)
					(ret=initDynCtrls(vd->fd));
				else ret= VDIN_DYNCTRL_ERR;
			}
			else (ret=initDynCtrls(vd->fd));
		}
		else ret = VDIN_DYNCTRL_ERR;

	}

	if(global->add_ctrls)
	{
		//added extension controls so now we can exit
		//set a return code for enabling the correct warning window
		ret = (ret ? VDIN_DYNCTRL_ERR: VDIN_DYNCTRL_OK);
		goto error;
	}
	else
        ret = 0; //clean ret code

    if ((ret=init_v4l2(vd, &global->format, &global->width, &global->height, &global->fps, &global->fps_num)) < 0)
    {
        g_printerr("Init v4L2 failed !! \n");
        goto error;
    }

    g_printf("mode is set to %s\n", global->mode);
    g_printf("resolution is set to %dx%d\n", global->width, global->height);
    g_printf("fps is set to %i/%i\n", global->fps_num, global->fps);
    /*allocations*/
    if((ret = videoIn_frame_alloca(vd, global->format, global->width, global->height)) != VDIN_OK)
    {
        goto error;
    }
	return (ret);
error:
	v4l2_close(vd->fd);
	vd->fd=0;
	g_free(vd->videodevice);
	g_free(vd->VidFName);
	g_free(vd->ImageFName);
	vd->videodevice = NULL;
	vd->VidFName = NULL;
	vd->ImageFName = NULL;

	if(vd->cap_meth == IO_READ)
	{
		g_printf("cleaning read buffer\n");
		if((vd->buf.length > 0) && vd->mem[0])
		{
			g_free(vd->mem[0]);
			vd->mem[0] = NULL;
		}
	}
	__CLOSE_MUTEX( __VMUTEX );
	return ret;
}

/* decode video stream (frame buffer in yuyv format)
 * args:
 * vd: pointer to a VdIn struct ( must be allready allocated )
 *
 * returns: error code ( 0 - VDIN_OK)
*/
static int frame_decode(struct vdIn *vd, int format, int width, int height)
{
	int ret = VDIN_OK;
	int framesizeIn =(width * height << 1);//2 bytes per pixel
	switch (format)
	{
		case V4L2_PIX_FMT_JPEG:
		case V4L2_PIX_FMT_MJPEG:
			if(vd->buf.bytesused <= HEADERFRAME1)
			{
				// Prevent crash on empty image
				g_printf("Ignoring empty buffer ...\n");
				return (ret);
			}
			memcpy(vd->tmpbuffer, vd->mem[vd->buf.index],vd->buf.bytesused);

			if (jpeg_decode(&vd->framebuffer, vd->tmpbuffer, width, height) < 0)
			{
				g_printerr("jpeg decode errors\n");
				ret = VDIN_DECODE_ERR;
				return ret;
			}
			break;

		case V4L2_PIX_FMT_UYVY:
			memcpy(vd->tmpbuffer, vd->mem[vd->buf.index],vd->buf.bytesused);
			uyvy_to_yuyv(vd->framebuffer, vd->tmpbuffer, width, height);
			break;

		case V4L2_PIX_FMT_YVYU:
			memcpy(vd->tmpbuffer, vd->mem[vd->buf.index],vd->buf.bytesused);
			yvyu_to_yuyv(vd->framebuffer, vd->tmpbuffer, width, height);
			break;

		case V4L2_PIX_FMT_YYUV:
			memcpy(vd->tmpbuffer, vd->mem[vd->buf.index],vd->buf.bytesused);
			yyuv_to_yuyv(vd->framebuffer, vd->tmpbuffer, width, height);
			break;

		case V4L2_PIX_FMT_YUV420:
			memcpy(vd->tmpbuffer, vd->mem[vd->buf.index],vd->buf.bytesused);
			yuv420_to_yuyv(vd->framebuffer, vd->tmpbuffer, width, height);
			break;

		case V4L2_PIX_FMT_YVU420:
			memcpy(vd->tmpbuffer, vd->mem[vd->buf.index],vd->buf.bytesused);
			yvu420_to_yuyv(vd->framebuffer, vd->tmpbuffer, width, height);
			break;

		case V4L2_PIX_FMT_NV12:
			memcpy(vd->tmpbuffer, vd->mem[vd->buf.index],vd->buf.bytesused);
			nv12_to_yuyv(vd->framebuffer, vd->tmpbuffer, width, height);
			break;

		case V4L2_PIX_FMT_NV21:
			memcpy(vd->tmpbuffer, vd->mem[vd->buf.index],vd->buf.bytesused);
			nv21_to_yuyv(vd->framebuffer, vd->tmpbuffer, width, height);
			break;

		case V4L2_PIX_FMT_NV16:
			memcpy(vd->tmpbuffer, vd->mem[vd->buf.index],vd->buf.bytesused);
			nv16_to_yuyv(vd->framebuffer, vd->tmpbuffer, width, height);
			break;

		case V4L2_PIX_FMT_NV61:
			memcpy(vd->tmpbuffer, vd->mem[vd->buf.index],vd->buf.bytesused);
			nv61_to_yuyv(vd->framebuffer, vd->tmpbuffer, width, height);
			break;

		case V4L2_PIX_FMT_Y41P:
			memcpy(vd->tmpbuffer, vd->mem[vd->buf.index],vd->buf.bytesused);
			y41p_to_yuyv(vd->framebuffer, vd->tmpbuffer, width, height);
			break;

		case V4L2_PIX_FMT_GREY:
			memcpy(vd->tmpbuffer, vd->mem[vd->buf.index],vd->buf.bytesused);
			grey_to_yuyv(vd->framebuffer, vd->tmpbuffer, width, height);
			break;

		case V4L2_PIX_FMT_SPCA501:
			memcpy(vd->tmpbuffer, vd->mem[vd->buf.index],vd->buf.bytesused);
			s501_to_yuyv(vd->framebuffer, vd->tmpbuffer, width, height);
			break;

		case V4L2_PIX_FMT_SPCA505:
			memcpy(vd->tmpbuffer, vd->mem[vd->buf.index],vd->buf.bytesused);
			s505_to_yuyv(vd->framebuffer, vd->tmpbuffer, width, height);
			break;

		case V4L2_PIX_FMT_SPCA508:
			memcpy(vd->tmpbuffer, vd->mem[vd->buf.index],vd->buf.bytesused);
			s508_to_yuyv(vd->framebuffer, vd->tmpbuffer, width, height);
			break;

		case V4L2_PIX_FMT_YUYV:
			if(vd->isbayer>0)
			{
				if (!(vd->tmpbuffer))
				{
					// rgb buffer for decoding bayer data
					vd->tmpbuffer = g_new0(unsigned char,
						width * height * 3);
				}
				bayer_to_rgb24 (vd->mem[vd->buf.index],vd->tmpbuffer, width, height, vd->pix_order);
				// raw bayer is only available in logitech cameras in yuyv mode
				rgb2yuyv (vd->tmpbuffer,vd->framebuffer, width, height);
			}
			else
			{
				if (vd->buf.bytesused > framesizeIn)
					memcpy(vd->framebuffer, vd->mem[vd->buf.index],
						(size_t) framesizeIn);
				else
					memcpy(vd->framebuffer, vd->mem[vd->buf.index],
						(size_t) vd->buf.bytesused);
			}
			break;

		case V4L2_PIX_FMT_SGBRG8: //0
			bayer_to_rgb24 (vd->mem[vd->buf.index],vd->tmpbuffer, width, height, 0);
			rgb2yuyv (vd->tmpbuffer, vd->framebuffer, width, height);
			break;

		case V4L2_PIX_FMT_SGRBG8: //1
			bayer_to_rgb24 (vd->mem[vd->buf.index], vd->tmpbuffer, width, height, 1);
			rgb2yuyv (vd->tmpbuffer, vd->framebuffer, width, height);
			break;

		case V4L2_PIX_FMT_SBGGR8: //2
			bayer_to_rgb24 (vd->mem[vd->buf.index], vd->tmpbuffer, width, height, 2);
			rgb2yuyv (vd->tmpbuffer, vd->framebuffer, width, height);
			break;
		case V4L2_PIX_FMT_SRGGB8: //3
			bayer_to_rgb24 (vd->mem[vd->buf.index], vd->tmpbuffer, width, height, 3);
			rgb2yuyv (vd->tmpbuffer, vd->framebuffer, width, height);
			break;

		case V4L2_PIX_FMT_RGB24:
			memcpy(vd->tmpbuffer, vd->mem[vd->buf.index], vd->buf.bytesused);
			rgb2yuyv(vd->tmpbuffer, vd->framebuffer, width, height);
			break;
		case V4L2_PIX_FMT_BGR24:
			memcpy(vd->tmpbuffer, vd->mem[vd->buf.index],vd->buf.bytesused);
			bgr2yuyv(vd->tmpbuffer, vd->framebuffer, width, height);
			break;

		default:
			g_printerr("error grabbing (v4l2uvc.c) unknown format: %i\n", format);
			ret = VDIN_UNKNOWN_ERR;
			return ret;
	}

    /*
    Always save raw frame in tmpbuffer for snapshot.
    The filters will not apply to snapshot.
    */
    if (vd->snapshot != vd->framebuffer)
        memcpy(vd->snapshot, vd->framebuffer, vd->framebuffer_size);

	return ret;
}

/* Grabs video frame and decodes it if necessary
 * args:
 * vd: pointer to a VdIn struct ( must be allready initiated)
 *
 * returns: error code ( 0 - VDIN_OK)
*/
int uvcGrab(struct vdIn *vd, int format, int width, int height, int *fps, int *fps_num)
{
	int ret = VDIN_OK;
	fd_set rdset;
	struct timeval timeout;
	UINT64 ts = 0;
	//make sure streaming is on
	if (!vd->isstreaming)
		if (video_enable(vd))
        {
            vd->signalquit = TRUE;
            return ret;
        }

	FD_ZERO(&rdset);
	FD_SET(vd->fd, &rdset);
	timeout.tv_sec = 1; // 1 sec timeout
	timeout.tv_usec = 0;
	// select - wait for data or timeout
	ret = select(vd->fd + 1, &rdset, NULL, NULL, &timeout);
	if (ret < 0)
	{
		perror(" Could not grab image (select error)");
		vd->timestamp = 0;
		return VDIN_SELEFAIL_ERR;
	}
	else if (ret == 0)
	{
		perror(" Could not grab image (select timeout)");
		vd->timestamp = 0;
		return VDIN_SELETIMEOUT_ERR;
	}
	else if ((ret > 0) && (FD_ISSET(vd->fd, &rdset)))
	{
		switch(vd->cap_meth)
		{
			case IO_READ:
				if(vd->setFPS > 0)
				{
					video_disable(vd);
					input_set_framerate (vd, fps, fps_num);
					video_enable(vd);
					vd->setFPS = 0; /*no need to query and queue buufers*/
				}
				vd->buf.bytesused = v4l2_read (vd->fd, vd->mem[vd->buf.index], vd->buf.length);
				vd->timestamp = ns_time_monotonic();
				if (-1 == vd->buf.bytesused )
				{
					switch (errno)
					{
						case EAGAIN:
							g_print("No data available for read\n");
							return VDIN_SELETIMEOUT_ERR;
							break;
						case EINVAL:
							perror("Read method error, try mmap instead");
							return VDIN_READ_ERR;
							break;
						case EIO:
							perror("read I/O Error");
							return VDIN_READ_ERR;
							break;
						default:
							perror("read");
							return VDIN_READ_ERR;
							break;
					}
					vd->timestamp = 0;
				}
				break;

			case IO_MMAP:
			default:
				/*query and queue buffers since fps or compression as changed*/
				if((vd->setFPS > 0) || (vd->setJPEGCOMP > 0))
				{
					/*------------------------------------------*/
					/*  change video fps or frame compression   */
					/*------------------------------------------*/
					if(vd->setFPS) //change fps
					{
						video_disable(vd);
						unmap_buff(vd);
						input_set_framerate (vd, fps, fps_num);
						query_buff(vd);
						queue_buff(vd);
						video_enable(vd);
						vd->setFPS = 0;
					}
					else if(vd->setJPEGCOMP) //change jpeg quality/compression in video frame
					{
						video_disable(vd);
						unmap_buff(vd);
						set_jpegcomp(vd);
						get_jpegcomp(vd);
						query_buff(vd);
						queue_buff(vd);
						video_enable(vd);
						vd->setJPEGCOMP = 0;
					}
				}
				else
				{
					memset(&vd->buf, 0, sizeof(struct v4l2_buffer));
					vd->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
					vd->buf.memory = V4L2_MEMORY_MMAP;

					ret = xioctl(vd->fd, VIDIOC_DQBUF, &vd->buf);
					if (ret < 0)
					{
						perror("VIDIOC_DQBUF - Unable to dequeue buffer ");
						ret = VDIN_DEQBUFS_ERR;
						return ret;
					}
					ts = (UINT64) vd->buf.timestamp.tv_sec * G_NSEC_PER_SEC +
                        vd->buf.timestamp.tv_usec * 1000; //in nanosec

					/* use buffer timestamp if set by the driver, otherwise use current system time */
					if(ts > 0) vd->timestamp = ts;
					else vd->timestamp = ns_time_monotonic();

                    // BUG why loss frame_decode() between DQBUF and QBUF?
                    // 不知道原先為什麼在 DQBUF 和 QBUF 之間沒做任何事？ (例如 frame_decode)
                    // 這樣應該不會繪出任何畫面。但原本的寫法就這樣，卻還是能工作？

                    // 移到 frame_decode() 之後再 QBUF.
                    /*
					ret = xioctl(vd->fd, VIDIOC_QBUF, &vd->buf);
					if (ret < 0)
					{
						perror("VIDIOC_QBUF - Unable to queue buffer");
						ret = VDIN_QBUF_ERR;
						return ret;
					}
					*/
				}
		}
	}

	// save raw frame
	if (vd->cap_raw > 0)
	{
		SaveBuff(vd->ImageFName,vd->buf.bytesused,vd->mem[vd->buf.index]);
		vd->cap_raw=0;
	}

	if ((ret = frame_decode(vd, format, width, height)) != VDIN_OK)
    {
        vd->signalquit = TRUE;
    }

    // FIXBUG
    if (vd->cap_meth == IO_MMAP) {
        ret = xioctl(vd->fd, VIDIOC_QBUF, &vd->buf);
        if (ret < 0)
        {
            perror("VIDIOC_QBUF - Unable to queue buffer");
            ret = VDIN_QBUF_ERR;
        }
    }

    return ret;
}

static int close_v4l2_buffers (struct vdIn *vd)
{
	//clean frame buffers
	if(vd->tmpbuffer != NULL) g_free(vd->tmpbuffer);
	vd->tmpbuffer = NULL;
	if(vd->framebuffer != NULL) g_free(vd->framebuffer);
	vd->framebuffer = NULL;
	// unmap queue buffers
	switch(vd->cap_meth)
	{
		case IO_READ:
			if(vd->mem[vd->buf.index]!= NULL)
	    		{
				g_free(vd->mem[vd->buf.index]);
				vd->mem[vd->buf.index] = NULL;
			 }
			break;

		case IO_MMAP:
		default:
			//delete requested buffers
			unmap_buff(vd);
			memset(&vd->rb, 0, sizeof(struct v4l2_requestbuffers));
			vd->rb.count = 0;
			vd->rb.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			vd->rb.memory = V4L2_MEMORY_MMAP;
			if(xioctl(vd->fd, VIDIOC_REQBUFS, &vd->rb)<0)
			{
				g_printerr("VIDIOC_REQBUFS - Failed to delete buffers: %s (errno %d)\n", strerror(errno), errno);
				return(VDIN_REQBUFS_ERR);
			}
			break;
	}
	return (VDIN_OK);
}


int restart_v4l2(struct vdIn *vd, struct GLOBAL *global)
{
	int ret = VDIN_OK;
	video_disable(vd);
	close_v4l2_buffers(vd);

	if ((ret=init_v4l2(vd, &global->format, &global->width, &global->height, &global->fps, &global->fps_num)) < 0)
	{
		g_printerr("Init v4L2 failed !! \n");
		goto error;
	}
	/*allocations*/
	if((ret = videoIn_frame_alloca(vd, global->format, global->width, global->height)) != VDIN_OK)
	{
		goto error;
	}
	/*try to start the video stream*/
	//it's OK if it fails since it is retried in uvcGrab
	video_enable(vd);

	return (ret);
//error: clean up allocs
error:
	vd->signalquit = TRUE;
	return (ret);

}

/* cleans VdIn struct and allocations
 * args:
 * pointer to initiated vdIn struct
 *
 * returns: void
*/
void close_v4l2(struct vdIn *vd)
{
	if (vd->isstreaming) video_disable(vd);

	if(vd->videodevice) g_free(vd->videodevice);
	if(vd->ImageFName)g_free(vd->ImageFName);
	if(vd->VidFName)g_free(vd->VidFName);
	// free format allocations
	if(vd->listFormats) freeFormats(vd->listFormats);
    close_v4l2_buffers(vd);
	vd->videodevice = NULL;
	vd->tmpbuffer = NULL;
	vd->framebuffer = NULL;
	vd->ImageFName = NULL;
	vd->VidFName = NULL;
	if(vd->listDevices != NULL) freeDevices(vd->listDevices);
	// close device descriptor
	if(vd->fd) v4l2_close(vd->fd);
	__CLOSE_MUTEX( __VMUTEX );
	// free struct allocation
	if(vd) g_free(vd);
	vd=NULL;
}

/* sets video device frame rate
 * args:
 * vd: pointer to a VdIn struct ( must be allready initiated)
 *
 * returns: VIDIOC_S_PARM ioctl result value
*/
int
input_set_framerate (struct vdIn * device, int *fps, int *fps_num)
{
	int fd;
	int ret=0;

	fd = device->fd;

	device->streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	device->streamparm.parm.capture.timeperframe.numerator = *fps_num;
	device->streamparm.parm.capture.timeperframe.denominator = *fps;

	ret = xioctl(fd,VIDIOC_S_PARM,&device->streamparm);
	if (ret < 0)
	{
		g_printerr("Unable to set %d/%d fps\n", *fps_num, *fps);
		perror("VIDIOC_S_PARM error");
	}

	/*make sure we now have the correct fps*/
	input_get_framerate (device, fps, fps_num);

	return ret;
}

/* gets video device defined frame rate (not real - consider it a maximum value)
 * args:
 * vd: pointer to a VdIn struct ( must be allready initiated)
 *
 * returns: VIDIOC_G_PARM ioctl result value
*/
int
input_get_framerate (struct vdIn * device, int *fps, int *fps_num)
{
	int fd;
	int ret=0;

	fd = device->fd;

	ret = xioctl(fd,VIDIOC_G_PARM,&device->streamparm);
	if (ret < 0)
	{
		perror("VIDIOC_G_PARM - Unable to get timeperframe");
	}
	else
	{
		// it seems numerator is allways 1 but we don't do assumptions here :-)
		*fps = device->streamparm.parm.capture.timeperframe.denominator;
		*fps_num = device->streamparm.parm.capture.timeperframe.numerator;
	}
	return ret;
}
