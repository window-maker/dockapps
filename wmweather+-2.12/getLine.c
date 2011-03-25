#include "config.h"

/*  Copyright (C) 2000  Brad Jorsch <anomie@users.sourceforge.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* inspired by read_line from Eric S. Raymond's fetchmail program, by
 * way of Aaron Sethman's odsclient.  */
int getLine(char **s, FILE *fd){
    int len=8196;
    int toread=len;
    int l=0;
    char *ptr, *p, *q;

    *s=NULL;
    if(feof(fd)) return -1;

    q=ptr=malloc(len);
    if(ptr==NULL) return -1;
    *q=0;
    p=q=ptr;
    while(fgets(p, toread, fd)!=NULL){
	l=strlen(ptr);
        q=strchr(p, '\n');
	if(q!=NULL){ l=q-ptr+1; break; }
        len*=2;
	toread=len-l;
	p=realloc(ptr, len);
	if(p==NULL){
	    free(ptr);
	    return -1;
	}
	ptr=p;
	q=p=ptr+l;
    }
    while(q>ptr && (*q=='\0' || isspace(*q))){
        *q='\0';
	l--;
	q--;
    }
    for(p=ptr; isspace(*p); p++, l--);
    if((*s=malloc(l+1))==NULL){
        free(ptr);
	return -1;
    }
    memcpy(*s, p, l+1);
    free(ptr);
    return l;
}
