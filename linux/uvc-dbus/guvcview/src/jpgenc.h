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
#  prototypes for Jpeg encoder                                                  #
#                                                                               # 
#  Adapted for linux, Paulo Assis, 2007 <pj.assis@gmail.com>                    #
********************************************************************************/

#ifndef JPGENC_H
#define JPGENC_H

#include "jdatatype.h"
#include "dct.h"

void initialization (struct JPEG_ENCODER_STRUCTURE * jpeg, int image_width, int image_height);

UINT16 DSP_Division (UINT32 numer, UINT32 denom);

void initialize_quantization_tables (struct JPEG_ENCODER_STRUCTURE * jpeg_encoder_structure);

UINT8* write_markers (struct JPEG_ENCODER_STRUCTURE * jpeg_encoder_structure, 
	UINT8 * output_ptr,int huff, UINT32 image_width, UINT32 image_height);

int encode_image (UINT8 * input_ptr,UINT8 * output_ptr, 
	struct JPEG_ENCODER_STRUCTURE * jpeg_encoder_structure,
	int huff, UINT32 image_width, UINT32 image_height);

UINT8* read_422_format (struct JPEG_ENCODER_STRUCTURE * jpeg_encoder_structure, UINT8 *input_ptr);

UINT8* encodeMCU (struct JPEG_ENCODER_STRUCTURE * jpeg_encoder_structure, UINT8 *output_ptr);

void quantization (struct JPEG_ENCODER_STRUCTURE * jpeg_encoder_structure, 
	INT16* const data, UINT16* const quant_table_ptr);

UINT8* huffman (struct JPEG_ENCODER_STRUCTURE * jpeg_encoder_structure, 
	UINT16 component, UINT8 *output_ptr);

UINT8* close_bitstream (struct JPEG_ENCODER_STRUCTURE * jpeg_encoder_structure, UINT8 *output_ptr);
#endif
