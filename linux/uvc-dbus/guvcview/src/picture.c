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

#include "picture.h"
#include "huffman.h"
#include "jpgenc.h"
#include <glib.h>
#include <glib/gprintf.h>
#include <png.h>

/*----------------------------------- Image Files ----------------------------*/ 
int 
SaveJPG(const char *Filename,int imgsize,BYTE *ImagePix) 
{
	int ret=0;
	int jpgsize=0;
	BYTE *jpgtmp=NULL;
	BYTE *Pimg=NULL;
	BYTE *Pjpg=NULL;
	BYTE *tp=NULL;	
	JPGFILEHEADER JpgFileh;
	int jpghsize=sizeof(JpgFileh);
	FILE *fp;
	jpgsize=jpghsize+imgsize+sizeof(JPEGHuffmanTable)+4;/*header+huffman+marker+buffsize*/
	Pimg=ImagePix;
	
	if((jpgtmp=g_malloc0(jpgsize))!=NULL) 
	{
		Pjpg=jpgtmp;
		/*Fill JFIF header*/
		JpgFileh.SOI[0]=0xff;
		JpgFileh.SOI[1]=0xd8;
		JpgFileh.APP0[0]=0xff;
		JpgFileh.APP0[1]=0xe0;
		JpgFileh.length[0]=0x00;
		JpgFileh.length[1]=0x10;
		JpgFileh.JFIF[0]=0x4a;//JFIF0
		JpgFileh.JFIF[1]=0x46;
		JpgFileh.JFIF[2]=0x49;
		JpgFileh.JFIF[3]=0x46;
		JpgFileh.JFIF[4]=0x00;
		JpgFileh.VERS[0]=0x01;//version 1.2
		JpgFileh.VERS[1]=0x02;
		JpgFileh.density=0x00;
		JpgFileh.xdensity[0]=0x00;
		JpgFileh.xdensity[1]=0x78;
		JpgFileh.ydensity[0]=0x00;
		JpgFileh.ydensity[1]=0x78;
		JpgFileh.WTN=0;
		JpgFileh.HTN=0;
	
		/*adds header (JFIF)*/
		memmove(Pjpg,&JpgFileh,jpghsize);
		/*moves to the end of the header struct (JFIF)*/
		Pjpg+=jpghsize;
		int headSize = ImagePix[4]*256+ImagePix[5] + 4;/*length + SOI+APP0*/
		/*moves to the end of header (MJPG)*/
		Pimg+=headSize;
		/*adds Quantization tables and everything else until   * 
		* start of frame marker (FFC0) 	 */
		tp=Pimg;
		int qtsize=0;
		while(!((tp[qtsize]== 0xff) && (tp[qtsize+1]== 0xc0))) 
		{
			qtsize++;
		}
		memmove(Pjpg,Pimg,qtsize);
		/*moves to the begining of frame marker*/
		Pjpg+=qtsize; 
		Pimg+=qtsize;
		/*insert huffman table with marker (FFC4) and length(x01a2)*/
		BYTE HUFMARK[4];
		HUFMARK[0]=0xff;
		HUFMARK[1]=0xc4;
		HUFMARK[2]=0x01;
		HUFMARK[3]=0xa2;
		memmove(Pjpg,&HUFMARK,4);
		Pjpg+=4;
		memmove(Pjpg,&JPEGHuffmanTable,JPG_HUFFMAN_TABLE_LENGTH);/*0x01a0*/
		/*moves to the end of huffman tables (JFIF)*/
		Pjpg+=JPG_HUFFMAN_TABLE_LENGTH;
		/*copys frame data(JFIF)*/
		memmove(Pjpg,Pimg,(imgsize-(Pimg-ImagePix)));
		Pjpg+=imgsize-(Pimg-ImagePix);
		
		int totSize = Pjpg - jpgtmp;
	
		if ((fp = fopen(Filename,"wb"))!=NULL) 
		{
			ret=fwrite(jpgtmp,totSize,1,fp);/*jpeg - jfif*/
			if (ret< 1) ret=1; //write error 
			else ret=0;
			fflush(fp); //flush data stream to file system
			if(fsync(fileno(fp)) || fclose(fp))
				perror("JPEG ERROR - couldn't write to file");
		} 
		else ret=1;

		g_free(jpgtmp);
		jpgtmp=NULL;
		Pimg=NULL;
		Pjpg=NULL;
		tp=NULL;
	}
	else 
	{
		g_printerr("could not allocate memmory for jpg file\n");
		ret=1;
	}
	return ret;
}


int 
SaveBuff(const char *Filename,int imgsize,BYTE *data) 
{
	FILE *fp;
	int ret = 0;
	if ((fp = fopen(Filename,"wb"))!=NULL) 
	{
		ret=fwrite(data,imgsize,1,fp);
		
		if (ret<1) ret=1;//write error
		else ret=0;
		
		fflush(fp); //flush data stream to file system
		if(fsync(fileno(fp)) || fclose(fp))
			perror("BUFF WRITE ERROR - couldn't write buffer to file");
	} 
	else ret = 1;
	return (ret);
}

int 
SaveBPM(const char *Filename, long width, long height, int BitCount, BYTE *ImagePix) 
{
	int ret=0;
	BITMAPFILEHEADER BmpFileh;
	BITMAPINFOHEADER BmpInfoh;
	DWORD imgsize;
	FILE *fp;

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
	return ret;
}

/* write a png file */
int write_png(char *file_name, int width, int height,BYTE *prgb_data)
{
	int l=0;
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	png_text text_ptr[3];

	png_bytep row_pointers[height];
	/* open the file */
	fp = fopen(file_name, "wb");
	if (fp == NULL)
	return (1);

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
		fclose(fp);
		return (2);
	}
	
	/* Allocate/initialize the image information data. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		fclose(fp);
		png_destroy_write_struct(&png_ptr, NULL);
		return (3);
	}
  
	/* Set error handling.  REQUIRED if you aren't supplying your own
	* error handling functions in the png_create_write_struct() call.
	*/
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* If we get here, we had a problem reading the file */
		fclose(fp);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return (4);
	}

	/* set up the output control using standard C streams */
	png_init_io(png_ptr, fp);
	
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
	text_ptr[0].text = file_name;
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
	fflush(fp); //flush data stream to file system
	if(fsync(fileno(fp)) || fclose(fp))
	{
		perror("PNG ERROR - couldn't write to file");
		return(5);
	}
	
	for(l=0;l<height;l++) 
	{
		row_pointers[l]=NULL;
	}

	/* that's it */
	return (0);
}

