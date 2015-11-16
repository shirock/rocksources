/*******************************************************************************#
#           guvcview              http://guvcview.berlios.de                    #
#                                                                               #
#           Paulo Assis <pj.assis@gmail.com>                                    #
#                                                                               #
#           Modifier: rock <shirock.tw@gmail.com>                               #
#             add Pan/Tilt controls                                             #
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

#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <glib/gprintf.h>

#include <glib/gstdio.h>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <libv4l2.h>
#include <errno.h>

#include "guvcview.h"
#include "autofocus.h"
#include "v4l2uvc.h"
#include "string_utils.h"
#include "v4l2_controls.h"
#include "v4l2_dyna_ctrls.h"

/*
 * returns a Control structure NULL terminated linked list
 * with all of the device controls with Read/Write permissions.
 * These are the only ones that we can store/restore.
 * Also sets num_ctrls with the controls count.
 */
Control *get_control_list(int hdevice, int *num_ctrls)
{
    int ret=0;
    Control *first   = NULL;
    Control *current = NULL;
    Control *control = NULL;

    int n = 0;
    struct v4l2_queryctrl queryctrl={0};
    struct v4l2_querymenu querymenu={0};

    queryctrl.id = 0 | V4L2_CTRL_FLAG_NEXT_CTRL;

    if ((ret=xioctl (hdevice, VIDIOC_QUERYCTRL, &queryctrl)) == 0)
    {
        // The driver supports the V4L2_CTRL_FLAG_NEXT_CTRL flag
        queryctrl.id = 0;
        int currentctrl= queryctrl.id;
        queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;

        while((ret = xioctl(hdevice, VIDIOC_QUERYCTRL, &queryctrl)), ret ? errno != EINVAL : 1)
        {
            struct v4l2_querymenu *menu = NULL;

            // Prevent infinite loop for buggy NEXT_CTRL implementations
            if(ret && queryctrl.id <= currentctrl)
            {
                currentctrl++;
                goto next_control;
            }
            currentctrl = queryctrl.id;

            // skip if control is disabled or failed
            if (ret || (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) )
                goto next_control;

            //check menu items if needed
            if(queryctrl.type == V4L2_CTRL_TYPE_MENU)
            {
                menu = calloc((queryctrl.maximum - queryctrl.minimum) + 1, sizeof(struct v4l2_querymenu));
                int i = 0;
                for (querymenu.index = queryctrl.minimum;
                    querymenu.index <= queryctrl.maximum;
                    querymenu.index++)
                {
                    querymenu.id = queryctrl.id;
                    if (0 == xioctl (hdevice, VIDIOC_QUERYMENU, &querymenu))
                    {
                        memcpy(&(menu[i]), &querymenu, sizeof(struct v4l2_querymenu));
                        i++;
                    }
                    else
                    {
                        perror ("VIDIOC_QUERYMENU");
                        free (menu);
                        menu = NULL;
                        goto next_control;
                    }
                }
            }

            // Add the control to the linked list
            control = calloc (1, sizeof(Control));
            memcpy(&(control->control), &queryctrl, sizeof(struct v4l2_queryctrl));
            control->class = (control->control.id & 0xFFFF0000);
            //add the menu adress (NULL if not a menu)
            control->menu = menu;
#ifndef DISABLE_STRING_CONTROLS
            //allocate a string with max size if needed
            if(control->control.type == V4L2_CTRL_TYPE_STRING)
                control->string = calloc(control->control.maximum + 1, sizeof(char));
            else
#endif
                control->string = NULL;

            if(first != NULL)
            {
                current->next = control;
                current = control;
            }
            else
            {
                first = control;
                current = first;
            }

            n++;

next_control:
            queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
        }
    }
    else
    {
        printf("NEXT_CTRL flag not supported\n");
        int currentctrl;
        for(currentctrl = V4L2_CID_BASE; currentctrl < V4L2_CID_LASTP1; currentctrl++)
        {
            struct v4l2_querymenu *menu = NULL;
            queryctrl.id = currentctrl;
            ret = xioctl(hdevice, VIDIOC_QUERYCTRL, &queryctrl);

            if (ret || (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) )
                continue;

            //check menu items if needed
            if(queryctrl.type == V4L2_CTRL_TYPE_MENU)
            {
                menu = calloc((queryctrl.maximum - queryctrl.minimum) + 1, sizeof(struct v4l2_querymenu));
                int i = 0;
                for (querymenu.index = queryctrl.minimum;
                    querymenu.index <= queryctrl.maximum;
                    querymenu.index++)
                {
                    querymenu.id = queryctrl.id;
                    if (0 == xioctl (hdevice, VIDIOC_QUERYMENU, &querymenu))
                    {
                        memcpy(&(menu[i]), &querymenu, sizeof(struct v4l2_querymenu));
                        i++;
                    }
                    else
                    {
                        perror ("VIDIOC_QUERYMENU");
                        free (menu);
                        menu = NULL;
                        querymenu.index = queryctrl.maximum + 2; //exits loop
                    }
                }

                if(querymenu.index > (queryctrl.maximum + 1))
                    continue; //query menu failed
            }

            // Add the control to the linked list
            control = calloc (1, sizeof(Control));
            memcpy(&(control->control), &queryctrl, sizeof(struct v4l2_queryctrl));
            control->class = 0x00980000;
            //add the menu adress (NULL if not a menu)
            control->menu = menu;

            if(first != NULL)
            {
                current->next = control;
                current = control;
            }
            else
            {
                first = control;
                current = first;
            }

            n++;
        }

        for (queryctrl.id = V4L2_CID_PRIVATE_BASE;;queryctrl.id++)
        {
            struct v4l2_querymenu *menu = NULL;
            ret = xioctl(hdevice, VIDIOC_QUERYCTRL, &queryctrl);
            if(ret)
                break;
            else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
                continue;

            //check menu items if needed
            if(queryctrl.type == V4L2_CTRL_TYPE_MENU)
            {
                menu = calloc((queryctrl.maximum - queryctrl.minimum) + 1, sizeof(struct v4l2_querymenu));
                int i = 0;
                for (querymenu.index = queryctrl.minimum;
                    querymenu.index <= queryctrl.maximum;
                    querymenu.index++)
                {
                    querymenu.id = queryctrl.id;
                    if (0 == xioctl (hdevice, VIDIOC_QUERYMENU, &querymenu))
                    {
                        memcpy(&(menu[i]), &querymenu, sizeof(struct v4l2_querymenu));
                        i++;
                    }
                    else
                    {
                        perror ("VIDIOC_QUERYMENU");
                        free (menu);
                        menu = NULL;
                        querymenu.index = queryctrl.maximum + 2; //exits loop
                    }
                }

                if(querymenu.index > (queryctrl.maximum + 1))
                    continue; //query menu failed
            }

            // Add the control to the linked list
            control = calloc (1, sizeof(Control));
            memcpy(&(control->control), &queryctrl, sizeof(struct v4l2_queryctrl));
            control->class = 0x00980000;
            //add the menu adress (NULL if not a menu)
            control->menu = menu;

            if(first != NULL)
            {
                current->next = control;
                current = control;
            }
            else
            {
                first = control;
                current = first;
            }

            n++;
        }
    }

    *num_ctrls = n;
    return first;
}

/*
 * called when setting controls
 */
static void update_ctrl_flags(Control *control_list, int id)
{
    switch (id)
    {
        case V4L2_CID_EXPOSURE_AUTO:
            {
                Control *ctrl_this = get_ctrl_by_id(control_list, id );
                if(ctrl_this == NULL)
                    break;

                switch (ctrl_this->value)
                {
                    case V4L2_EXPOSURE_AUTO:
                        {
                            //printf("auto\n");
                            Control *ctrl_that = get_ctrl_by_id(control_list,
                                V4L2_CID_IRIS_ABSOLUTE );
                            if (ctrl_that)
                                ctrl_that->control.flags |= V4L2_CTRL_FLAG_GRABBED;

                            ctrl_that = get_ctrl_by_id(control_list,
                                V4L2_CID_IRIS_RELATIVE );
                            if (ctrl_that)
                                ctrl_that->control.flags |= V4L2_CTRL_FLAG_GRABBED;
                            ctrl_that = get_ctrl_by_id(control_list,
                                V4L2_CID_EXPOSURE_ABSOLUTE );
                            if (ctrl_that)
                                ctrl_that->control.flags |= V4L2_CTRL_FLAG_GRABBED;
                        }
                        break;

                    case V4L2_EXPOSURE_APERTURE_PRIORITY:
                        {
                            //printf("AP\n");
                            Control *ctrl_that = get_ctrl_by_id(control_list,
                                V4L2_CID_EXPOSURE_ABSOLUTE );
                            if (ctrl_that)
                                ctrl_that->control.flags |= V4L2_CTRL_FLAG_GRABBED;
                            ctrl_that = get_ctrl_by_id(control_list,
                                V4L2_CID_IRIS_ABSOLUTE );
                            if (ctrl_that)
                                ctrl_that->control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                            ctrl_that = get_ctrl_by_id(control_list,
                                V4L2_CID_IRIS_RELATIVE );
                            if (ctrl_that)
                                ctrl_that->control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                        }
                        break;

                    case V4L2_EXPOSURE_SHUTTER_PRIORITY:
                        {
                            //printf("SP\n");
                            Control *ctrl_that = get_ctrl_by_id(control_list,
                                V4L2_CID_IRIS_ABSOLUTE );
                            if (ctrl_that)
                                ctrl_that->control.flags |= V4L2_CTRL_FLAG_GRABBED;

                            ctrl_that = get_ctrl_by_id(control_list,
                                V4L2_CID_IRIS_RELATIVE );
                            if (ctrl_that)
                                ctrl_that->control.flags |= V4L2_CTRL_FLAG_GRABBED;
                            ctrl_that = get_ctrl_by_id(control_list,
                                V4L2_CID_EXPOSURE_ABSOLUTE );
                            if (ctrl_that)
                                ctrl_that->control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                        }
                        break;

                    default:
                        {
                            //printf("manual\n");
                            Control *ctrl_that = get_ctrl_by_id(control_list,
                                V4L2_CID_EXPOSURE_ABSOLUTE );
                            if (ctrl_that)
                                ctrl_that->control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                            ctrl_that = get_ctrl_by_id(control_list,
                                V4L2_CID_IRIS_ABSOLUTE );
                            if (ctrl_that)
                                ctrl_that->control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                            ctrl_that = get_ctrl_by_id(control_list,
                                V4L2_CID_IRIS_RELATIVE );
                            if (ctrl_that)
                                ctrl_that->control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                        }
                        break;
                }
            }
            break;

        case V4L2_CID_FOCUS_AUTO:
            {
                Control *ctrl_this = get_ctrl_by_id(control_list, id );
                if(ctrl_this == NULL)
                    break;
                if(ctrl_this->value > 0)
                {
                    Control *ctrl_that = get_ctrl_by_id(control_list,
                        V4L2_CID_FOCUS_ABSOLUTE);
                    if (ctrl_that)
                        ctrl_that->control.flags |= V4L2_CTRL_FLAG_GRABBED;

                    ctrl_that = get_ctrl_by_id(control_list,
                        V4L2_CID_FOCUS_RELATIVE);
                    if (ctrl_that)
                        ctrl_that->control.flags |= V4L2_CTRL_FLAG_GRABBED;
                }
                else
                {
                    Control *ctrl_that = get_ctrl_by_id(control_list,
                        V4L2_CID_FOCUS_ABSOLUTE);
                    if (ctrl_that)
                        ctrl_that->control.flags &= !(V4L2_CTRL_FLAG_GRABBED);

                    ctrl_that = get_ctrl_by_id(control_list,
                        V4L2_CID_FOCUS_RELATIVE);
                    if (ctrl_that)
                        ctrl_that->control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                }
            }
            break;

        case V4L2_CID_HUE_AUTO:
            {
                Control *ctrl_this = get_ctrl_by_id(control_list, id );
                if(ctrl_this == NULL)
                    break;
                if(ctrl_this->value > 0)
                {
                    Control *ctrl_that = get_ctrl_by_id(control_list,
                        V4L2_CID_HUE);
                    if (ctrl_that)
                        ctrl_that->control.flags |= V4L2_CTRL_FLAG_GRABBED;
                }
                else
                {
                    Control *ctrl_that = get_ctrl_by_id(control_list,
                        V4L2_CID_HUE);
                    if (ctrl_that)
                        ctrl_that->control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                }
            }
            break;

        case V4L2_CID_AUTO_WHITE_BALANCE:
            {
                Control *ctrl_this = get_ctrl_by_id(control_list, id );
                if(ctrl_this == NULL)
                    break;

                if(ctrl_this->value > 0)
                {
                    Control *ctrl_that = get_ctrl_by_id(control_list,
                        V4L2_CID_WHITE_BALANCE_TEMPERATURE);
                    if (ctrl_that)
                        ctrl_that->control.flags |= V4L2_CTRL_FLAG_GRABBED;
                    ctrl_that = get_ctrl_by_id(control_list,
                        V4L2_CID_BLUE_BALANCE);
                    if (ctrl_that)
                        ctrl_that->control.flags |= V4L2_CTRL_FLAG_GRABBED;
                    ctrl_that = get_ctrl_by_id(control_list,
                        V4L2_CID_RED_BALANCE);
                    if (ctrl_that)
                        ctrl_that->control.flags |= V4L2_CTRL_FLAG_GRABBED;
                }
                else
                {
                    Control *ctrl_that = get_ctrl_by_id(control_list,
                        V4L2_CID_WHITE_BALANCE_TEMPERATURE);
                    if (ctrl_that)
                        ctrl_that->control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                    ctrl_that = get_ctrl_by_id(control_list,
                        V4L2_CID_BLUE_BALANCE);
                    if (ctrl_that)
                        ctrl_that->control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                    ctrl_that = get_ctrl_by_id(control_list,
                        V4L2_CID_RED_BALANCE);
                    if (ctrl_that)
                        ctrl_that->control.flags &= !(V4L2_CTRL_FLAG_GRABBED);
                }
            }
            break;
    }
}

/*
 * update flags of entire control list
 */
static void update_ctrl_list_flags(Control *control_list)
{
    Control *current = control_list;
    Control *next = current->next;
    int done = 0;

    while(!done)
    {
        update_ctrl_flags(control_list, current->control.id);

        if(next == NULL)
            done = 1;
        else
        {
            current = next;
            next = current->next;
        }
    }
}

/*
 * Disables special auto-controls with higher IDs than
 * their absolute/relative counterparts
 * this is needed before restoring controls state
 */
void disable_special_auto (int hdevice, Control *control_list, int id)
{
    Control *current = get_ctrl_by_id(control_list, id);
    if(current && ((id == V4L2_CID_FOCUS_AUTO) || (id == V4L2_CID_HUE_AUTO)))
    {
        current->value = 0;
        set_ctrl(hdevice, control_list, id);
    }
}

/*
 * Returns the Control structure corresponding to control id,
 * from the control list.
 */
Control *get_ctrl_by_id(Control *control_list, int id)
{
    Control *current = control_list;
    Control *next = current->next;
    while (next != NULL)
    {
        if(current->control.id == id)
        {
            return (current);
        }
        current = next;
        next = current->next;
    }
    if(current->control.id == id)
        return (current);
    else//no id match
        return(NULL);
}

/*
 * Goes through the control list and gets the controls current values
 * also updates flags and widget states
 */
void get_ctrl_values (int hdevice, Control *control_list, int num_controls, void *all_data)
{
    int ret = 0;
    struct v4l2_ext_control clist[num_controls];
    Control *current = control_list;
    Control *next = current->next;
    int count = 0;
    int i = 0;
    int done = 0;

    while(!done)
    {
        if(current->control.flags & V4L2_CTRL_FLAG_WRITE_ONLY)
            goto next_control;

        clist[count].id = current->control.id;
#ifndef DISABLE_STRING_CONTROLS
        clist[count].size = 0;
        if(current->control.type == V4L2_CTRL_TYPE_STRING)
        {
            clist[count].size = current->control.maximum + 1;
            clist[count].string = current->string;
        }
#endif
        count++;

        if((next == NULL) || (next->class != current->class))
        {
            struct v4l2_ext_controls ctrls = {0};
            ctrls.ctrl_class = current->class;
            ctrls.count = count;
            ctrls.controls = clist;
            ret = xioctl(hdevice, VIDIOC_G_EXT_CTRLS, &ctrls);
            if(ret)
            {
                printf("VIDIOC_G_EXT_CTRLS failed\n");
                struct v4l2_control ctrl;
                //get the controls one by one
                if( current->class == V4L2_CTRL_CLASS_USER)
                {
                    printf("   using VIDIOC_G_CTRL for user class controls\n");
                    for(i=0; i < count; i++)
                    {
                        ctrl.id = clist[i].id;
                        ctrl.value = 0;
                        ret = xioctl(hdevice, VIDIOC_G_CTRL, &ctrl);
                        if(ret)
                            continue;
                        clist[i].value = ctrl.value;
                    }
                }
                else
                {
                    printf("   using VIDIOC_G_EXT_CTRLS on single controls for class: 0x%08x\n",
                        current->class);
                    for(i=0;i < count; i++)
                    {
                        ctrls.count = 1;
                        ctrls.controls = &clist[i];
                        ret = xioctl(hdevice, VIDIOC_G_EXT_CTRLS, &ctrls);
                        if(ret)
                            printf("control id: 0x%08x failed to set (error %i)\n",
                                clist[i].id, ret);
                    }
                }
            }

            //fill in the values on the control list
            for(i=0; i<count; i++)
            {
                Control *ctrl = get_ctrl_by_id(control_list, clist[i].id);
                if(!ctrl)
                {
                    printf("couldn't get control for id: %i\n", clist[i].id);
                    continue;
                }
                switch(ctrl->control.type)
                {
#ifndef DISABLE_STRING_CONTROLS
                    case V4L2_CTRL_TYPE_STRING:
                        //string gets set on VIDIOC_G_EXT_CTRLS
                        //add the maximum size to value
                        ctrl->value = clist[i].size;
                        break;
#endif
                    case V4L2_CTRL_TYPE_INTEGER64:
                        ctrl->value64 = clist[i].value64;
                        break;
                    default:
                        ctrl->value = clist[i].value;
                        //printf("control %i [0x%08x] = %i\n",
                        //    i, clist[i].id, clist[i].value);
                        break;
                }
            }

            count = 0;

            if(next == NULL)
                done = 1;
        }

next_control:
        if(!done)
        {
            current = next;
            next = current->next;
        }
    }

    update_ctrl_list_flags(control_list);
//    update_widget_state(control_list, all_data);

}

/*
 * Gets the value for control id
 * and updates control flags and widgets
 */
int get_ctrl(int hdevice, Control *control_list, int id, void *all_data)
{
    Control *control = get_ctrl_by_id(control_list, id );
    int ret = 0;

    if(!control)
        return (-1);
    if(control->control.flags & V4L2_CTRL_FLAG_WRITE_ONLY)
        return (-1);

    if( control->class == V4L2_CTRL_CLASS_USER)
    {
        struct v4l2_control ctrl;
        //printf("   using VIDIOC_G_CTRL for user class controls\n");
        ctrl.id = control->control.id;
        ctrl.value = 0;
        ret = xioctl(hdevice, VIDIOC_G_CTRL, &ctrl);
        if(ret)
            printf("control id: 0x%08x failed to get value (error %i)\n",
                ctrl.id, ret);
        else
            control->value = ctrl.value;
    }
    else
    {
        //printf("   using VIDIOC_G_EXT_CTRLS on single controls for class: 0x%08x\n",
        //    current->class);
        struct v4l2_ext_controls ctrls = {0};
        struct v4l2_ext_control ctrl = {0};
        ctrl.id = control->control.id;
#ifndef DISABLE_STRING_CONTROLS
        ctrl.size = 0;
        if(control->control.type == V4L2_CTRL_TYPE_STRING)
        {
            ctrl.size = control->control.maximum + 1;
            ctrl.string = control->string;
        }
#endif
        ctrls.count = 1;
        ctrls.controls = &ctrl;
        ret = xioctl(hdevice, VIDIOC_G_EXT_CTRLS, &ctrls);
        if(ret)
            printf("control id: 0x%08x failed to get value (error %i)\n",
                ctrl.id, ret);
        else
        {
            switch(control->control.type)
            {
#ifndef DISABLE_STRING_CONTROLS
                case V4L2_CTRL_TYPE_STRING:
                    //string gets set on VIDIOC_G_EXT_CTRLS
                    //add the maximum size to value
                    control->value = ctrl.size;
                    break;
#endif
                case V4L2_CTRL_TYPE_INTEGER64:
                    control->value64 = ctrl.value64;
                    break;
                default:
                    control->value = ctrl.value;
                    //printf("control %i [0x%08x] = %i\n",
                    //    i, clist[i].id, clist[i].value);
                    break;
            }
        }
    }

    update_ctrl_flags(control_list, id);
//    update_widget_state(control_list, all_data);

    return (ret);
}

// TODO #1375
Control *get_ctrl_value_by_name(int hdevice, Control *control_list, const char *name)
{
    Control *current = control_list;
    Control *next = current->next;
    Control *ctrl = NULL;
    while (next != NULL)
    {
        //if(current->control.id == id)
        if (strcmp((char*)current->control.name, name) == 0)
        {
            //return (current);
            ctrl = current;
            break;
        }
        current = next;
        next = current->next;
    }
    //if(current->control.id == id)
    if (strcmp((char*)current->control.name, name) == 0)
        //return (current);
        ctrl = current;
    else//no id match
        return(NULL);

    get_ctrl(hdevice, control_list, ctrl->control.id, NULL);
    return ctrl;
}

/*
 * Goes through the control list and tries to set the controls values
 */
void set_ctrl_values (int hdevice, Control *control_list, int num_controls)
{
    int ret = 0;
    struct v4l2_ext_control clist[num_controls];
    Control *current = control_list;
    Control *next = current->next;
    int count = 0;
    int i = 0;
    int done = 0;

    while(!done)
    {
        if(current->control.flags & V4L2_CTRL_FLAG_READ_ONLY)
            goto next_control;

        clist[count].id = current->control.id;
        switch (current->control.type)
        {
#ifndef DISABLE_STRING_CONTROLS
            case V4L2_CTRL_TYPE_STRING:
                clist[count].size = current->value;
                clist[count].string = current->string;
                break;
#endif
            case V4L2_CTRL_TYPE_INTEGER64:
                clist[count].value64 = current->value64;
                break;
            default:
                clist[count].value = current->value;
                break;
        }
        count++;

        if((next == NULL) || (next->class != current->class))
        {
            struct v4l2_ext_controls ctrls = {0};
            ctrls.ctrl_class = current->class;
            ctrls.count = count;
            ctrls.controls = clist;
            ret = xioctl(hdevice, VIDIOC_S_EXT_CTRLS, &ctrls);
            if(ret)
            {
                printf("VIDIOC_S_EXT_CTRLS for multiple controls failed (error %i)\n", ret);
                struct v4l2_control ctrl;
                //set the controls one by one
                if( current->class == V4L2_CTRL_CLASS_USER)
                {
                    printf("   using VIDIOC_S_CTRL for user class controls\n");
                    for(i=0;i < count; i++)
                    {
                        ctrl.id = clist[i].id;
                        ctrl.value = clist[i].value;
                        ret = xioctl(hdevice, VIDIOC_S_CTRL, &ctrl);
                        if(ret)
                        {
                            Control *ctrl = get_ctrl_by_id(control_list, clist[i].id);
                            if(ctrl)
                                printf("control(0x%08x) \"%s\" failed to set (error %i)\n",
                                    clist[i].id, ctrl->control.name, ret);
                            else
                              printf("control(0x%08x) failed to set (error %i)\n",
                                    clist[i].id, ret);
                        }
                    }
                }
                else
                {
                    printf("   using VIDIOC_S_EXT_CTRLS on single controls for class: 0x%08x\n",
                        current->class);
                    for(i=0;i < count; i++)
                    {
                        ctrls.count = 1;
                        ctrls.controls = &clist[i];
                        ret = xioctl(hdevice, VIDIOC_S_EXT_CTRLS, &ctrls);
                        if(ret)
                        {
                            Control *ctrl = get_ctrl_by_id(control_list, clist[i].id);
                            if(ctrl)
                                printf("control(0x%08x) \"%s\" failed to set (error %i)\n",
                                    clist[i].id, ctrl->control.name, ret);
                            else
                              printf("control(0x%08x) failed to set (error %i)\n",
                                    clist[i].id, ret);
                        }
                    }
                }
            }



            count = 0;

            if(next == NULL)
                done = 1;
        }

next_control:
        if(!done)
        {
            current = next;
            next = current->next;
        }
    }

    //update list with real values
    //get_ctrl_values (hdevice, control_list, num_controls);
}

/*
 * sets all controls to default values
 */
void set_default_values(int hdevice, Control *control_list, int num_controls, void *all_data)
{
    Control *current = control_list;
    Control *next = current->next;
    int done = 0;

    while(!done)
    {
        if(current->control.flags & V4L2_CTRL_FLAG_READ_ONLY)
        {
            if(next == NULL)
                break;
            else
            {
                current = next;
                next = current->next;
            }
            continue;
        }
        //printf("setting 0x%08X to %i\n",current->control.id, current->control.default_value);
        switch (current->control.type)
        {
#ifndef DISABLE_STRING_CONTROLS
            case V4L2_CTRL_TYPE_STRING:
                break;
#endif
            case V4L2_CTRL_TYPE_INTEGER64:
                current->value64 = current->control.default_value;
                break;
            default:
                //if its one of the special auto controls disable it first
                disable_special_auto (hdevice, control_list, current->control.id);
                current->value = current->control.default_value;
                break;
        }

        if(next == NULL)
            done = 1;
        else
        {
            current = next;
            next = current->next;
        }
    }

    set_ctrl_values (hdevice, control_list, num_controls);
    get_ctrl_values (hdevice, control_list, num_controls, all_data);

}

/*
 * sets the value for control id
 */
int set_ctrl(int hdevice, Control *control_list, int id)
{
    Control *control = get_ctrl_by_id(control_list, id );
    int ret = 0;

    if(!control)
        return (-1);
    if(control->control.flags & V4L2_CTRL_FLAG_READ_ONLY)
        return (-1);

    if( control->class == V4L2_CTRL_CLASS_USER)
    {
        struct v4l2_control ctrl;
        //printf("   using VIDIOC_G_CTRL for user class controls\n");
        ctrl.id = control->control.id;
        ctrl.value = control->value;
        ret = xioctl(hdevice, VIDIOC_S_CTRL, &ctrl);
    }
    else
    {
        //printf("   using VIDIOC_G_EXT_CTRLS on single controls for class: 0x%08x\n",
        //    current->class);
        struct v4l2_ext_controls ctrls = {0};
        struct v4l2_ext_control ctrl = {0};
        ctrl.id = control->control.id;
        switch (control->control.type)
        {
#ifndef DISABLE_STRING_CONTROLS
            case V4L2_CTRL_TYPE_STRING:
                ctrl.size = control->value;
                ctrl.string = control->string;
                break;
#endif
            case V4L2_CTRL_TYPE_INTEGER64:
                ctrl.value64 = control->value64;
                break;
            default:
                ctrl.value = control->value;
                break;
        }
        ctrls.count = 1;
        ctrls.controls = &ctrl;
        ret = xioctl(hdevice, VIDIOC_S_EXT_CTRLS, &ctrls);
        if(ret)
            printf("control id: 0x%08x failed to set (error %i)\n",
                ctrl.id, ret);
    }

    //update real value
    get_ctrl(hdevice, control_list, id, NULL);

    return (ret);
}

/*
 * frees the control list allocations
 */
void free_control_list (Control *control_list)
{
    Control *first = control_list;
    Control *next = first->next;
    while (next != NULL)
    {
        if(first->string) free(first->string);
        if(first->menu) free(first->menu);
        free(first);
        first = next;
        next = first->next;
    }
    //clean the last one
    if(first->string) free(first->string);
    if(first) free(first);
    control_list = NULL;
}

// init_controls()
// base on draw_controls@img_controls.c
void
init_controls (struct ALL_DATA *all_data)
{
	struct VidState *s = all_data->s;
	struct GLOBAL *global = all_data->global;
	struct vdIn *videoIn = all_data->videoIn;
	struct focusData *AFdata = all_data->AFdata;

    if (s->control_list)
    {
        free_control_list(s->control_list);
    }
    s->num_controls = 0;
    //get the control list
    s->control_list = get_control_list(videoIn->fd, &(s->num_controls));

    if(!s->control_list)
    {
        printf("Error: empty control list\n");
        return;
    }

    get_ctrl_values (videoIn->fd, s->control_list, s->num_controls, NULL);

    int done = 0;
    int row=0;

    Control *current = s->control_list;
    Control *next = current->next;

    while(!done)
    {
#if 0
        printf("control name: %s; type: %d;\n", current->control.name, current->control.type);
        switch (current->control.type)
        {
#ifndef DISABLE_STRING_CONTROLS
            case V4L2_CTRL_TYPE_STRING: // 7
                printf("\tstring: %s\n", current->string);
                break;
#endif
            case V4L2_CTRL_TYPE_INTEGER64: // 5
                printf("\tint64: %ld\n", current->value64);
                break;
            default:
                printf("\tint32: %d\n", current->value);
                break;
        }
#endif
        //add some flags
        if ((current->control.id == V4L2_CID_PAN_RELATIVE) ||
	        (current->control.id == V4L2_CID_TILT_RELATIVE))
	    {
	        videoIn->PanTilt++;
	    }

        //special cases (extra software controls)
	    if (((current->control.id == V4L2_CID_FOCUS_ABSOLUTE) ||
	        (current->control.id == V4L2_CID_FOCUS_LOGITECH)) &&
	         !(global->control_only))
	    {
		    global->AFcontrol=1;

			if(!AFdata)
			{
			    AFdata = initFocusData(current->control.maximum,
			        current->control.minimum,
			        current->control.step,
			        current->control.id);
				all_data->AFdata = AFdata;
			}

			if(!AFdata)
			    global->AFcontrol = 0;
			else
			{
				//printf("global->autofocus: %s\n", global->autofocus ? "Yes" : "No");
				row++; /*increment control row*/

			}
		}

        if(next == NULL)
            done = 1;
        else
        {
            row++;
            current = next;
            next = current->next;
        }
    }

	/*              try to start the video stream             */
	/* do it here (after all ioctls) since some cameras take  */
	/* a long time to initialize after this                   */
	/* it's OK if it fails since it is retried in uvcGrab     */
	if(!global->control_only) video_enable(videoIn);

	s = NULL;
	global = NULL;
	videoIn = NULL;
}
