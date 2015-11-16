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

/*******************************************************************************#
#                                                                               #
#  jpeg encoder struct used in Jpeg encoder                                     #
#                                                                               # 
#  Adapted for linux, Paulo Assis, 2007 <pj.assis@gmail.com>                    #
********************************************************************************/

#ifndef JDATATYPE_H
#define JDATATYPE_H

#include "defs.h"

typedef struct JPEG_ENCODER_STRUCTURE 
{
	UINT16	mcu_width;
	UINT16	mcu_height;
	UINT16	horizontal_mcus;
	UINT16	vertical_mcus;

	UINT16	rows;
	UINT16	cols;

	UINT16	length_minus_mcu_width;
	UINT16	length_minus_width;
	UINT16	incr;
	UINT16	mcu_width_size;
	UINT16	offset;

	INT16	ldc1;
	INT16	ldc2;
	INT16	ldc3;
	
	UINT32	lcode;
	UINT16	bitindex;
	/* MCUs */
	INT16	Y1 [64];
	INT16	Y2 [64];
	INT16	Temp [64];
	INT16	CB [64];
	INT16	CR [64];
	/* Quantization Tables */
	UINT8	Lqt [64];
	UINT8	Cqt [64];
	UINT16	ILqt [64];
	UINT16	ICqt [64];

} _JPEG_ENCODER_STRUCTURE;

#endif

