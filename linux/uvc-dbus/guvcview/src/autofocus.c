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
#  autofocus - using dct                                                        #
#                                                                               #
#                                                                               #
********************************************************************************/

#include "autofocus.h"
#include "dct.h"
#include "defs.h"
#include "ms_time.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <math.h>

/*use insert sort - it's the fastest for small and almost sorted arrays (our case)*/
#define SORT (3) //1 - Quick sort   2 - Shell sort  3- insert sort  other - bubble sort

#define _TH_		(80) // default treshold = 1/80 of focus sharpness value

#define FLAT 		(0)
#define LOCAL_MAX	(1)
#define LEFT		(2)
#define RIGHT		(3)
#define INCSTEP		(4)

#define SWAP(x, y) temp = (x); (x) = (y); (y) = temp

static double sumAC[64];
static int ACweight[64] = {
	0,1,2,3,4,5,6,7,
	1,1,2,3,4,5,6,7,
	2,2,2,3,4,5,6,7,
	3,3,3,3,4,5,6,7,
	4,4,4,4,4,5,6,7,
	5,5,5,5,5,5,6,7,
	7,7,7,7,7,7,7,7
};

struct focusData *initFocusData (int f_max, int f_min, int step, int id) 
{
	struct focusData *AFdata = g_new0(struct focusData, 1);
	if(AFdata == NULL) 
		return (AFdata);
    AFdata->id = id;
	AFdata->f_max = f_max;
	AFdata->f_min = f_min;
	AFdata->f_step = step;
	AFdata->i_step = (f_max + 1 - f_min)/32;
	if(AFdata->i_step <= step) AFdata->i_step = step * 2;
	//g_printf("focus step:%i\n", AFdata->i_step);
	AFdata->right = f_max;
	AFdata->left = f_min + AFdata->i_step; /*start with focus at 8*/
	AFdata->focus = -1;
	AFdata->focus_wait = 0;
	memset(sumAC,0,64);
	/*all other values are 0 */
	return (AFdata);
}

#if (SORT == 1)
/*quick sort (the fastest and more complex - recursive, doesn't do well on almost sorted data)*/
static void q_sort(struct focusData *AFdata, int left, int right)
{
	int pivot, l_hold, r_hold, temp;

	l_hold = left;
	r_hold = right;
	pivot = AFdata->arr_sharp[left];
	temp = AFdata->arr_foc[left];
	
	while(left < right)
	{
		while((AFdata->arr_sharp[right] >= pivot) && (left < right))
			right--;
		if (left != right)
		{
			AFdata->arr_sharp[left] = AFdata->arr_sharp[right];
			AFdata->arr_foc[left] = AFdata->arr_foc[right];
			left++;
		}
		while((AFdata->arr_sharp[left] <= pivot) && (left < right))
			left++;
		if (left != right)
		{
			AFdata->arr_sharp[right] = AFdata->arr_sharp[left];
			AFdata->arr_foc[right] = AFdata->arr_foc[left];
			right--;
		}
	}
	AFdata->arr_sharp[left] = pivot;
	AFdata->arr_foc[left] = temp;
	pivot = left;
	left = l_hold;
	right = r_hold;
	if (left < pivot) q_sort(AFdata, left, pivot-1);
	if (right > pivot) q_sort(AFdata, pivot+1, right);
}
#elif (SORT == 2)  
/* shell sort (based on insert sort, but with some optimization)*/
/* for small arrays insert sort is still faster */
void s_sort(struct focusData *AFdata, int size)
{
	int i, j, temp, gap;
	
	for (gap = size / 2; gap > 0; gap /= 2)
	{
		for (i = gap; i <= size; i++)
		{
			for (j = i-gap; j >= 0 && (AFdata->arr_sharp[j] > AFdata->arr_sharp[j + gap]); j -= gap) 
			{
				SWAP(AFdata->arr_sharp[j], AFdata->arr_sharp[j + gap]);
				SWAP(AFdata->arr_foc[j], AFdata->arr_foc[j + gap]);
			}
		}
	}
}

#elif (SORT == 3)
/*insert sort (fastest for small arrays, around 15 elements)*/
static void i_sort (struct focusData *AFdata, int size)
{
	int i,j,temp;
	
	for (i = 1; i <= size; i++) 
	{
		for(j = i; j > 0 && (AFdata->arr_sharp[j-1] > AFdata->arr_sharp[j]); j--)
		{
			SWAP(AFdata->arr_sharp[j],AFdata->arr_sharp[j-1]);
			SWAP(AFdata->arr_foc[j],AFdata->arr_foc[j-1]);
		}
	}
}

#else
/*buble sort (the simplest and most inefficient) - in real test with focus data*/
/*it did better than shell or quick sort (focus data is almost sorted)*/
static void b_sort (struct focusData *AFdata, int size) 
{
	int i, temp, swapped;
	
	do 
	{
		swapped = 0;
		size--;
		for (i=0;i<=size;i++) 
		{
			if (AFdata->arr_sharp[i+1] < AFdata->arr_sharp[i]) 
			{
				SWAP(AFdata->arr_sharp[i],AFdata->arr_sharp[i+1]);
				SWAP(AFdata->arr_foc[i],AFdata->arr_foc[i+1]);
				swapped = 1;
			}
		}
	} while (swapped);
}
#endif

static int Sort(struct focusData *AFdata, int size)
{
	if (size>=20) 
	{
		g_printerr("WARNING: focus array size=%d exceeds 20\n",size);
		size = 10;
	}
#if (SORT == 1)
	q_sort(AFdata, 0, size);
#elif (SORT == 2)
	s_sort(AFdata, size);
#elif (SORT == 3)
	i_sort(AFdata, size);
#else
	b_sort(AFdata, size);
#endif
	/*better focus value*/
	return(AFdata->arr_foc[size]);
}

/* extract lum (y) data from image    (YUYV)                */
/* img - image data pointer                                 */
/* dataY - pointer for lum (y) data                         */
/* width - width of img (in pixels)                         */
/* height - height of img (in pixels)                       */
static INT16* extractY (BYTE* img, INT16* dataY, int width, int height) 
{
	int i=0;
	BYTE *pimg;
	pimg=img;
	
	for (i=0;i<(height*width);i++) 
	{
		dataY[i]=(INT16) *pimg++;
		pimg++;
	}
	
	return (dataY);
}

/* measure sharpness in MCU                 */
/* data - MCU data [8x8]                    */
/* t - highest order coef.                  */
static void getSharpnessMCU (INT16 *data, double weight)
{

	int i=0;
	int j=0;

	levelshift (data);
	DCT (data);

	for (i=0;i<8;i++) 
	{
		for(j=0;j<8;j++) 
		{
			sumAC[i*8+j]+=data[i*8+j]*data[i*8+j]*weight;
		}	
	}	
}

/* sharpness in focus window */
int getSharpness (BYTE* img, int width, int height, int t) 
{
	float res=0;
	int numMCUx = width/(8*2); /*covers 1/2 of width - width should be even*/
	int numMCUy = height/(8*2); /*covers 1/2 of height- height should be even*/
	INT16 dataMCU[64];
	INT16* data;
	INT16 dataY[width*height];
	INT16 *Y = dataY;
	
	double weight;
	double xp_;
	double yp_;
	int ctx = numMCUx >> 1; /*center*/ 
	int cty = numMCUy >> 1;
	double rad=ctx/2; 
	if (cty<ctx) { rad=cty/2; }
	rad=rad*rad;
	int cnt2 =0;
	
	data=dataMCU;

	Y = extractY (img, Y, width, height);
	
	int i=0;
	int j=0;
	int xp=0;
	int yp=0;
	/*calculate MCU sharpness*/
	for (yp=0;yp<numMCUy;yp++) 
	{
		yp_=yp-cty;
    		for (xp=0;xp<numMCUx;xp++) 
		{
			xp_=xp-ctx;
			weight = exp(-(xp_*xp_)/rad-(yp_*yp_)/rad);
			for (i=0;i<8;i++) 
			{
				for(j=0;j<8;j++) 
				{
					/*center*/
					dataMCU[i*8+j]=Y[(((height-(numMCUy-(yp*2))*8)>>1)+i)*width
						+(((width-(numMCUx-(xp*2))*8)>>1)+j)];
				}
			}
			getSharpnessMCU(data,weight);
			cnt2++;
		}
	}
	
	for (i=0;i<=t;i++) 
	{
		for(j=0;j<t;j++) 
		{
			sumAC[i*8+j]/=(double) (cnt2); /*average = mean*/
			res+=sumAC[i*8+j]*ACweight[i*8+j];
		}
	}
	return (roundf(res*10)); /*round to int (4 digit precision)*/
}


static int checkFocus(struct focusData *AFdata) 
{
	/*change treshold according to sharpness*/
	int TH = _TH_; 
	//if(AFdata->focus_sharpness < (5 * _TH_)) TH = _TH_ * 4 ;
	
	if (AFdata->step <= AFdata->i_step) 
	{
		if (abs((AFdata->sharpLeft-AFdata->focus_sharpness)<(AFdata->focus_sharpness/TH)) && 
			(abs(AFdata->sharpRight-AFdata->focus_sharpness)<(AFdata->focus_sharpness/TH))) 
		{
			return (FLAT);
		}
		else if (((AFdata->focus_sharpness-AFdata->sharpRight))>=(AFdata->focus_sharpness/TH) && 
			((AFdata->focus_sharpness-AFdata->sharpLeft))>=(AFdata->focus_sharpness/TH)) 
		{
			// significantly down in both directions -> check another step
			// outside for local maximum
			//AFdata->step=16;
			return (INCSTEP);
		}
		else 
		{
			// one is significant, the other is not...
			int left=0; int right=0;
			if (abs((AFdata->sharpLeft-AFdata->focus_sharpness))>=(AFdata->focus_sharpness/TH)) 
			{
				if (AFdata->sharpLeft>AFdata->focus_sharpness) left++;  
				else right++; 
			}
			if (abs((AFdata->sharpRight-AFdata->focus_sharpness))>=(AFdata->focus_sharpness/TH)) 
			{
				if (AFdata->sharpRight>AFdata->focus_sharpness) right++; 
				else left++;
			}
			if (left==right) return (FLAT);
			else if (left>right) return (LEFT);
			else return (RIGHT);
		}
	}
	else 
	{
		if (((AFdata->focus_sharpness-AFdata->sharpRight))>=(AFdata->focus_sharpness/TH) && 
			((AFdata->focus_sharpness-AFdata->sharpLeft))>=(AFdata->focus_sharpness/TH)) 
		{
			return (LOCAL_MAX);
		}
		else 
		{
			return (FLAT);
		}
	}
}

int getFocusVal (struct focusData *AFdata) 
{
	int step = AFdata->i_step * 2;
	int step2 = AFdata->i_step / 2;
	if (step2 <= 0 ) step2 = 1;
	int focus=0;
	
	switch (AFdata->flag) 
	{
		/*--------- first time - run sharpness algorithm -----------------*/
		if(AFdata->ind >= 20) 
		{
			g_printerr ("WARNING ind=%d exceeds 20\n",AFdata->ind);
			AFdata->ind = 10;
		}
		
		case 0: /*sample left to right at higher step*/
			AFdata->arr_sharp[AFdata->ind] = AFdata->sharpness;
			AFdata->arr_foc[AFdata->ind] = AFdata->focus;
			/*reached max focus value*/
			if (AFdata->focus >= AFdata->right ) 
			{	/*get left and right from arr_sharp*/
				focus=Sort(AFdata,AFdata->ind);
				/*get a window around the best value*/
				AFdata->left = (focus- step/2);
				AFdata->right = (focus + step/2);
				if (AFdata->left < AFdata->f_min) AFdata->left = AFdata->f_min;
				if (AFdata->right > AFdata->f_max) AFdata->right = AFdata->f_max;
				AFdata->focus = AFdata->left;
				AFdata->ind=0;
				AFdata->flag = 1;
			} 
			else /*increment focus*/
			{ 
				AFdata->focus=AFdata->arr_foc[AFdata->ind] + step; /*next focus*/
				AFdata->ind++;
				AFdata->flag = 0;
			}
			break;
		case 1: /*sample left to right at lower step - fine tune*/ 
			AFdata->arr_sharp[AFdata->ind] = AFdata->sharpness;
			AFdata->arr_foc[AFdata->ind] = AFdata->focus;
			/*reached window max focus*/
			if (AFdata->focus >= AFdata->right ) 
			{	/*get left and right from arr_sharp*/
				focus=Sort(AFdata,AFdata->ind);
				/*get the best value*/
				AFdata->focus = focus;
				AFdata->focus_sharpness = AFdata->arr_sharp[AFdata->ind];
				AFdata->step = AFdata->i_step; /*first step for focus tracking*/
				AFdata->focusDir = FLAT; /*no direction for focus*/
				AFdata->flag = 2;
			}
			else /*increment focus*/
			{ 
				AFdata->focus=AFdata->arr_foc[AFdata->ind] + step2; /*next focus*/
				AFdata->ind++;
				AFdata->flag = 1;
			}
			break;
		case 2: /* set treshold in order to sharpness*/
			if (AFdata->setFocus) 
			{	
				/*reset*/
				AFdata->setFocus = 0;
				AFdata->flag= 0;
				AFdata->right = AFdata->f_max;
				AFdata->left = AFdata->f_min + AFdata->i_step;
				AFdata->ind = 0;
			}
			else 
			{
				/*track focus*/
				AFdata->focus_sharpness = AFdata->sharpness;
				AFdata->flag = 3;
				AFdata->sharpLeft = 0;
				AFdata->sharpRight = 0;
				AFdata->focus += AFdata->step; /*check right*/
			}
			break;
		case 3:
			/*track focus*/
			AFdata->flag = 4;
			AFdata->sharpRight = AFdata->sharpness;
			AFdata->focus -= (2*AFdata->step); /*check left*/
			break;
		case 4:
			/*track focus*/
			AFdata->sharpLeft=AFdata->sharpness;
			int ret=0;
			ret=checkFocus(AFdata);
			switch (ret) 
			{
				case LOCAL_MAX:
					AFdata->focus += AFdata->step; /*return to orig. focus*/
					AFdata->step = AFdata->i_step;
					AFdata->flag = 2;
					break;
					
				case FLAT:
					if(AFdata->focusDir == FLAT) 
					{
						AFdata->step = AFdata->i_step;
						if(AFdata->focus_sharpness < 4 * _TH_) 
						{
							/* 99% chance we lost focus     */	
							/* move focus to half the range */
							AFdata->focus = AFdata->f_max / 2;
						}
						else
						{
							AFdata->focus += AFdata->step; /*return to orig. focus*/
						}
						AFdata->flag = 2;
					}
					else if (AFdata->focusDir == RIGHT) 
					{
						AFdata->focus += 2*AFdata->step; /*go right*/
						AFdata->step = AFdata->i_step;
						AFdata->flag = 2;
					} 
					else 
					{	/*go left*/
						AFdata->step = AFdata->i_step;
						AFdata->flag = 2;
					}
					break;
					
				case RIGHT:
					AFdata->focus += 2*AFdata->step; /*go right*/
					AFdata->flag = 2;
					break;
					
				case LEFT:
					/*keep focus on left*/
					AFdata->flag = 2;
					break;
					
				case INCSTEP:
					AFdata->focus += AFdata->step; /*return to orig. focus*/
					AFdata->step = 2 * AFdata->i_step;
					AFdata->flag = 2;
					break;
			}
			break;
	}
	/*clip focus, right and left*/
	AFdata->focus=(AFdata->focus > AFdata->f_max) ? AFdata->f_max : ((AFdata->focus < AFdata->f_min) ? AFdata->f_min : AFdata->focus);
	AFdata->right=(AFdata->right > AFdata->f_max) ? AFdata->f_max : ((AFdata->right < AFdata->f_min) ? AFdata->f_min : AFdata->right);
	AFdata->left =(AFdata->left > AFdata->f_max) ? AFdata->f_max : ((AFdata->left < AFdata->f_min) ? AFdata->f_min : AFdata->left);

	return AFdata->focus;
}
