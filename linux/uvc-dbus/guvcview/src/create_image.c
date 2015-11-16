/*******************************************************************************#
#           guvcview              http://guvcview.berlios.de                    #
#                                                                               #
#           Paulo Assis <pj.assis@gmail.com>                                    #
#           rock <shirock.tw@gmail.com>  Fix snapshot and MJPG saving bug.      #
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
#include <png.h>

#include "defs.h"
#include "guvcview.h"
#include "v4l2uvc.h"
#include "colorspaces.h"
#include "jpgenc.h"
#include "picture.h"
#include "ms_time.h"
#include "string_utils.h"

int store_picture(void *data)
{
	struct ALL_DATA *all_data = (struct ALL_DATA *) data;

	struct GLOBAL *global = all_data->global;
	struct vdIn *videoIn = all_data->videoIn;

	struct JPEG_ENCODER_STRUCTURE *jpeg_struct = NULL;
	BYTE *pim =  NULL;
	BYTE *jpeg = NULL;
	int jpeg_size = 0;
	int rc = 0;

    /*
    TODO snapshot by rock.
    I save raw (no filter, no image processing) frame in snapshot.
    So here should save picture from snapshot, not framebuffer.
    */

	switch(global->imgFormat)
	{
		case IMG_FORMAT_JPG:/*jpg*/
			/* Save directly from MJPG frame */
			#if 0
			// TODO #1432, some webcam's mjpeg frame could not directly save as jpeg.
			if((global->Frame_Flags==0) && (global->format==V4L2_PIX_FMT_MJPEG))
			{
				if(SaveJPG(videoIn->ImageFName,videoIn->buf.bytesused,videoIn->tmpbuffer))
				{
					g_printerr ("Error: Couldn't capture Image to %s \n",
						videoIn->ImageFName);
					return(-1);
				}
			}
			else
			#endif
			if ((global->Frame_Flags==0) && (global->format==V4L2_PIX_FMT_JPEG))
			{
				if (SaveBuff(videoIn->ImageFName,videoIn->buf.bytesused,videoIn->tmpbuffer))
				{
					g_printerr ("Error: Couldn't capture Image to %s \n",
						videoIn->ImageFName);
					//return(-1);
					rc = -1;
					goto end_func;
				}
			}
			else
			{ /* use built in encoder */
				jpeg = g_new0(BYTE, ((global->width)*(global->height))>>1);
				jpeg_struct = g_new0(struct JPEG_ENCODER_STRUCTURE, 1);

				/* Initialization of JPEG control structure */
				initialization (jpeg_struct,global->width,global->height);

				/* Initialization of Quantization Tables  */
				initialize_quantization_tables (jpeg_struct);


				//jpeg_size = encode_image(videoIn->framebuffer, jpeg,
				jpeg_size = encode_image(videoIn->snapshot, jpeg,
					jpeg_struct, 1, global->width, global->height);

				if(SaveBuff(videoIn->ImageFName, jpeg_size, jpeg))
				{
					g_printerr ("Error: Couldn't capture Image to %s \n",
						videoIn->ImageFName);
					//return(-1);
					rc = -1;
					goto end_func;
				}
			}
			break;

		case IMG_FORMAT_BMP:/*bmp*/
			/*24 bits -> 3bytes     32 bits ->4 bytes*/
			pim = g_new0(BYTE, (global->width)*(global->height)*3);
			//yuyv2bgr(videoIn->framebuffer,pim,global->width,global->height);
			yuyv2bgr(videoIn->snapshot,pim,global->width,global->height);

			if(SaveBPM(videoIn->ImageFName, global->width, global->height, 24, pim))
			{
				g_printerr ("Error: Couldn't capture Image to %s \n",
					videoIn->ImageFName);
				//return(-1);
				rc = -1;
				goto end_func;
			}
			break;

		case IMG_FORMAT_PNG:/*png*/
			/*24 bits -> 3bytes     32 bits ->4 bytes*/
			pim = g_new0(BYTE, (global->width)*(global->height)*3);
			//yuyv2rgb(videoIn->framebuffer,pim,global->width,global->height);
			yuyv2rgb(videoIn->snapshot,pim,global->width,global->height);
			write_png(videoIn->ImageFName, global->width, global->height, pim);
			break;

       case IMG_FORMAT_RAW:/*raw*/
            videoIn->cap_raw = 1;
            //return 1;
            rc = 1;
	}

  end_func:
	if(jpeg_struct) g_free(jpeg_struct);
	jpeg_struct=NULL;
	if(jpeg) g_free(jpeg);
	jpeg = NULL;
	if(pim) g_free(pim);
	pim=NULL;

	return rc;
}


static int
_store_picture_to_buffer_bmp(long width, long height, int BitCount, BYTE *ImagePix,
    guint8 **data, guint *data_len)
{
	int ret=0;
	BITMAPFILEHEADER BmpFileh;
	BITMAPINFOHEADER BmpInfoh;
	DWORD imgsize;
	//FILE *fp;
	guint8 *dp;

	imgsize=width*height*BitCount/8;

	BmpFileh.bfType=0x4d42;//must be BM (x4d42)
	/*Specifies the size, in bytes, of the bitmap file*/
	BmpFileh.bfSize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+imgsize;
	BmpFileh.bfReserved1=0; //Reserved; must be zero
	BmpFileh.bfReserved2=0; //Reserved; must be zero
	/*Specifies the offset, in bytes,                      */
	/*from the beginning of the BITMAPFILEHEADER structure */
	/* to the bitmap bits                                  */
	BmpFileh.bfOffBits=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);

	BmpInfoh.biSize=40;
	BmpInfoh.biWidth=width;
	BmpInfoh.biHeight=height;
	BmpInfoh.biPlanes=1;
	BmpInfoh.biBitCount=BitCount;
	BmpInfoh.biCompression=0; // 0
	BmpInfoh.biSizeImage=imgsize;
	BmpInfoh.biXPelsPerMeter=0;
	BmpInfoh.biYPelsPerMeter=0;
	BmpInfoh.biClrUsed=0;
	BmpInfoh.biClrImportant=0;

    *data_len = sizeof(BITMAPFILEHEADER) +
                sizeof(BITMAPINFOHEADER) +
                imgsize;
    *data = malloc(*data_len);
    dp = *data;
    memmove(dp, &BmpFileh, sizeof(BITMAPFILEHEADER));
    dp += sizeof(BITMAPFILEHEADER);
    memmove(dp, &BmpInfoh, sizeof(BITMAPINFOHEADER));
    dp += sizeof(BITMAPINFOHEADER);
    memmove(dp, ImagePix, imgsize);
    #if 0
	if ((fp = fopen(Filename,"wb"))!=NULL)
	{	// (wb) write in binary mode
		ret=fwrite(&BmpFileh, sizeof(BITMAPFILEHEADER), 1, fp);
		ret+=fwrite(&BmpInfoh, sizeof(BITMAPINFOHEADER),1,fp);
		ret+=fwrite(ImagePix,imgsize,1,fp);
		if (ret<3) ret=1;//write error
		else ret=0;

		fflush(fp); //flush data stream to file system
		if(fsync(fileno(fp)) || fclose(fp))
		{
			perror("BMP ERROR - couldn't write to file");
			ret=1;
		}
	}
	else
	{
		ret=1;
		g_printerr("ERROR: Could not open file %s for write \n",Filename);
	}
	#endif
	return ret;
}

static guint8 *_png_current_write_position;

static void _png_mem_write_data(png_structp png_ptr,
    png_bytep data, png_size_t length)
{
    memmove(_png_current_write_position, data, length);
    _png_current_write_position += length;
    return;
}

static void _png_mem_flush_data(png_structp png_ptr)
{
    return;
}

static int
_store_picture_to_buffer_png(int width, int height,BYTE *prgb_data,
    guint8 **data, guint *data_len)
{
	int l=0;
	//FILE *fp;
	guint8 *png_buff = g_new0(BYTE, width*height*3);
	png_structp png_ptr;
	png_infop info_ptr;
	png_text text_ptr[3];

	png_bytep row_pointers[height];
	/* open the file */
//	fp = fopen(file_name, "wb");
//	if (fp == NULL)
//	return (1);

	/* Create and initialize the png_struct with the desired error handler
	* functions.  If you want to use the default stderr and longjump method,
	* you can supply NULL for the last three parameters.  We also check that
	* the library version is compatible with the one used at compile time,
	* in case we are using dynamically linked libraries.
	*/
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
		NULL, NULL, NULL);

	if (png_ptr == NULL)
	{
		//fclose(fp);
		g_free(png_buff);
		return (2);
	}

	/* Allocate/initialize the image information data. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		//fclose(fp);
		g_free(png_buff);
		png_destroy_write_struct(&png_ptr, NULL);
		return (3);
	}

	/* Set error handling.  REQUIRED if you aren't supplying your own
	* error handling functions in the png_create_write_struct() call.
	*/
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* If we get here, we had a problem reading the file */
		//fclose(fp);
		g_free(png_buff);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return (4);
	}

	/* set up the output control using standard C streams */
	//png_init_io(png_ptr, fp);
	_png_current_write_position = png_buff;
    voidp write_io_ptr = png_get_io_ptr(png_ptr);
    png_set_write_fn(png_ptr,
                     write_io_ptr,
                     _png_mem_write_data,
                     _png_mem_flush_data);

	/* turn on or off filtering, and/or choose
	specific filters.  You can use either a single
	PNG_FILTER_VALUE_NAME or the bitwise OR of one
	or more PNG_FILTER_NAME masks. */
	/* png_set_filter(png_ptr, 0,
	PNG_FILTER_NONE  | PNG_FILTER_VALUE_NONE |
	PNG_FILTER_SUB   | PNG_FILTER_VALUE_SUB  |
	PNG_FILTER_UP    | PNG_FILTER_VALUE_UP   |
	PNG_FILTER_AVE   | PNG_FILTER_VALUE_AVE  |
	PNG_FILTER_PAETH | PNG_FILTER_VALUE_PAETH|
	PNG_ALL_FILTERS);*/

	/* set the zlib compression level */
	//png_set_compression_level(png_ptr,
	//	Z_BEST_COMPRESSION);

	/* set other zlib parameters */
	//png_set_compression_mem_level(png_ptr, 8);
	//png_set_compression_strategy(png_ptr,
	//	Z_DEFAULT_STRATEGY);
	//png_set_compression_window_bits(png_ptr, 15);
	//png_set_compression_method(png_ptr, 8);
	//png_set_compression_buffer_size(png_ptr, 8192);

	png_set_IHDR(png_ptr, info_ptr, width, height,
		8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	/* Optional gamma chunk is strongly suggested if you have any guess
	* as to the correct gamma of the image.
	*/
	//png_set_gAMA(png_ptr, info_ptr, gamma);

	/* Optionally write comments into the image */
	text_ptr[0].key = "Title";
	//text_ptr[0].text = file_name;
	text_ptr[0].text = "Snapshot";
	text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
	text_ptr[1].key = "Software";
	text_ptr[1].text = "guvcview";
	text_ptr[1].compression = PNG_TEXT_COMPRESSION_NONE;
	text_ptr[2].key = "Description";
	text_ptr[2].text = "File generated by guvcview <http://guvcview.berlios.de>";
	text_ptr[2].compression = PNG_TEXT_COMPRESSION_NONE;
#ifdef PNG_iTXt_SUPPORTED
	text_ptr[0].lang = NULL;
	text_ptr[1].lang = NULL;
	text_ptr[2].lang = NULL;
#endif
	png_set_text(png_ptr, info_ptr, text_ptr, 3);

	/* Write the file header information.  REQUIRED */
	png_write_info(png_ptr, info_ptr);

	/* flip BGR pixels to RGB */
	//png_set_bgr(png_ptr); /*?no longuer required?*/

	/* Write the image data.*/
	for (l = 0; l < height; l++)
		row_pointers[l] = prgb_data + l*width*3;

	png_write_image(png_ptr, row_pointers);

	/* You can write optional chunks like tEXt, zTXt, and tIME at the end
	* as well.  Shouldn't be necessary in 1.1.0 and up as all the public
	* chunks are supported and you can use png_set_unknown_chunks() to
	* register unknown chunks into the info structure to be written out.
	*/

	/* It is REQUIRED to call this to finish writing the rest of the file */
	png_write_end(png_ptr, info_ptr);

	/* If you png_malloced a palette, free it here (don't free info_ptr->palette,
	as recommended in versions 1.0.5m and earlier of this example; if
	libpng mallocs info_ptr->palette, libpng will free it).  If you
	allocated it with malloc() instead of png_malloc(), use free() instead
	of png_free(). */
	//png_free(png_ptr, palette);
	//palette=NULL;

	/* Similarly, if you png_malloced any data that you passed in with
	png_set_something(), such as a hist or trans array, free it here,
	when you can be sure that libpng is through with it. */
	//png_free(png_ptr, trans);
	//trans=NULL;

	/* clean up after the write, and free any memory allocated */
	png_destroy_write_struct(&png_ptr, &info_ptr);

	/* close the file */
	#if 0
	fflush(fp); //flush data stream to file system
	if(fsync(fileno(fp)) || fclose(fp))
	{
		perror("PNG ERROR - couldn't write to file");
		return(5);
	}
	#endif
	*data_len = _png_current_write_position - png_buff;
	*data = malloc(*data_len);
	g_print("png size: %d\n", *data_len);
	memmove(*data, png_buff, *data_len);
    g_free(png_buff);

	for(l=0;l<height;l++)
	{
		row_pointers[l]=NULL;
	}

	/* that's it */
	return (0);
}


/**
use free() to free data.
 */
int store_picture_to_buffer(guint8 **data, guint *data_len)
{
	struct GLOBAL *global = all_data.global;
	struct vdIn *videoIn = all_data.videoIn;

	struct JPEG_ENCODER_STRUCTURE *jpeg_struct = NULL;
	BYTE *pim =  NULL;
	BYTE *jpeg = NULL;
	int jpeg_size = 0;

    /*
    TODO snapshot by rock.
    I save raw (no filter, no image processing) frame in snapshot.
    So here should save picture from snapshot, not framebuffer.
    */

	switch(global->imgFormat)
	{
		case 0:/*jpg*/
			/* Save directly from MJPG frame */
			#if 0
			// TODO #1432, some webcam's mjpeg frame could not directly save as jpeg.
			if((global->Frame_Flags==0) && (global->format==V4L2_PIX_FMT_MJPEG))
			{
				if(SaveJPG(videoIn->ImageFName,videoIn->buf.bytesused,videoIn->tmpbuffer))
				{
					g_printerr ("Error: Couldn't capture Image to %s \n",
						videoIn->ImageFName);
					return(-1);
				}
			}
			else
			#endif
			if ((global->Frame_Flags==0) && (global->format==V4L2_PIX_FMT_JPEG))
			{
			    *data_len = videoIn->buf.bytesused;
			    *data = malloc(videoIn->buf.bytesused);
			    memmove(*data, videoIn->tmpbuffer, videoIn->buf.bytesused);
			    #if 0
				if (SaveBuff(videoIn->ImageFName,videoIn->buf.bytesused,videoIn->tmpbuffer))
				{
					g_printerr ("Error: Couldn't capture Image to %s \n",
						videoIn->ImageFName);
					return(-1);
				}
				#endif
			}
			else
			{ /* use built in encoder */
				jpeg = g_new0(BYTE, ((global->width)*(global->height))>>1);
				jpeg_struct = g_new0(struct JPEG_ENCODER_STRUCTURE, 1);

				/* Initialization of JPEG control structure */
				initialization (jpeg_struct,global->width,global->height);

				/* Initialization of Quantization Tables  */
				initialize_quantization_tables (jpeg_struct);


				//jpeg_size = encode_image(videoIn->framebuffer, jpeg,
				jpeg_size = encode_image(videoIn->snapshot, jpeg,
					jpeg_struct, 1, global->width, global->height);

			    *data_len = jpeg_size;
			    *data = malloc(jpeg_size);
			    memmove(*data, jpeg, jpeg_size);

                #if 0
				if(SaveBuff(videoIn->ImageFName, jpeg_size, jpeg))
				{
					g_printerr ("Error: Couldn't capture Image to %s \n",
						videoIn->ImageFName);
					return(-1);
				}
				#endif
			}
			break;

		case 1:/*bmp*/
			/*24 bits -> 3bytes     32 bits ->4 bytes*/
			pim = g_new0(BYTE, (global->width)*(global->height)*3);
			//yuyv2bgr(videoIn->framebuffer,pim,global->width,global->height);
			yuyv2bgr(videoIn->snapshot,pim,global->width,global->height);

            _store_picture_to_buffer_bmp(global->width, global->height, 24, pim, data, data_len);
			#if 0
			if(SaveBPM(videoIn->ImageFName, global->width, global->height, 24, pim))
			{
				g_printerr ("Error: Couldn't capture Image to %s \n",
					videoIn->ImageFName);
				return(-1);
			}
			#endif
			break;

		case 2:/*png*/
			/*24 bits -> 3bytes     32 bits ->4 bytes*/
			pim = g_new0(BYTE, (global->width)*(global->height)*3);
			//yuyv2rgb(videoIn->framebuffer,pim,global->width,global->height);
			yuyv2rgb(videoIn->snapshot,pim,global->width,global->height);
			_store_picture_to_buffer_png(global->width, global->height, pim, data, data_len);
			//write_png(videoIn->ImageFName, global->width, global->height, pim);
			break;

       case 3:/*raw*/
            videoIn->cap_raw = 1;
            return 1;
	}

	if(jpeg_struct) g_free(jpeg_struct);
	jpeg_struct=NULL;
	if(jpeg) g_free(jpeg);
	jpeg = NULL;
	if(pim) g_free(pim);
	pim=NULL;

	return 0;
}

