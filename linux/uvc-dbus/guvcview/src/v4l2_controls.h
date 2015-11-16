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
#ifndef V4L2_CONTROLS_H
#define V4L2_CONTROLS_H

#include <linux/videodev2.h>
#include <inttypes.h>

typedef struct _Control
{
    struct v4l2_queryctrl control;
    struct v4l2_querymenu *menu;
    int32_t class;
    int32_t value; //also used for string max size
    int64_t value64;
    char *string;
    //next control in the list
    struct _Control *next;
} Control;

struct VidState
{
    Control *control_list;

    int num_controls;
    int width_req;
    int height_req;
};

/*
 * returns a Control structure NULL terminated linked list
 * with all of the device controls with Read/Write permissions.
 * These are the only ones that we can store/restore.
 * Also sets num_ctrls with the controls count.
 */
Control *get_control_list(int hdevice, int *num_ctrls);

/*
 * Returns the Control structure corresponding to control id,
 * from the control list.
 */
Control *get_ctrl_by_id(Control *control_list, int id);

/*
 * Goes through the control list and gets the controls current values
 */
void get_ctrl_values (int hdevice, Control *control_list, int num_controls, void *all_data);

/*
 * Gets the value for control id
 * and updates control flags and widgets
 */
int get_ctrl(int hdevice, Control *control_list, int id, void *all_data);

Control *get_ctrl_value_by_name(int hdevice, Control *control_list, const char *name);

/*
 * Disables special auto-controls with higher IDs than
 * their absolute/relative counterparts
 * this is needed before restoring controls state
 */
void disable_special_auto (int hdevice, Control *control_list, int id);

/*
 * Goes through the control list and tries to set the controls values
 */
void set_ctrl_values (int hdevice, Control *control_list, int num_controls);

/*
 * sets all controls to default values
 */
void set_default_values(int hdevice, Control *control_list, int num_controls, void *all_data);

/*
 * sets the value for control id
 */
int set_ctrl(int hdevice, Control *control_list, int id);

/*
 * frees the control list allocations
 */
void free_control_list (Control *control_list);

#ifndef V4L2_CID_IRIS_ABSOLUTE
#define V4L2_CID_IRIS_ABSOLUTE		(V4L2_CID_CAMERA_CLASS_BASE +17)
#endif

#ifndef V4L2_CID_IRIS_RELATIVE
#define V4L2_CID_IRIS_RELATIVE		(V4L2_CID_CAMERA_CLASS_BASE +18)
#endif

#ifndef V4L2_CTRL_TYPE_STRING
#define V4L2_CTRL_TYPE_STRING 7
#define DISABLE_STRING_CONTROLS
#endif

#endif
