#include "config.h"

/*  Copyright (C) 2002  Brad Jorsch <anomie@users.sourceforge.net>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "wmweather+.h"

int diff(char *file1, char *file2){
    FILE *fp1, *fp2;
    size_t len1, len2;
    struct stat statbuf;
    int ret;
    int len=BIGBUF_LEN/2;
    char *s1=(char *)bigbuf, *s2=(char *)(bigbuf+len);

    if((fp1=fopen(file1, "r"))==NULL) return -1;
    if((fp2=fopen(file2, "r"))==NULL){ fclose(fp1); return -1; }
    if(fstat(fileno(fp1), &statbuf)<0 || !S_ISREG(statbuf.st_mode)){
       fclose(fp1);
       fclose(fp2);
       return -1;
    }
    len1=statbuf.st_size;
    if(fstat(fileno(fp2), &statbuf)<0 || !S_ISREG(statbuf.st_mode)){
       fclose(fp1);
       fclose(fp2);
       return -1;
    }
    len2=statbuf.st_size;
    if(len1!=len2){ fclose(fp1); fclose(fp2); return 1; }
    if(len1==0){ fclose(fp1); fclose(fp2); return 0; }
    while(!feof(fp1) && !feof(fp2)){
        len1=fread(s1, sizeof(char), len, fp1);
        len2=fread(s2, sizeof(char), len, fp2);
        if(len1!=len2 || memcmp(s1, s2, len1)){
            fclose(fp1); fclose(fp2); return 1;
        } 
    }
    ret=(!feof(fp1) || !feof(fp2));
    fclose(fp1);
    fclose(fp2);
    return ret;
}
