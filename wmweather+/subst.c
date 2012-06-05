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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#include "die.h"
#include "subst.h"

#define GROW(var, len){{ \
    void *c; \
    len<<=1; \
    if(len==0) len=1; \
    if((var=realloc(c=var, len))==NULL){ \
        out=c; \
        warn("realloc error"); \
        goto fail; \
    } \
}}

#define COPY(c) { \
    if(k>=formatlen) GROW(format, formatlen); \
    format[k++]=c; \
}

char *subst(const char *s, struct subst_val *substitutes){
    int i, j, k, n, m;
    char *out=NULL;
    size_t outlen=0;
    char *format=NULL;
    size_t formatlen=0;
    int flags;
    ssize_t str_start;

    for(i=j=0; s[i]!='\0'; i++){
        if(s[i]!='%'){
            if(j>=outlen) GROW(out, outlen);
            out[j++]=s[i];
            continue;
        }
        if(s[i+1]=='%'){
            if(j>=outlen) GROW(out, outlen);
            out[j++]=s[i++];
            continue;
        }

        n=i;
        k=0;
        COPY('%');

        /* skip flags */
        flags=0;
        while(strchr("#0- +'!", s[++n])){
            if(s[n]=='!'){
                flags|=1;
            } else {
                COPY(s[n]);
            }
        }

        /* min width? */
        if(isdigit(s[n]) && s[n]!='0'){
            COPY(s[n]);
            while(isdigit(s[++n])){ COPY(s[n]); }
        }

        /* precision? */
        if(s[n]=='.'){
            COPY('.');
            while(isdigit(s[++n])){ COPY(s[n]); }
        }

        str_start=0;
        if(s[n]=='>'){
            if(s[n+1]=='-'){
                flags|=2;
                n++;
            }
            while(isdigit(s[++n])){
                str_start=str_start*10+s[n]-'0';
            }
            if(flags&2) str_start=-str_start;
        }

        for(m=0; s[n]!=substitutes[m].id && substitutes[m].id!='\0'; m++);
        if(substitutes[m].id=='\0'){
            warn("Unknown substitition character '%c' (at %d)\n", s[n], i);
            goto fail;
        }

        switch(substitutes[m].type){
          case HEX:
          case FLOAT_E:
          case FLOAT_F:
          case FLOAT_G:
          case FLOAT_A:
            if(flags&1){
                COPY(toupper(substitutes[m].type));
                break;
            }
            /* fall through*/

          default:
            COPY(substitutes[m].type);
            break;
        }
        COPY('\0');

#define PRINT(var) { while((k=j+snprintf(out+j, outlen-j, format, var))>=outlen) GROW(out, outlen); j=k; }
        switch(substitutes[m].type){
          case INT:
            PRINT(*(signed int *)substitutes[m].val);
            break;
                  
          case UINT:
          case OCTAL:
          case HEX:
            PRINT(*(unsigned int *)substitutes[m].val);
            break;
            
          case FLOAT_E:
          case FLOAT_F:
          case FLOAT_G:
          case FLOAT_A:
            PRINT(*(double *)substitutes[m].val);
            break;

          case CHAR:
            PRINT(*(char *)substitutes[m].val);
            if(flags&1) out[j-1]=toupper(out[j-1]);
            break;

          case STRING:
            {
                char *s=*(char **)substitutes[m].val;
                if(str_start<0){
                    str_start+=strlen(s);
                    if(str_start<0) str_start=0;
                } else if(str_start>strlen(s)){
                    s="";
                    str_start=0;
                }
                s+=str_start;
                i=j;
                PRINT(s);
                if(flags&1){
                    for(; i<j; i++) out[i]=toupper(out[i]);
                }
            }
            break;

          default:
            warn("Unknown substitution type '%c'\n", substitutes[m].type);
            goto fail;
        }

        i=n;
    }

    free(format);
    if(j>=outlen) GROW(out, outlen);
    out[j]='\0';
    return out;

fail:
    free(format);
    free(out);
    return NULL;
}
