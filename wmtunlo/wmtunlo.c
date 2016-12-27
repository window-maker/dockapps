
/*
 * wmtunlo
 *
 * Copyright (C) 2001 pasp
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <X11/X.h>
#include <X11/xpm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include "master.h"
#include "mask.h"

#include "docklib.h"

void wmtunlo_read_prefs(void);
void wmtunlo_write_prefs(void);

void   ButtonPressEvent(XButtonEvent *);
int    GotFirstClick1, GotDoubleClick1;
int    GotFirstClick2, GotDoubleClick2;
int    GotFirstClick3, GotDoubleClick3;
int    DblClkDelay;
int    HasExecute;
char   ClickCommand1[MAX_PATH];
char   ClickCommand2[MAX_PATH];
char   ClickCommand3[MAX_PATH];

int		tex_block_w, tex_block_h, shade_switch, persp_switch;
int		t_color_1, t_color_2;
float	t_persp, t_shade_level;
float	t_mov_counter, t_rot_counter;
float	t_mov_step, t_rot_step;
float	t_col_add_R, t_col_add_G, t_col_add_B;
float	t_col_mul_R, t_col_mul_G, t_col_mul_B;

int main(int argc, char *argv[])
{

typedef struct {
    Display			*display;
    int				screen;
    Visual			*visual;
    int				depth;
    Colormap		cmap;
    int				format;
    int				bitmap_pad;
    int				Color[256];
    unsigned char	R[256];
    unsigned char	G[256];
    unsigned char	B[256];
} DisplayInfo;

XImage *xim;
XEvent event;
XColor xColor, xColors[256];
DisplayInfo Info;
int i, j, shade, k, l, p, q, result, col;
float xz, yz, px, py;
unsigned char *Image, *Texture, *Tunnel_map;


    /* Read preferences */
	wmtunlo_read_prefs(); 

    /* Open window */
    openXwindow(argc, argv, dock_master, dock_mask_bits, dock_mask_width, dock_mask_height);

    /* Get Display parameters */
    Info.display = display;
    Info.screen  = DefaultScreen(display);
    Info.visual  = DefaultVisual(display, Info.screen);
    Info.depth   = DefaultDepth(display, Info.screen);
    Info.cmap    = DefaultColormap(display, 0);

    /* Initialize Color Table */

		for (i=0; i<256; ++i) {
			col = t_col_mul_R * (float)i + t_col_add_R;
			if(col<0) col = 0;
 			if(col>255) col = 255;
			Info.R[i] = col;

			col = t_col_mul_G * (float)i + t_col_add_G;
			if(col<0) col = 0;
 			if(col>255) col = 255;
			Info.G[i] = col;

			col = t_col_mul_B * (float)i + t_col_add_B;
			if(col<0) col = 0;
 			if(col>255) col = 255;
			Info.B[i] = col;
		}

	/* Create an XImage with null data. Then allocate space for data. */
    Info.format = ZPixmap;
    if (Info.depth == 8){

        Info.bitmap_pad = 8;

		/* Set a private colormap */
		Info.cmap = XCreateColormap(Info.display, RootWindow(Info.display, Info.screen), Info.visual, AllocAll);
	        for (i=0; i<256; ++i){
			    Info.Color[i] = i;
			    xColors[i].pixel = i;
	    		xColors[i].red   = (unsigned short)Info.R[i] << 8;
			    xColors[i].green = (unsigned short)Info.G[i] << 8;
			    xColors[i].blue  = (unsigned short)Info.B[i] << 8;
			    xColors[i].flags = DoRed | DoGreen | DoBlue;
    	    }
		XStoreColors(Info.display, Info.cmap, xColors, 256);
		XSetWindowColormap(Info.display, win, Info.cmap);

    } else if (Info.depth > 8) {

		/* Allocate Colors */
        for (i=0; i<256; ++i){
		    xColor.red   = (unsigned short)Info.R[i] << 8;
		    xColor.green = (unsigned short)Info.G[i] << 8;
		    xColor.blue  = (unsigned short)Info.B[i] << 8;
		    xColor.flags = DoRed | DoGreen | DoBlue;
		    XAllocColor(Info.display, Info.cmap, &xColor);
		    Info.Color[i] = xColor.pixel;
        }
        Info.bitmap_pad = 32;

    } else {
		fprintf(stderr, "Need at least 8-bit display!\n");
		exit(-1);
    }

    xim = XCreateImage(Info.display, Info.visual, Info.depth, Info.format, 0, (char *)0, 54, 54, Info.bitmap_pad, 0);
    xim->data = (char *)malloc(xim->bytes_per_line * 54 );

    /* Allocate memory for image data */
    Image    = (unsigned char *)malloc(sizeof(unsigned char)*54*54);

	/* Generate tunnel map */
    Tunnel_map = (unsigned char *)malloc(sizeof(unsigned char)*54*54*4);

		for (k=0, yz=-54/2; yz<54/2; yz++)
		   for (xz=-54/2; xz<54/2; xz++) {

		        py = sqrt(xz*xz + yz*yz);
		        shade= py * t_shade_level;
				if(shade>255) shade=255;
		        if(persp_switch) py = t_persp/py;
		        px = atan2(yz,xz) * 128.0 / M_PI;
		        result = (int)py * 256 + (int)px;

		        Tunnel_map[k++]= result&0xff; 
				Tunnel_map[k++]= result>>8;
		        Tunnel_map[k++]= shade&0xff;
		    }

	/* Generate texture */
    Texture = (unsigned char *)malloc(sizeof(unsigned char)*256*256);

	for(i=0,q=1;i<256;i+=tex_block_h, q*=-1)
		for(j=0,p=q;j<256;j+=tex_block_w, p*=-1) 
	       	for(l=0;l<tex_block_h;l++)
		    	for(k=0;k<tex_block_w;k++)
					if((j+k < 256) && (i+l < 256)) {
						if(p>0)
							Texture[(i+l)*256+j+k] = t_color_1;
						else 
							Texture[(i+l)*256+j+k] = t_color_2;
						}

	/* Animation. */

	t_mov_counter = 0.0;
	t_rot_counter = 0.0;

    while(1) {

		/* Process any pending X events. */

        while(XPending(display)){
            XNextEvent(display, &event);
            switch(event.type){
                case Expose:
                        RedrawWindow();
                        break;
                case EnterNotify:
						XSetInputFocus(display, iconwin, RevertToNone, CurrentTime);
						if (Info.depth == 8) XInstallColormap(display, Info.cmap);
                        break;
                case LeaveNotify:
						XSetInputFocus(display, None, RevertToNone, CurrentTime);
						if (Info.depth == 8) XUninstallColormap(display, Info.cmap);
                        break;
				case ButtonPress:
						ButtonPressEvent(&event.xbutton);
						break;
				case ButtonRelease:
						break;
    		}
        }

		/* Draw tunnel. */

		for(i=0;i<54;i++)
			for(j=0;j<54;j++) {

				if(shade_switch)
		            *(Image + 54*i + j) = (Tunnel_map[(54*i+j)*3 + 2] * Texture[ \
					((Tunnel_map[(54*i+j)*3 + 1] + (int)t_mov_counter) & 0xff)*256 + \
					Tunnel_map[(54*i+j)*3 + 0] - (int)t_rot_counter]) >> 8;
				else
		            *(Image + 54*i + j) = Texture[ \
					((Tunnel_map[(54*i+j)*3 + 1] + (int)t_mov_counter) & 0xff)*256 + \
					Tunnel_map[(54*i+j)*3 + 0] - (int)t_rot_counter];
			}

		/* Move tunnel. */

		t_mov_counter += t_mov_step;
		if(t_mov_counter > 256.0) t_mov_counter = 0.0;
		if(t_mov_counter < 0.0) t_mov_counter = 256.0;
		t_rot_counter += t_rot_step;
		if(t_rot_counter > 256.0) t_rot_counter = 0.0;
		if(t_rot_counter < 0.0) t_rot_counter = 256.0;

		/* Paste up image. */
	    for ( i=0; i<54; ++i )
			for ( j=0; j<54; ++j ) {
			    XPutPixel(xim, i, j,  Info.Color[*(Image + j*54 + i)]);
			    XFlush(display);
			}

	    XPutImage(display, wmgen.pixmap, NormalGC, xim, 0, 0, 5, 5, 54, 54);

		/* Make changes visible */
	    RedrawWindow();

		/* Wait for next update  */
		usleep(10000L);
    }

    free(xim->data);
    XDestroyImage(xim);

	free(Image);
	free(Tunnel_map);
	free(Texture);
}

/*
 *  This routine handles button presses.
 *
 *   Double click on
 *              Mouse Button 1: Execute the command defined in the -e command-line option.
 *              Mouse Button 2: No action assigned.
 *              Mouse Button 3: No action assigned.
 *
 *
 */
void ButtonPressEvent(XButtonEvent *xev){

    DblClkDelay = 0;
    if ((xev->button == Button1) && (xev->type == ButtonPress)){
        if (GotFirstClick1) GotDoubleClick1 = 1;
        else GotFirstClick1 = 1;
    } else if ((xev->button == Button2) && (xev->type == ButtonPress)){
        if (GotFirstClick2) GotDoubleClick2 = 1;
        else GotFirstClick2 = 1;
    } else if ((xev->button == Button3) && (xev->type == ButtonPress)){
        if (GotFirstClick3) GotDoubleClick3 = 1;
        else GotFirstClick3 = 1;
    }

    /*
     *  We got a double click on Mouse Button1 (i.e. the left one)
     */
    if (GotDoubleClick1) {
        GotFirstClick1 = 0;
        GotDoubleClick1 = 0;
        system(ClickCommand1);
    }

    /*
     *  We got a double click on Mouse Button2 (i.e. the left one)
     */
    if (GotDoubleClick2) {
        GotFirstClick2 = 0;
        GotDoubleClick2 = 0;
        system(ClickCommand2);
    }

    /*
     *  We got a double click on Mouse Button3 (i.e. the left one)
     */
    if (GotDoubleClick3) {
        GotFirstClick3 = 0;
        GotDoubleClick3 = 0;
        system(ClickCommand3);
    }

   return;

}

/*----------------------------------------------------------------------*/
/* Write preferences */

void wmtunlo_write_prefs(void) 
{
	if (p_prefs_openfile (p_getfilename_config (".clay", "wmtunlorc"), P_WRITE)) {

		p_prefs_put_int("shade_switch", shade_switch);
		p_prefs_put_int("persp_switch", persp_switch);
		p_prefs_put_lf ();
		p_prefs_put_float("t_shade_level", t_shade_level);
		p_prefs_put_float("t_persp", t_persp);
		p_prefs_put_float("t_mov_step", t_mov_step);
		p_prefs_put_float("t_rot_step", t_rot_step);
		p_prefs_put_lf ();
		p_prefs_put_int("tex_block_w", tex_block_w);
		p_prefs_put_int("tex_block_h", tex_block_h);
		p_prefs_put_lf ();
		p_prefs_put_float("t_col_add_R", t_col_add_R);
		p_prefs_put_float("t_col_add_G", t_col_add_G);
		p_prefs_put_float("t_col_add_B", t_col_add_B);
		p_prefs_put_lf ();
		p_prefs_put_float("t_col_mul_R", t_col_mul_R);
		p_prefs_put_float("t_col_mul_G", t_col_mul_G);
		p_prefs_put_float("t_col_mul_B", t_col_mul_B);
		p_prefs_put_lf ();
		p_prefs_put_int("t_color_1", t_color_1);
		p_prefs_put_int("t_color_2", t_color_2);
		p_prefs_put_lf ();
		p_prefs_put_string("command1", ClickCommand1);
		p_prefs_put_string("command2", ClickCommand2);
		p_prefs_put_string("command3", ClickCommand3);

	}
        
	p_prefs_closefile ();
}

/*----------------------------------------------------------------------*/
/* Read preferences */

void wmtunlo_read_prefs(void) 
{
	if (p_prefs_openfile (p_getfilename_config(".clay", "wmtunlorc"), P_READ)) {

		shade_switch = p_prefs_get_int("shade_switch");
		persp_switch = p_prefs_get_int("persp_switch");

		t_shade_level = p_prefs_get_float("t_shade_level");
		t_persp = p_prefs_get_float("t_persp");
		t_mov_step = p_prefs_get_float("t_mov_step");
		t_rot_step = p_prefs_get_float("t_rot_step");

		tex_block_w = p_prefs_get_int("tex_block_w");
		tex_block_h = p_prefs_get_int("tex_block_h");

		t_col_add_R = p_prefs_get_float("t_col_add_R");
		t_col_add_G = p_prefs_get_float("t_col_add_G");
		t_col_add_B = p_prefs_get_float("t_col_add_B");

		t_col_mul_R = p_prefs_get_float("t_col_mul_R");
		t_col_mul_G = p_prefs_get_float("t_col_mul_G");
		t_col_mul_B = p_prefs_get_float("t_col_mul_B");

		t_color_1 = p_prefs_get_int("t_color_1") % 255;
		t_color_2 = p_prefs_get_int("t_color_2") % 255;

		strcpy(ClickCommand1, p_prefs_get_string ("command1"));
		strcpy(ClickCommand2, p_prefs_get_string ("command2"));
		strcpy(ClickCommand3, p_prefs_get_string ("command3"));
    
		p_prefs_closefile ();
        
	} else {

		shade_switch = 1;
		persp_switch = 1;

		t_shade_level = 5.2;
		t_persp = 1000.0;
		t_mov_step = 0.6;
		t_rot_step = 0.3;

		tex_block_w = 32;
		tex_block_h = 32;

		t_col_add_R = 24.0;
		t_col_add_G = -12.0;
		t_col_add_B = 55.0;

		t_col_mul_R = 1.0;
		t_col_mul_G = 1.0;
		t_col_mul_B = 0.0;

		t_color_1 = 20;
		t_color_2 = 128;

		strcpy(ClickCommand1, "xlock");
		strcpy(ClickCommand2, "xlock -mode thornbird");
		strcpy(ClickCommand3, "xlock -mode blank");

		wmtunlo_write_prefs ();

	}
}

/*----------------------------------------------------------------------*/

