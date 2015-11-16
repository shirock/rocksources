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
#  quantization for Jpeg encoder                                                #
#                                                                               # 
#  Adapted for linux, Paulo Assis, 2007 <pj.assis@gmail.com>                    #
********************************************************************************/

#include "jpgenc.h"

UINT8 zigzag_table [] =
{
	0,  1,   5,  6, 14, 15, 27, 28,
	2,  4,   7, 13, 16, 26, 29, 42,
	3,  8,  12, 17, 25, 30, 41, 43,
	9,  11, 18, 24, 31, 40, 44, 53,
	10, 19, 23, 32, 39, 45, 52, 54,
	20, 22, 33, 38, 46, 51, 55, 60,
	21, 34, 37, 47, 50, 56, 59, 61,
	35, 36, 48, 49, 57, 58, 62, 63
};

/*	This function implements 16 Step division for Q.15 format data */
UINT16 DSP_Division (UINT32 numer, UINT32 denom)
{
	UINT16 i;

	denom <<= 15;

	for (i=16; i>0; i--)
	{
		if (numer > denom)
		{
			numer -= denom;
			numer <<= 1;
			numer++;
		}
		else numer <<= 1;
	}

	return (UINT16) numer;
}

/* Multiply Quantization table with quality factor to get LQT and CQT */
/* Will use constant Quantization tables to make it faster            */
void initialize_quantization_tables (struct JPEG_ENCODER_STRUCTURE * jpeg_encoder_structure)
{
	UINT16 i, index;
	UINT32 value;

/* comment next line to use JPEG default tables*/
#define QCAM_TABLES
	
#ifndef QCAM_TABLES
	/* JPEG default tables (good Quality) */
	static UINT8 luminance_quant_table [] =
	{
		16, 11, 10, 16,  24,  40,  51,  61,
		12, 12, 14, 19,  26,  58,  60,  55,
		14, 13, 16, 24,  40,  57,  69,  56,
		14, 17, 22, 29,  51,  87,  80,  62,
		18, 22, 37, 56,  68, 109, 103,  77,
		24, 35, 55, 64,  81, 104, 113,  92,
		49, 64, 78, 87, 103, 121, 120, 101,
		72, 92, 95, 98, 112, 100, 103,  99
		
	};

	static UINT8 chrominance_quant_table [] =
	{
		17, 18, 24, 47, 99, 99, 99, 99,
		18, 21, 26, 66, 99, 99, 99, 99,
		24, 26, 56, 99, 99, 99, 99, 99,
		47, 66, 99, 99, 99, 99, 99, 99,
		99, 99, 99, 99, 99, 99, 99, 99,
		99, 99, 99, 99, 99, 99, 99, 99,
		99, 99, 99, 99, 99, 99, 99, 99,
		99, 99, 99, 99, 99, 99, 99, 99
	};
#else	
	/* quickcam 5000pro tables (very good quality) */
	static UINT8 luminance_quant_table [] = 
	{
	 0x04, 0x02, 0x03, 0x03, 0x03, 0x02, 0x04, 0x03,
	 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x06, 0x0a,
	 0x06, 0x06, 0x05, 0x05, 0x06, 0x0c, 0x08, 0x09,
	 0x07, 0x0a, 0x0e, 0x0c, 0x0f, 0x0f, 0x0e, 0x0c,
	 0x0e, 0x0f, 0x10, 0x12, 0x17, 0x13, 0x10, 0x11,
	 0x15, 0x11, 0x0d, 0x0e, 0x14, 0x1a, 0x14, 0x15,
	 0x17, 0x18, 0x19, 0x1a, 0x19, 0x0f, 0x13, 0x1c,
	 0x1e, 0x1c, 0x19, 0x1e, 0x17, 0x19, 0x19, 0x18 
	};
	
	static UINT8 chrominance_quant_table [] =
	{
	 0x04, 0x04, 0x04, 0x06, 0x05, 0x06, 0x0b, 0x06, 
	 0x06, 0x0b, 0x18, 0x10, 0x0e, 0x10, 0x18, 0x18, 
	 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
	 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
	 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
	 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
	 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
	 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18	
	};
#endif

/* static UINT8 qtable_jpeg_flattened[] =
 *    { 18,  23,  22,  28,  34,  44,  50,  55,
 * 	 24,  24,  26,  31,  36,  53,  54,  52,
 * 	 26,  25,  28,  34,  44,  53,  58,  52,
 * 	 26,  29,  33,  38,  50,  65,  63,  55,
 * 	 30,  33,  43,  52,  58,  73,  71,  61,
 * 	 34,  41,  52,  56,  63,  71,  74,  67,
 * 	 49,  56,  62,  65,  71,  77,  77,  70,
 * 	 59,  67,  68,  69,  74,  70,  71,  70};
 * 
 * static UINT8 qtable_semiuniform[] =
 *    { 16,  32,  64,  64,  64,  64,  64,  64,
 *      32,  64,  64,  64,  64,  64,  64,  64,
 *      64,  64,  64,  64,  64,  64,  64,  64,
 *      64,  64,  64,  64,  64,  64,  64,  64,
 *      64,  64,  64,  64,  64,  64,  64,  64,
 *      64,  64,  64,  64,  64,  64,  64,  64,
 *      64,  64,  64,  64,  64,  64,  64,  64,
 *      64,  64,  64,  64,  64,  64,  64,  64 };
 * 
 * static UINT8 qtable_uniform[] =
 *    { 100,  100,  100,  100,  100,  100,  100,  100,
 *      100,  100,  100,  100,  100,  100,  100,  100,
 *      100,  100,  100,  100,  100,  100,  100,  100,
 *      100,  100,  100,  100,  100,  100,  100,  100,
 *      100,  100,  100,  100,  100,  100,  100,  100,
 *      100,  100,  100,  100,  100,  100,  100,  100,
 *      100,  100,  100,  100,  100,  100,  100,  100,
 *      100,  100,  100,  100,  100,  100,  100,  100 };
 * 
 * static UINT8 qtable_none[] =
 *    { 1,   1,   1,   1,   1,   1,   1,   1,
 *      1,   1,   1,   1,   1,   1,   1,   1,
 *      1,   1,   1,   1,   1,   1,   1,   1,
 *      1,   1,   1,   1,   1,   1,   1,   1,
 *      1,   1,   1,   1,   1,   1,   1,   1,
 *      1,   1,   1,   1,   1,   1,   1,   1,
 *      1,   1,   1,   1,   1,   1,   1,   1,
 *      1,   1,   1,   1,   1,   1,   1,   1 };
 * 
 * static UINT8 qtable_jpeg_squashed[] = {	//4*sqrt(jpeg_flat)
 * 	18, 19, 19, 21, 23, 27, 28, 30,
 * 	20, 20, 21, 22, 24, 29, 30, 29,
 * 	21, 20, 21, 23, 27, 29, 31, 29,
 * 	21, 22, 23, 25, 28, 32, 32, 30,
 * 	22, 23, 26, 29, 31, 34, 34, 31,
 * 	23, 26, 29, 30, 32, 34, 35, 33,
 * 	28, 30, 32, 32, 34, 35, 35, 34,
 * 	31, 33, 33, 33, 35, 34, 34, 34 };
 */
	
	
	
	
	
	
	for (i=0; i<64; i++)
	{
		index = zigzag_table [i];
		
		value= luminance_quant_table [i];

		jpeg_encoder_structure->Lqt [index] = (UINT8) value;
		jpeg_encoder_structure->ILqt [i] = DSP_Division (0x8000, value);


		value = chrominance_quant_table [i];
				
		jpeg_encoder_structure->Cqt[index] = (UINT8) value;
		jpeg_encoder_structure->ICqt [i] = DSP_Division (0x8000, value);
	}
}

/* multiply DCT Coefficients with Quantization table and store in ZigZag location */
void quantization (struct JPEG_ENCODER_STRUCTURE * jpeg_encoder_structure, INT16* const data, UINT16* const quant_table_ptr)
{
	INT16 i;
	INT32 value;

	for (i=63; i>=0; i--)
	{
		value = data [i] * quant_table_ptr [i];
		value = (value + 0x4000) >> 15;

		jpeg_encoder_structure->Temp [zigzag_table [i]] = (INT16) value;
	}
}
