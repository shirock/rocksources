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

#ifndef VIDEO_FILTERS_H
#define VIDEO_FILTERS_H

#include "defs.h"

struct particle
{
	int PX;
	int PY;
	BYTE Y;
	BYTE U;
	BYTE V;
	int size;
	float decay;
};

//struct dizzy_data
//{
//	BYTE *buffer;
//	BYTE *current_buffer, *alt_buffer;
//	int dx;
//	int dy;
//	int sx;
//	int sy;
//	double phase = 0.0;
//	double phase_increment = 0.02;
//	double zoomrate = 1.01;
//};

/* Flip YUYV frame - horizontal
 * args:
 *      frame = pointer to frame buffer (yuyv format)
 *      width = frame width
 *      height= frame height
 * returns: void
 */
void 
yuyv_mirror (BYTE *frame, int width, int height);

/* Flip YUV frame - vertical
 * args:
 *      frame = pointer to frame buffer (yuyv or uyvy format)
 *      width = frame width
 *      height= frame height
 * returns: void
 */
void  
yuyv_upturn(BYTE* frame, int width, int height);

/* Invert YUV frame
 * args:
 *      frame = pointer to frame buffer (yuyv or uyvy format)
 *      width = frame width
 *      height= frame height
 * returns: void
 */
void 
yuyv_negative(BYTE* frame, int width, int height);

/* monochromatic effect for YUYV frame
 * args:
 *      frame = pointer to frame buffer (yuyv format)
 *      width = frame width
 *      height= frame height
 * returns: void
 */
void 
yuyv_monochrome(BYTE* frame, int width, int height);

/*break image in little square pieces
 * args:
 *    frame  = pointer to frame buffer (yuyv or uyvy format)
 *    width  = frame width
 *    height = frame height
 *    piece_size = multiple of 2 (we need at least 2 pixels to get the entire pixel information)
 *    format = v4l2 pixel format
 */
void
pieces(BYTE* frame, int width, int height, int piece_size );

/*sets a trail of particles obtained from the image
 * args:
 *    frame  = pointer to frame buffer (yuyv format)
 *    width  = frame width
 *    height = frame height
 *    trail_size  = trail size (in frames)
 *    particle_size = maximum size in pixels - should be even (square - size x size)
 *    particles = pointer to particles array (struct particle)
 */
struct particle*
particles_effect(BYTE* frame, int width, int height, int trail_size, int particle_size, struct particle* particles);

#endif
