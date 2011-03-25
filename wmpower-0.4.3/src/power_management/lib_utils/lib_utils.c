/***************************************************************************
                          lib_utils.c  -  description
                             -------------------
    begin                : Sun Jan 20 15:34:25 CET 2002
    copyright            : (C) 2002-2004 by Noberasco Michele
    e-mail               : noberasco.gnu@disi.unige.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.              *
 *                                                                         *
 ***************************************************************************/

#define MAX 255

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <envz.h>
#include "lib_utils.h"


char *jump_next_line(char *ptr)
{
	char *temp;

	if (!ptr) return NULL;

	for (temp=ptr; temp[0]!='\0'; temp++)
		if (temp[0] == '\n') return (temp+1);

	return NULL;
}

/* append any number of strings to dst */
char *StrApp (char **dst, ...)
{
  int len = 1;
  char *pt, *temp;
  va_list va;

  if (dst) if (*dst) len += strlen(*dst);
  va_start(va, dst);
  for (;;)
  {
    pt = va_arg(va, char *);
    if (!pt) break;
    len += strlen(pt);
  }

  va_end (va);

	temp = (char *) calloc((size_t)len, sizeof(char));

  if (dst) if (*dst)
  {
    strcpy(temp, *dst);
    free(*dst);
  }

  va_start(va, dst);

  for (;;)
  {
    pt = va_arg(va, char *);
    if (!pt) break;
    strcat(temp, pt);
  }
  va_end (va);

  if (dst) *dst = temp;

  return temp;
}
