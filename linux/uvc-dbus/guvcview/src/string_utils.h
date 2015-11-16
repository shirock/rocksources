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

#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include "defs.h"

int
num_chars (int n);

/* check image file extension and return image type*/
int 
check_image_type (char *filename);

/* check video file extension and return video format*/
int 
check_video_type (char *filename);

/* split fullpath in Path (splited[1]) and filename (splited[0])*/
pchar* 
splitPath(char *FullPath, char* splited[2]);

/*join splited path into fullpath*/
char *
joinPath(char *fullPath, pchar * splited);

/*increment file name with inc*/
char *
incFilename(char *fullPath, pchar *splited, int inc);

char *
setImgExt(char *filename, int format);

char *
setVidExt(char *filename, int format_ind);

#endif
