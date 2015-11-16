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
#  read YUYV or UYVY data for Jpeg encoding                                             #
#                                                                               # 
#  Adapted for linux, Paulo Assis, 2007 <pj.assis@gmail.com>                    #
********************************************************************************/

#include <stdlib.h>
#include "jpgenc.h"

/*YUYV*/
UINT8* read_422_format (struct JPEG_ENCODER_STRUCTURE * jpeg_encoder_structure, UINT8 *input_ptr)
{
	INT32 i, j;
	
	INT16 *Y1_Ptr = jpeg_encoder_structure->Y1; /*64 int16 block*/ 
	INT16 *Y2_Ptr = jpeg_encoder_structure->Y2;
	INT16 *CB_Ptr = jpeg_encoder_structure->CB;
	INT16 *CR_Ptr = jpeg_encoder_structure->CR;

	UINT16 incr = jpeg_encoder_structure->incr;
	
	UINT8 *tmp_ptr=NULL;
	tmp_ptr=input_ptr;
	
	for (i=8; i>0; i--) /*8 rows*/
	{
		for (j=4; j>0; j--) /* 8 cols*/
		{
			*Y1_Ptr++ = *tmp_ptr++;
			*CB_Ptr++ = *tmp_ptr++;
			*Y1_Ptr++ = *tmp_ptr++;
			*CR_Ptr++ = *tmp_ptr++;
		}

		for (j=4; j>0; j--) /* 8 cols*/
		{
			*Y2_Ptr++ = *tmp_ptr++;
			*CB_Ptr++ = *tmp_ptr++;
			*Y2_Ptr++ = *tmp_ptr++;
			*CR_Ptr++ = *tmp_ptr++;
		}

		tmp_ptr += incr; /* next row (width - mcu_width)*/
	}
	tmp_ptr=NULL;/*clean*/
	return (input_ptr);
}
