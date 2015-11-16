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

#include <string.h>
#include <glib.h>
#include <glib/gprintf.h>
#include "defs.h"
//#include "video_format.h"


/* counts chars needed for n*/
int
num_chars (int n)
{
	int i = 0;

	if (n <= 0)
	{
		i++;
		n = -n;
	}

	while (n != 0)
	{
		n /= 10;
		i++;
	}
	return i;
}

/* check image file extension and return image type*/
int
check_image_type (char *filename)
{
	int format=0;
	char str_ext[3];
	/*get the file extension*/
	sscanf(filename,"%*[^.].%3c",str_ext);
	/* change image type */
	int somExt = g_ascii_tolower(str_ext[0])*g_ascii_tolower(str_ext[1])+g_ascii_tolower(str_ext[2]);
	switch (somExt)
	{
		case ('j'*'p'+'g'):
			format=0;
			break;

		case ('b'*'m'+'p'):
			format=1;
			break;

		case ('p'*'n'+'g'):
			format=2;
			break;

		case ('r'*'a'+'w'):
			format=3;
		 	break;

		default: /* use jpeg as default*/
			format=0;
	}

	return (format);
}


/* check video file extension and return video format*/
int
check_video_type (char *filename)
{
	int format=0;
	char str_ext[3];
	/*get the file extension*/
	sscanf(filename,"%*[^.].%3c",str_ext);
	/* change image type */
	int somExt = g_ascii_tolower(str_ext[0])*g_ascii_tolower(str_ext[1])+g_ascii_tolower(str_ext[2]);
	switch (somExt)
	{
		case ('a'*'v'+'i'):
			format=0;
			break;

		case ('m'*'k'+'v'):
			format=1;
			break;

		default: /* use avi as default*/
			format=0;
	}
	return (format);
}

/* split fullpath in Path (splited[1]) and filename (splited[0])*/
pchar* splitPath(char *FullPath, char* splited[2])
{
	char *basename = g_path_get_basename(FullPath);
	char *dirname  = g_path_get_dirname(FullPath);

	int cpysize = 0;
	int size = strlen(basename)+1;

	if (size > (strlen(splited[0])+1))
	{
		/* strlen doesn't count '/0' so add 1 char*/
		splited[0]=g_renew(char, splited[0], size);
	}

	cpysize = g_strlcpy(splited[0], basename, size*sizeof(char));
	if ( (cpysize+1) < (size*sizeof(char)) )
		g_printerr("filename copy size error:(%i != %lu)\n",
			cpysize+1,
			(unsigned long) size*sizeof(char));

	/*only change stored dirname if one is set*/
	if(g_strcmp0(".",dirname)!=0)
	{
		size = strlen(dirname)+1;

		if (size > (strlen(splited[1])+1))
		{
			/* strlen doesn't count '/0' so add 1 char*/
			splited[1]=g_renew(char, splited[1], size);
		}

		cpysize = g_strlcpy(splited[1], dirname, size*sizeof(char));
		if ( (cpysize + 1) < (size*sizeof(char)) )
			g_printerr("dirname copy size error:(%i != %lu)\n",
				cpysize+1,
				(unsigned long) size*sizeof(char));
	}

	g_free(basename);
	g_free(dirname);

	return (splited);
}

char *joinPath(char *fullPath, pchar *splited)
{
	/*clean existing string allocation*/
	g_free(fullPath);

	/*allocate newly formed string*/
	fullPath = g_strjoin ("/", splited[1], splited[0], NULL);

	return (fullPath);
}

char *incFilename(char *fullPath, pchar *splited, int inc)
{
	int fsize=strlen(splited[0]);
	char basename[fsize];
	char extension[4];
	int inc_n_char = num_chars(inc);
	gchar *buffer = g_new0(gchar, inc_n_char+1);/*include '\0' terminator*/
	buffer = g_ascii_dtostr(buffer, inc_n_char+1, inc);

	sscanf(splited[0],"%[^.].%3c", basename, extension);
	extension[3]='\0';/*terminate extension string*/

	g_free (fullPath);
	fullPath=NULL;
	fullPath = g_strjoin("", splited[1], "/", basename,
			"-", buffer, ".", extension, NULL);
	g_free(buffer);

	return(fullPath);
}

char *setImgExt(char *filename, int format)
{
	int sname = strlen(filename)+1; /*include '\0' terminator*/
	char basename[sname];
	sscanf(filename,"%[^.]",basename);
	switch(format)
	{
		case 0:
			g_snprintf(filename, sname, "%s.jpg", basename);
			break;
		case 1:
			g_snprintf(filename, sname, "%s.bmp", basename);
			break;
		case 2:
			g_snprintf(filename, sname, "%s.png", basename);
			break;
		case 3:
			g_snprintf(filename, sname, "%s.raw", basename);
			break;
		default:
			g_printerr("Image format not supported\n");
	}
	return (filename);
}

char *setVidExt(char *filename, int format_ind)
{
	int sname = strlen(filename)+1; /*include '\0' terminator*/
	char basename[sname];
	sscanf(filename,"%[^.]",basename);

//	g_snprintf(filename, sname, "%s.%s", basename, get_vformat_extension(format_ind));
  g_snprintf(filename, sname, "%s.avi", basename);

	return (filename);
}

