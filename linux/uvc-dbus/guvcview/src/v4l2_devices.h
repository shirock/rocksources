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

#ifndef V4L2_DEVICES_H
#define V4L2_DEVICES_H

typedef struct _VidDevice
{
	char *device;
	char *name;
	char *driver;
	char *location;
	guint vendor;
	guint product;
	int valid;
	int current;
} VidDevice;

typedef struct _LDevices
{
	VidDevice *listVidDevices;
	int num_devices;
	int current_device;
} LDevices;

/* enumerates system video devices
 * by checking /sys/class/video4linux
 * args: 
 * videodevice: current device string (default "/dev/video0")
 * 
 * returns: pointer to LDevices struct containing the video devices list */
LDevices *enum_devices( gchar *videodevice );

/*clean video devices list
 * args:
 * listDevices: pointer to LDevices struct containing the video devices list
 *
 * returns: void                                                       */
void freeDevices(LDevices *listDevices);

#endif
