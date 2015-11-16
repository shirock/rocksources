/*******************************************************************************#
#           guvcview              http://guvcview.berlios.de                    #
#                                                                               #
#           Paulo Assis <pj.assis@gmail.com>                                    #
#           Dr. Alexander K. Seewald <alex@seewald.at>                          #
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
#  autofocus - using dct for sharpness measure                                  #
#                                                                               #
#                                                                               #
********************************************************************************/

#ifndef AUTOFOCUS_H
#define AUTOFOCUS_H

#include "defs.h"

#define MAX_ARR_S 20

struct focusData 
{
	int focus;
	int step;
	int old_focus;
	int right;
	int left;
	int sharpness;
	int focus_sharpness;
	int sharpLeft;
	int sharpRight;
    int id;
	int f_max;
	int f_min;
	int f_step;
	int i_step;
	int focusDir;
	int arr_sharp[MAX_ARR_S];
	int arr_foc[MAX_ARR_S];
	int ind;
	int flag;
	int setFocus;
	int focus_wait;
};

struct focusData *initFocusData (int f_max, int f_min, int step, int id);

int getSharpness (BYTE* img, int width, int height, int t);

int getSharpMeasure (BYTE *img, int width, int height, int t);

int getFocusVal (struct focusData *AFdata);

#endif

