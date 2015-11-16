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
*                                                                               #  
*     Jpeg encoder                                                              # 
*                                                                               #
*     Adapted for linux, Paulo Assis, 2007 <pj.assis@gmail.com>                 #  
*                                                                               #
********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include "jpgenc.h"


void 
initialization (struct JPEG_ENCODER_STRUCTURE * jpeg, int image_width, int image_height)
{
	UINT16 mcu_width, mcu_height, bytes_per_pixel;

	jpeg->mcu_width = mcu_width = 16;
	jpeg->horizontal_mcus = (UINT16) (image_width >> 4);/* width/16 */
	
	jpeg->mcu_height = mcu_height = 8;
	jpeg->vertical_mcus = (UINT16) (image_height >> 3); /* height/8 */ 
	
	bytes_per_pixel = 2;
		
	jpeg->length_minus_mcu_width = (UINT16) ((image_width - mcu_width) * bytes_per_pixel);
	jpeg->length_minus_width = (UINT16) (image_width * bytes_per_pixel);
	
	jpeg->mcu_width_size = (UINT16) (mcu_width * bytes_per_pixel);

	jpeg->rows = jpeg->mcu_height;
	jpeg->cols = jpeg->mcu_width;
	jpeg->incr = jpeg->length_minus_mcu_width;
	jpeg->offset = (UINT16) ((image_width * mcu_height) * bytes_per_pixel);
	
	jpeg->ldc1 = 0;
	jpeg->ldc2 = 0;
	jpeg->ldc3 = 0;
	

	jpeg->lcode = 0;
	jpeg->bitindex = 0;
}

void 
jpeg_restart (struct JPEG_ENCODER_STRUCTURE * jpeg_encoder_structure)
{
	jpeg_encoder_structure->ldc1 = 0;
	jpeg_encoder_structure->ldc2 = 0;
	jpeg_encoder_structure->ldc3 = 0;

	jpeg_encoder_structure->lcode = 0;
	jpeg_encoder_structure->bitindex = 0;
}


int encode_image (UINT8 *input_ptr,UINT8 *output_ptr, 
	struct JPEG_ENCODER_STRUCTURE * jpeg_encoder_structure,
	int huff, UINT32 image_width,UINT32 image_height)
{
	int size;
	UINT16 i, j;
	UINT8 *tmp_ptr=NULL;
	UINT8 *tmp_iptr=NULL;
	UINT8 *tmp_optr=NULL;
	tmp_iptr=input_ptr;
	tmp_optr=output_ptr;
	
	/* clean jpeg parameters*/
	jpeg_restart(jpeg_encoder_structure);
	
	/* Writing Marker Data */
	tmp_optr = write_markers (jpeg_encoder_structure, tmp_optr, huff, 
							         image_width, image_height);

	for (i=0; i<jpeg_encoder_structure->vertical_mcus; i++) /* height /8 */
	{
		tmp_ptr=tmp_iptr;
		for (j=0; j<jpeg_encoder_structure->horizontal_mcus; j++) /* width /16 */
		{	
			/*reads a block*/
			read_422_format (jpeg_encoder_structure, tmp_iptr); /*YUYV*/
	
			/* Encode the data in MCU */
			tmp_optr = encodeMCU (jpeg_encoder_structure, tmp_optr);
			
			if(j<(jpeg_encoder_structure->horizontal_mcus -1)) 
			{
				tmp_iptr += jpeg_encoder_structure->mcu_width_size;
			}
			else 
			{
				tmp_iptr=tmp_ptr;
			}
		}
		tmp_iptr += jpeg_encoder_structure->offset;
		
	}

	/* Close Routine */
	tmp_optr=close_bitstream (jpeg_encoder_structure, tmp_optr);
	size=tmp_optr-output_ptr;
	tmp_iptr=NULL;
	tmp_optr=NULL;

	return (size);
}

UINT8* 
encodeMCU (struct JPEG_ENCODER_STRUCTURE * jpeg_encoder_structure, UINT8 *output_ptr)
{
	levelshift (jpeg_encoder_structure->Y1);
	DCT (jpeg_encoder_structure->Y1);
	quantization (jpeg_encoder_structure, jpeg_encoder_structure->Y1, 
		jpeg_encoder_structure->ILqt);
	
	output_ptr = huffman (jpeg_encoder_structure, 1, output_ptr);

	levelshift (jpeg_encoder_structure->Y2);
	DCT (jpeg_encoder_structure->Y2);

	quantization (jpeg_encoder_structure, jpeg_encoder_structure->Y2, 
		jpeg_encoder_structure->ILqt);
	
	output_ptr = huffman (jpeg_encoder_structure, 1, output_ptr);
 
	levelshift (jpeg_encoder_structure->CB);
	DCT (jpeg_encoder_structure->CB);
	
	quantization (jpeg_encoder_structure, jpeg_encoder_structure->CB, 
		jpeg_encoder_structure->ICqt);
	
	output_ptr = huffman (jpeg_encoder_structure, 2, output_ptr);

	levelshift (jpeg_encoder_structure->CR);
	DCT (jpeg_encoder_structure->CR);
	
	quantization (jpeg_encoder_structure, jpeg_encoder_structure->CR, 
		jpeg_encoder_structure->ICqt);
	
	output_ptr = huffman (jpeg_encoder_structure, 3, output_ptr);

	return output_ptr;
}
