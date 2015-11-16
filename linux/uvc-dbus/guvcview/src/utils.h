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
#  MJpeg decoding and frame capture taken from luvcview                         #
#                                                                               # 
#                                                                               #
********************************************************************************/

#ifndef UTILS_H
#define UTILS_H

#include "defs.h"

/*video defs*/
//#define BI_RGB 0;
//#define BI_RLE4 1;
//#define BI_RLE8 2;
//#define BI_BITFIELDS 3;

/* Fixed point arithmetic */
//#define FIXED	Sint32
//#define FIXED_BITS 16
//#define TO_FIXED(X) (((Sint32)(X))<<(FIXED_BITS))
//#define FROM_FIXED(X) (((Sint32)(X))>>(FIXED_BITS))

#define ISHIFT 11

#define IFIX(a) ((int)((a) * (1 << ISHIFT) + .5))

#ifndef __P
# define __P(x) x
#endif

/* special markers */
#define M_BADHUFF	-1
#define M_EOF		0x80

struct jpeg_decdata 
{
	int dcts[6 * 64 + 16];
	int out[64 * 6];
	int dquant[3][64];
};

struct in 
{
	BYTE *p;
	DWORD bits;
	int left;
	int marker;
	int (*func) __P((void *));
	void *data;
};

/*********************************/
#define DECBITS 10		/* seems to be the optimum */

struct dec_hufftbl 
{
	int maxcode[17];
	int valptr[16];
	BYTE vals[256];
	DWORD llvals[1 << DECBITS];
};

//struct enc_hufftbl;

union hufftblp 
{
	struct dec_hufftbl *dhuff;
	//struct enc_hufftbl *ehuff;
};

struct scan 
{
	int dc;			/* old dc value */

	union hufftblp hudc;
	union hufftblp huac;
	int next;		/* when to switch to next scan */

	int cid;		/* component id */
	int hv;			/* horiz/vert, copied from comp */
	int tq;			/* quant tbl, copied from comp */
};

/******** Markers *********/
#define M_SOI   0xd8
#define M_APP0  0xe0
#define M_DQT   0xdb
#define M_SOF0  0xc0
#define M_DHT   0xc4
#define M_DRI   0xdd
#define M_SOS   0xda
#define M_RST0  0xd0
#define M_EOI   0xd9
#define M_COM   0xfe

/*******Error codes *******/
#define ERR_NO_SOI 1
#define ERR_NOT_8BIT 2
#define ERR_HEIGHT_MISMATCH 3
#define ERR_WIDTH_MISMATCH 4
#define ERR_BAD_WIDTH_OR_HEIGHT 5
#define ERR_TOO_MANY_COMPPS 6
#define ERR_ILLEGAL_HV 7
#define ERR_QUANT_TABLE_SELECTOR 8
#define ERR_NOT_YCBCR_221111 9
#define ERR_UNKNOWN_CID_IN_SCAN 10
#define ERR_NOT_SEQUENTIAL_DCT 11
#define ERR_WRONG_MARKER 12
#define ERR_NO_EOI 13
#define ERR_BAD_TABLES 14
#define ERR_DEPTH_MISMATCH 15


int jpeg_decode(unsigned char **pic, unsigned char *buf, int width, int height);

#endif

