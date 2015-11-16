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

#ifndef PICTURE_H
#define PICTURE_H

#include "defs.h"

typedef struct tagBITMAPFILEHEADER 
{ 
	WORD    bfType; //Specifies the file type, must be BM
	DWORD   bfSize; //Specifies the size, in bytes, of the bitmap file
	WORD    bfReserved1; //Reserved; must be zero
	WORD    bfReserved2; //Reserved; must be zero
	DWORD   bfOffBits; /*Specifies the offset, in bytes, 
			    from the beginning of the BITMAPFILEHEADER structure 
			    to the bitmap bits= FileHeader+InfoHeader+RGBQUAD(0 for 24bit BMP)=64*/
}   __attribute__ ((packed)) BITMAPFILEHEADER, *PBITMAPFILEHEADER;


typedef struct tagBITMAPINFOHEADER
{
	DWORD  biSize; 
	LONG   biWidth; 
	LONG   biHeight; 
	WORD   biPlanes; 
	WORD   biBitCount; 
	DWORD  biCompression; 
	DWORD  biSizeImage; 
	LONG   biXPelsPerMeter; 
	LONG   biYPelsPerMeter; 
	DWORD  biClrUsed; 
	DWORD  biClrImportant; 
}  __attribute__ ((packed)) BITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct tagJPGFILEHEADER 
{
	BYTE SOI[2];/*SOI Marker 0xFFD8*/
	BYTE APP0[2];/*APP0 MARKER 0xFF0E*/
	BYTE length[2];/*length of header without APP0 in bytes*/
	BYTE JFIF[5];/*set to JFIF0 0x4A46494600*/
	BYTE VERS[2];/*1-2 0x0102*/
	BYTE density;/* 0 - No units, aspect ratio only specified
		        1 - Pixels per Inch on quickcam5000pro
			2 - Pixels per Centimetre                */
	BYTE xdensity[2];/*120 on quickcam5000pro*/
	BYTE ydensity[2];/*120 on quickcam5000pro*/
	BYTE WTN;/*width Thumbnail 0*/
	BYTE HTN;/*height Thumbnail 0*/	
} __attribute__ ((packed)) JPGFILEHEADER, *PJPGFILEHEADER;

int SaveJPG(const char *Filename,int imgsize,BYTE *ImagePix);

int SaveBuff(const char *Filename,int imgsize,BYTE *data);

int SaveBPM(const char *Filename, long width, long height, int BitCount, BYTE *ImagePix);

int write_png(char *file_name, int width, int height,BYTE *prgb_data);

#endif

