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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>

#include "wmgeneral/wmgeneral-x11.h"
#include "wmgeneral/xpm_trans.h"

#include "wmweather+.h"
#include "animation.h"
#include "moon.h"
#include "font.h"

#define P 4   /* Percent to increment by */
#define X 4   /* Frames to wait before incrementing */
#define M (((100/P)+1)*X)

static int heights[5]={ 11, 12, 14, 14, 19 };

void SetAnimation(struct animation *a, int x, int y, int sky, int obs, int vis,
                  int frz, int snow, int rain, int tstorm, int svtstorm,
                  double moon){
    int i;

    a->changed=1;
    a->active=1;
    a->x=x;
    a->y=y;
    if(sky>=0 && sky<6) a->sky=sky*27;
    else a->sky=-1;
    if(vis<7 && obs>0 && vis>0 && obs<4){
        a->obs=obs*27-27;
        a->vis=256-236*(vis-1)/6;
    } else a->vis=0;

    a->items[0]=frz;
    a->items[1]=snow;
    a->items[2]=rain;
    a->items[3]=tstorm;
    a->items[4]=svtstorm;
    for(i=0; i<5; i++){
        if(a->items[i]==0) continue;
        if(a->items[i]%P) a->items[i]=a->items[i]/P+1;
        else a->items[i]=a->items[i]/P;
        a->items[i]*=X;
        if(a->items[i]!=0) a->changed=1;
    }
    a->ac=0;
    a->moon=moon;
}

void DoAnimation(struct animation *a){
    int i;
    int top, h;
    
    /* Turned off? */
    if(!a->active) return;

    /* Any parameters changed? If yes, draw */
    if(a->changed) goto doit;

    /* Nothing changed, quit if not animating */
    if(!a->do_animate) return;

    /* We are animating. If it's the first frame of a cycle, draw it */
    a->ac++; if(a->ac>=M){ a->ac=0; goto doit; }

    /* Not the first frame, see if anything changed. If so, draw it */
    for(i=0; i<5; i++){
        if(a->ac>=a->items[i] && a->ac<=a->items[i]+X) goto doit;
    }

    /* Just draw the counter... */
    if(a->show_counter) goto do_counter;

    /* Nothing to draw, quit */
    return;

doit:
    if(a->min_pct!=a->old_pct){
        if(a->min_pct%P) a->pct=(a->min_pct/P+1)*X;
        else a->pct=(a->min_pct/P)*X;
        a->old_pct=a->min_pct;
    }
    a->changed=0;

    if(!a->do_animate) a->ac=a->pct;
    copyPixmapArea(124, 18, 26, 31, a->x, a->y);
    if(a->sky!=-1){
        copySunMoon(a->x, a->y, a->moon);
        combineWithTrans(a->sky, 64, 26, 25, a->x, a->y);
    }
    if(a->vis>0) combineWithOpacity(a->obs, 89, 26, 25, a->x, a->y, a->vis);
    for(i=0; i<5; i++){
        if(a->items[i]==0 || a->ac>=a->items[i]+X) continue;
        h=heights[i];
        top=h;
        if(a->ac<X){
            h=h*a->ac/X;
        } else if(a->ac>=a->items[i] && a->ac<a->items[i]+X){
            h-=h*(a->ac%X)/X;
            top=h;
        }
        combineWithTrans(i*27, 129-top, 26, h, a->x, a->y+31-top);
    }
    if(a->show_counter){{
        char foo[5];
do_counter:
        if(!a->do_animate){
            snprintf(foo, 5, "%d%%", a->min_pct);
        } else {
            for(i=0; i<100 && a->ac>((i+P-1)/P)*X; i++);
            snprintf(foo, 5, "%d%%", i);
        }
        i=GetStringWidth(foo);
        copyPixmapArea(124, 18, i+2, 7, a->x+(26-i-2)/2, a->y+12);
        DrawString(a->x+(26-i)/2, a->y+13, foo, 2);
    }}
}
