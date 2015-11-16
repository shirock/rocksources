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
#  dct for Jpeg encoder                                                         #
#                                                                               # 
#  Adapted for linux, Paulo Assis, 2007 <pj.assis@gmail.com>                    #
********************************************************************************/

#include "dct.h"
#include "defs.h"


/* Level shifting to get 8 bit SIGNED values for the data  */
void levelshift (INT16* const data)
{
	INT16 i;

	for (i=63; i>=0; i--)
		data [i] -= 128;
}

/* DCT for One block(8x8) */
void DCT (INT16 *data)
{
	UINT16 i;
	INT32 x0, x1, x2, x3, x4, x5, x6, x7, x8;
	INT16 *tmp_ptr;
	tmp_ptr=data;
	/*  All values are shifted left by 10   */
	/*  and rounded off to nearest integer  */

	/* scale[0] = 1
	 * scale[k] = cos(k*PI/16)*root(2)
	 */
	static const UINT16 c1=1420;    /* cos PI/16 * root(2)  */
	static const UINT16 c2=1338;    /* cos PI/8 * root(2)   */
	static const UINT16 c3=1204;    /* cos 3PI/16 * root(2) */
	static const UINT16 c5=805;     /* cos 5PI/16 * root(2) */
	static const UINT16 c6=554;     /* cos 3PI/8 * root(2)  */
	static const UINT16 c7=283;     /* cos 7PI/16 * root(2) */

	static const UINT16 s1=3;
	static const UINT16 s2=10;
	static const UINT16 s3=13;


	/* row pass */
	for (i=8; i>0; i--)
	{
		x8 = data [0] + data [7];
		x0 = data [0] - data [7];

		x7 = data [1] + data [6];
		x1 = data [1] - data [6];

		x6 = data [2] + data [5];
		x2 = data [2] - data [5];

		x5 = data [3] + data [4];
		x3 = data [3] - data [4];

		x4 = x8 + x5;
		x8 -= x5;

		x5 = x7 + x6;
		x7 -= x6;

		data [0] = (INT16) (x4 + x5);
		data [4] = (INT16) (x4 - x5);

		data [2] = (INT16) ((x8*c2 + x7*c6) >> s2);
		data [6] = (INT16) ((x8*c6 - x7*c2) >> s2);

		data [7] = (INT16) ((x0*c7 - x1*c5 + x2*c3 - x3*c1) >> s2);
		data [5] = (INT16) ((x0*c5 - x1*c1 + x2*c7 + x3*c3) >> s2);
		data [3] = (INT16) ((x0*c3 - x1*c7 - x2*c1 - x3*c5) >> s2);
		data [1] = (INT16) ((x0*c1 + x1*c3 + x2*c5 + x3*c7) >> s2);

		data += 8;
	}

	data = tmp_ptr;/* return to start of mcu */
	
	/* column pass */
	for (i=8; i>0; i--)
	{
		x8 = data [0] + data [56];
		x0 = data [0] - data [56];

		x7 = data [8] + data [48];
		x1 = data [8] - data [48];

		x6 = data [16] + data [40];
		x2 = data [16] - data [40];

		x5 = data [24] + data [32];
		x3 = data [24] - data [32];

		x4 = x8 + x5;
		x8 -= x5;

		x5 = x7 + x6;
		x7 -= x6;

		data [0] = (INT16) ((x4 + x5) >> s1);
		data [32] = (INT16) ((x4 - x5) >> s1);

		data [16] = (INT16) ((x8*c2 + x7*c6) >> s3);
		data [48] = (INT16) ((x8*c6 - x7*c2) >> s3);

		data [56] = (INT16) ((x0*c7 - x1*c5 + x2*c3 - x3*c1) >> s3);
		data [40] = (INT16) ((x0*c5 - x1*c1 + x2*c7 + x3*c3) >> s3);
		data [24] = (INT16) ((x0*c3 - x1*c7 - x2*c1 - x3*c5) >> s3);
		data [8] = (INT16) ((x0*c1 + x1*c3 + x2*c5 + x3*c7) >> s3);

		data++;
	}
	data=tmp_ptr; /* return to start of mcu */
}
