/*
 * GAI - General Applet Interface Library
 * Copyright (C) 2003-2004 Jonas Aaberg <cja@gmx.net>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 *             Dedicated to Evelyn Reimann - Min ss sv gp af!!
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../config.h"
#include "gai.h"
#include "gai-private.h"

#ifdef GAI_WITH_GNOME
#include <panel-applet.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#endif


void 
gai_load_background (void)
{
    unsigned char *bg;
    int x,y, ypos;

    GAI_ENTER;  gai_is_init();


    if (GAI.use_default_background)
    {
	if(GAI.orig_background != NULL)
	    g_object_unref(GAI.orig_background);

	GAI.orig_background  = gdk_pixbuf_new (
	    GDK_COLORSPACE_RGB, TRUE, 8, 
	    GAI.default_width, GAI.default_height);

#      ifdef GAI_WITH_GNOME
	/* The gnome panel doesn't work with pixmap backgrounds
	    Could be a gtk+ bug, but I don't know.
       */
#      ifdef GNOME_24
	if ((GAI.applet_type == GAI_GNOME2 || GAI.applet_type == GAI_GNOME1) && 
	    GAI.bg_type == PANEL_PIXMAP_BACKGROUND)
	{
	    GAI_NOTE("pixmap bg!");
	    /* Copy what we got from the gnome panel as the image behind and
	       draw our original background*/
	    gdk_pixbuf_copy_area (GAI.bg_pixbuf,0,0,
				  gdk_pixbuf_get_width(GAI.bg_pixbuf),
				  gdk_pixbuf_get_height(GAI.bg_pixbuf),
				  GAI.orig_background,0,0);
	}
#      endif
#      endif

	bg = gdk_pixbuf_get_pixels(GAI.orig_background);
	
	for(y=0; y < GAI.default_height ; y++)
	{
	    ypos = y*gdk_pixbuf_get_rowstride (GAI.orig_background);
	    for (x=0; x < GAI.default_width*4 ; x += 4)
	    {
		
		if(GAI.background_has_border) {


		    if ((y < 3 || x < (3*4) || x > (4*(GAI.default_width-4)) ||
			 y > (GAI.default_height-4)) && GAI.applet_type == GAI_DOCKAPP)
		    {		
			bg[ypos+x+0]=0x0;
			bg[ypos+x+1]=0x0;
			bg[ypos+x+2]=0x0;
			bg[ypos+x+3]=0x0;
			continue;
		    }

		    if ((y == 3 && (x>4*2 && x<4*GAI.default_width-3*4)) || 
			(x == 3*4 &&(y>2 && y<GAI.default_height-3)))
		    {
			bg[ypos+x+0]=0x0;
			bg[ypos+x+1]=0x0;
			bg[ypos+x+2]=0x0;
			bg[ypos+x+3]=0xff;
			continue;
		    }

		    if ((y==(GAI.default_height-4) && (x>4*2 && x<4*GAI.default_width-3*4)) || 
			(x == (4*(GAI.default_width-4)) && (y>2 && y<GAI.default_height-3)))
		    {
			bg[ypos+x+0]=0xAB;
			bg[ypos+x+1]=0xBA;
			bg[ypos+x+2]=0xC6;
			bg[ypos+x+3]=0xFF;
			continue;
		    }
		}


#ifdef GAI_WITH_GNOME
		if (GAI.applet_type == GAI_GNOME1 ||
		    GAI.applet_type == GAI_GNOME2)
		{
		    
		    if (GAI.bg_type == PANEL_COLOR_BACKGROUND)
		    {

			bg[ypos+x+0]=(unsigned char)(GAI.bg_colour.red >>8);
			bg[ypos+x+1]=(unsigned char)(GAI.bg_colour.green >>8);
			bg[ypos+x+2]=(unsigned char)(GAI.bg_colour.blue >>8);
			bg[ypos+x+3]=0xFF;
			continue;
		    }
		    if (GAI.bg_type == PANEL_NO_BACKGROUND){
			if(GAI.use_default_background) {
			    bg[ypos+x+0]=0xdc;
			    bg[ypos+x+1]=0xda;
			    bg[ypos+x+2]=0xd5;
			    bg[ypos+x+3]=0xFF;
			}
			continue;
		    }

		    bg[ypos+x+0]=0xdc;
		    bg[ypos+x+1]=0xda;
		    bg[ypos+x+2]=0xd5; 
		    bg[ypos+x+3]=0xFF;

		    /* else { Do nothing when having a background } */
		} else {
		    bg[ypos+x+0]=0xdc;
		    bg[ypos+x+1]=0xda;
		    bg[ypos+x+2]=0xd5;
		    bg[ypos+x+3]=0xFF;
		}
#else
		bg[ypos+x+0]=0xdc;
		bg[ypos+x+1]=0xda;
		bg[ypos+x+2]=0xd5;
		bg[ypos+x+3]=0xFF;
#endif
	    }
	}

    }


    if(!GAI.use_default_background){

	if(GAI.orig_background != NULL)
	    g_object_unref(GAI.orig_background);

	GAI.orig_background = gdk_pixbuf_copy(GAI.file_background);

#   ifdef GAI_WITH_GNOME
	if (GAI.applet_type == GAI_GNOME2 || GAI.applet_type == GAI_GNOME1){

#      ifdef GNOME_24
	    if(GAI.bg_type == PANEL_PIXMAP_BACKGROUND){
		/* TO WRITE */
	    }
#      endif
	    if(GAI.bg_type == PANEL_COLOR_BACKGROUND){


		/* Makes the first work back */
		bg = gdk_pixbuf_get_pixels(GAI.orig_background);
		for(y=0;y<gdk_pixbuf_get_height(GAI.orig_background);y++){
		    ypos = y*gdk_pixbuf_get_rowstride(GAI.orig_background);
		    for(x=0;x<gdk_pixbuf_get_width(GAI.orig_background);x++){
			if(bg[ypos+x*4+3] != 0xff){
			    bg[ypos+x*4+0] = 
				(unsigned char)
				(((int)(256-bg[ypos+4*x+3]) * (int)(GAI.bg_colour.red>>8) + 
				  (int)bg[ypos+4*x+0] * (int)(256-bg[ypos+4*x+3])) >> 8);
			    bg[ypos+x*4+1] = 
				(unsigned char)
				(((int)(256-bg[ypos+4*x+3]) * (int)(GAI.bg_colour.green>>8) + 
				  (int)bg[ypos+4*x+1] * (int)(256-bg[ypos+4*x+3])) >> 8);
			    bg[ypos+x*4+2] = 
				(unsigned char)
				(((int)(256-bg[ypos+4*x+3]) * (int)(GAI.bg_colour.blue>>8) + 
				  (int)bg[ypos+4*x+2] * (int)(256-bg[ypos+4*x+3])) >> 8);
			    /* Don't bother Alpha channel - Looks better like that */
			    //bg[ypos+x*4+3]=0xff;
			}
		    }
		}
	    }
	}
#   endif
    }



   if(GAI.background !=NULL)
	g_object_unref(GAI.background);

    /* Makes the first work back */
    GAI.background = gdk_pixbuf_copy(GAI.orig_background);

   if(GAI.foreground !=NULL)
	g_object_unref(GAI.foreground);

    GAI.foreground = gdk_pixbuf_copy(GAI.background);

    if(GAI.init_done){
	if(GAI.gc != NULL)
	    g_object_unref(GAI.gc);
	GAI.gc = gdk_gc_new(GAI.window);
	gai_draw_update_bg();
    }

    GAI_LEAVE;
}

static
void gai_background_maybe_change_size(void)
{
    GdkGeometry geo;

    GAI_ENTER; gai_is_init();

    GAI.size_changing = 1;

    if(GAI.init_done){
#ifdef GAI_WITH_GNOME
	if(GAI.applet_type == GAI_GNOME1 || GAI.applet_type == GAI_GNOME2){
	    gai_gnome_change_size(NULL, -1 , NULL);
	    /* panel_applet_get_size(PANEL_APPLET(GAI.widget)) */
	} else {
#endif
	    geo.min_width = geo.max_width = gai_scale(GAI.default_width);
	    geo.min_height = geo.max_height = gai_scale(GAI.default_height);
	    gtk_window_set_geometry_hints(GTK_WINDOW(GAI.widget), GAI.widget, &geo, 
					  GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE);
	    gtk_widget_set_size_request(GAI.drawingarea,
					gai_scale(GAI.default_width), gai_scale(GAI.default_height));

	    gtk_widget_queue_resize(GAI.drawingarea);
	    gtk_widget_queue_resize(GAI.widget);

				
#ifdef GAI_WITH_GNOME					  
	}
#endif
    }
    GAI.size_changing = 0;
    GAI_LEAVE;
}

void 
gai_background_set(int width, int height, int max_size, int border)
{
    GAI_ENTER;  gai_is_init();

    GAI_D("width: %d height: %d\n",width, height);

    g_assert((width >0) && (width < 1280*2));
    g_assert((height >0) && (height < 1280*2));

    g_assert((max_size >=GAI_BACKGROUND_MAX_SIZE_IMAGE) && (max_size < 1280*2));
    g_assert((border == TRUE) || (border == FALSE));

    GAI.use_default_background = 1;
    GAI.background_has_border = border;



    if(max_size != GAI_BACKGROUND_MAX_SIZE_NONE) {
	if(max_size == GAI_BACKGROUND_MAX_SIZE_IMAGE){
	    GAI.max_size = GAI.default_height;
	} else
	    GAI.max_size = max_size;
    } else
	GAI.max_size = max_size;


    GAI.width = GAI.default_width = width;
    GAI.height = GAI.default_height = height;
    GAI.scale = 1.0;

    gai_background_maybe_change_size();


    gai_load_background();

    GAI_LEAVE;
}


void 
gai_background_from_gdkpixbuf(GdkPixbuf *pixbuf, int max_size)
{
    GdkEventConfigure e;
    int ow, oh;

    GAI_ENTER;	gai_is_init();


    g_assert(pixbuf != NULL);
    g_assert((max_size >= GAI_BACKGROUND_MAX_SIZE_IMAGE) && (max_size < 1280*2));


    GAI.use_default_background = 0;
    if(GAI.file_background!=NULL)
	g_object_unref(GAI.file_background);

    GAI.file_background = gdk_pixbuf_copy(pixbuf);
    oh = GAI.height;
    ow = GAI.width;

    GAI.width = GAI.default_width = gdk_pixbuf_get_width(GAI.file_background);
    GAI.height = GAI.default_height = gdk_pixbuf_get_height(GAI.file_background);
    GAI.scale = 1.0;

    if(max_size != GAI_BACKGROUND_MAX_SIZE_NONE) {
	if(max_size == GAI_BACKGROUND_MAX_SIZE_IMAGE){
	    GAI.max_size = GAI.default_height;
	} else
	    GAI.max_size = max_size;

    } else
	GAI.max_size = max_size;

    gai_background_maybe_change_size();
    
    if(GAI.transparent_bg && GAI.init_done && 
       (GAI.default_width != ow ||
	GAI.default_height != oh) && 
       (GAI.applet_type != GAI_GNOME1 && GAI.applet_type != GAI_GNOME2)){

	e.width = gdk_pixbuf_get_width(pixbuf);
	e.height = gdk_pixbuf_get_height(pixbuf);
	gdk_window_get_position(GAI.widget->window, &e.x, &e.y);
	//printf("root_win, x=%d y=%d w=%d h=%d\n", e.x, e.y, e.width, e.height); 
	gai_root_window_config(NULL, &e, (gpointer) -1);
    }

    gai_load_background();

    GAI_LEAVE;
}

void 
gai_background_from_xpm (const char **xpm_image, int max_size)
{
    GdkPixbuf *xpm_pixbuf;

    GAI_ENTER;  gai_is_init();

    g_assert(xpm_image !=NULL);

    xpm_pixbuf = gdk_pixbuf_new_from_xpm_data (xpm_image);
    gai_background_from_gdkpixbuf(xpm_pixbuf, max_size);
    g_object_unref(xpm_pixbuf);
	
    GAI_LEAVE;
}


void
gai_background_from_file(const char *file, int max_size)
{
    GdkPixbuf *file_pixbuf;

    GAI_ENTER;   gai_is_init();

    g_assert(file !=NULL);

    if(GAI.applet.image_path == NULL)
    {
	gai_display_error_quit(_("No image_path is set!\n"
			       "That is required before loading images.\n"));
	return;
    }

    file_pixbuf = gai_load_image (file);
    gai_background_from_gdkpixbuf(file_pixbuf, max_size);
    g_object_unref(file_pixbuf);

    GAI_LEAVE;

}


static void 
gai_general_draw(GdkPixbuf *target_buf, unsigned char *source, 
		 int sx, int sy, int sw, int sh,
		 int tx, int ty, 
		 int rs_source, int alpha_source, int ignore_alpha)
{

    int dw, di, dh, ds;
    int w, h;
    int th,tw, rs_target, alpha_target;
    int source_ypos, source_pos, target_ypos, target_pos;
    unsigned char *target;

    g_assert(target_buf != NULL);
    g_assert(source != NULL);
    g_assert((sx >=0) && (sy >=0) && (sw >=0) && (sh >=0) && (tx >=0) && (ty >=0));
    g_assert(rs_source>=0);
    g_assert((alpha_source == FALSE) || (alpha_source == TRUE));

    GAI.foreground_alpha = alpha_source;

    th = gdk_pixbuf_get_height(target_buf);
    tw = gdk_pixbuf_get_width(target_buf);
    rs_target = gdk_pixbuf_get_rowstride(target_buf);
    alpha_target = gdk_pixbuf_get_has_alpha(target_buf);

    target = gdk_pixbuf_get_pixels(target_buf);

    /* completely off the screen, don't bother drawing */
    if ((ty < -(sh)) || (ty > th) || (tx > tw) || (tx < -(sw)))
	return;

    /* do clipping for top side */
    ds = 0;
    if (ty < 0)
	ds = -(ty);

    /* do clipping for bottom side */
    dh = sh;
    if ((ty + sh) > th)
	dh = th - ty;

    /* do clipping for right side */
    dw = sw;
    if (tx > (tw - sw))
	dw = sw - (tx - (tw - sw));

    /* do clipping for left side */
    di = 0;
    if (tx < 0)
	di = -(tx);

    for (h = ds; h < dh; h++) 
    {
	/* offset to beginning of current row */
	target_ypos = (h + ty) * rs_target;
	source_ypos = (h + sy) * rs_source;

	for (w = di; w < dw; w++) 
	{
	    target_pos = target_ypos + (alpha_target+3)*(w + tx);
	    source_pos = source_ypos + (alpha_source+3)*(w + sx);

	    if (alpha_source && !ignore_alpha)
	    {
		if(source[source_pos+3]!=0)
		{
		    target[target_pos] = ( 
			(int)(256-source[source_pos+3]) * 
			(int)target[target_pos] + 
			(int)source[source_pos+3] * 
			(int)source[source_pos]
			) >> 8;

		    target[target_pos+1] = (
			(int)(256-source[source_pos+3]) *
			(int)target[target_pos+1] + 
			(int)source[source_pos+3] * 
			(int)source[source_pos+1]
			) >> 8;

		    target[target_pos+2] = (
			(int)(256-source[source_pos+3]) * 
			(int)target[target_pos+2] + 
			(int)source[source_pos+3] * 
			(int)source[source_pos+2]
			) >> 8;
		}
	    } else {
		target[target_pos+0] = source[source_pos+0];
		target[target_pos+1] = source[source_pos+1];
		target[target_pos+2] = source[source_pos+2];
		if(ignore_alpha && alpha_source && alpha_target)
		    target[target_pos+3] = source[source_pos+3];

	    }
	}
    }
}


/* All background draw commands ends up in this routine */
void 
gai_draw_bg (GdkPixbuf *src, 
	     int sx, int sy, int sw, int sh, int dx, int dy)
{
    GAI_ENTER;  gai_is_init();

    if(GAI.draw_bg_update_done){
	g_object_unref(GAI.background);
	GAI.background = gdk_pixbuf_copy(GAI.orig_background);
	GAI.draw_bg_update_done=0;
    }

    gai_general_draw(GAI.background,
		     gdk_pixbuf_get_pixels(src),
		     sx,sy,sw,sh,
		     dx,dy,
		     gdk_pixbuf_get_rowstride(src),
		     gdk_pixbuf_get_has_alpha(src),
		     TRUE);
    

    GAI_LEAVE;
}

static void 
draw_raw_bg (unsigned char *img, int x, int y, int w, int h, int rs, int alpha)
{
    GdkPixbuf *tmp_image;
    /* Applet takes care of img */

    g_assert(img !=NULL);
    g_assert((x>=0) && (y>=0) && (w>0) && (h>0) && (rs>0));
    g_assert((alpha == TRUE) || (alpha == FALSE));
    tmp_image = gdk_pixbuf_new_from_data 
	(img, GDK_COLORSPACE_RGB, alpha,8, w,h, rs, NULL, NULL);
    gai_draw_bg (tmp_image, 0,0, w,h, x,y);
    g_object_unref (tmp_image);

}

void 
gai_draw_raw_bg (unsigned char *img, int x, int y, int w, int h, int rs)
{
    GAI_ENTER;  gai_is_init();
    draw_raw_bg (img, x,y, w,h, rs, FALSE);
    GAI_LEAVE;
}

void 
gai_draw_raw_alpha_bg (unsigned char *img, int x, int y, int w, int h, int rs)
{
    GAI_ENTER;  gai_is_init();
    draw_raw_bg(img,x,y,w,h,rs,TRUE);
    GAI_LEAVE;
}


void 
gai_draw_update_bg (void)
{
    GdkPixmap *pixmap = NULL;
    GdkBitmap *mask = NULL;
    GdkPixbuf *image, *tmp_bg;
#  ifdef GAI_WITH_GNOME
    GdkPixbuf *image2;
    int x,y, ypos;
    unsigned char *bg;
#  endif

    GAI_ENTER;  gai_is_init();

    GAI.lock = 1;

    if (GAI.auto_scale && (GAI.height != GAI.default_height || GAI.width != GAI.default_width)){
	GAI_NOTE ("scaling");
	if(GAI.orient == GAI_VERTICAL && GAI.rotate)
	    image = gdk_pixbuf_scale_simple(GAI.background,
					    GAI.height,
					    GAI.width,
					    GDK_INTERP_BILINEAR);
	else
	    image = gdk_pixbuf_scale_simple(GAI.background,
					    GAI.width,
					    GAI.height,
					    GDK_INTERP_BILINEAR);
    }
    else 
	image = GAI.background;


#  ifdef GAI_WITH_GNOME
    if(GAI.applet_type == GAI_GNOME1 || GAI.applet_type == GAI_GNOME2){
	if(GAI.rotate && GAI.orient == GAI_VERTICAL && 
	   GAI.default_width != GAI.default_height)
	{
	    GAI_NOTE ("rotating");
	    image2 = gai_rotate(image);
	    if(image != GAI.background)
		g_object_unref (image);
	    image = image2;
	}

	if(GAI.bg_type == PANEL_COLOR_BACKGROUND && gdk_pixbuf_get_has_alpha(image)){
	    bg = gdk_pixbuf_get_pixels(image);
	    for(y=0;y<gdk_pixbuf_get_height(image);y++){
		ypos = gdk_pixbuf_get_rowstride(image)*y;
		for(x=0;x<gdk_pixbuf_get_width(image)*4;x+=4){
		    if(bg[ypos+x+3] != 0xff){
			bg[ypos+x+0]=(unsigned char)(GAI.bg_colour.red >>8);
			bg[ypos+x+1]=(unsigned char)(GAI.bg_colour.green >>8);
			bg[ypos+x+2]=(unsigned char)(GAI.bg_colour.blue >>8);
			bg[ypos+x+3]=0xff;
		    }
		}
	    }
	}

    }
#  endif


    if(GAI.behind_applet != NULL){
	tmp_bg = gdk_pixbuf_copy(GAI.behind_applet);
	gai_general_draw(tmp_bg, gdk_pixbuf_get_pixels(image),
			 0, 0,
			 gdk_pixbuf_get_width(image), gdk_pixbuf_get_height(image),
			 0, 0,
			 gdk_pixbuf_get_rowstride(image), gdk_pixbuf_get_has_alpha(image), FALSE);
	if (image != GAI.background)
	    g_object_unref (image);
	image = tmp_bg;
    }




    /* The Alpha level choosen here isn't maybe the best value */
    gdk_pixbuf_render_pixmap_and_mask(image, &pixmap, &mask, 0x80);


    /* Not understanding why gdk_window_shape_combine mask works better
       than gtk_widget_ on the gnome panel */
#ifdef GAI_WITH_GL
    if(!GAI.open_gl)
#endif
	gdk_window_clear(GAI.window);


#  ifdef GAI_WITH_GNOME
    if (GAI.applet_type == GAI_GNOME1 ||
	GAI.applet_type == GAI_GNOME2)
	gdk_window_shape_combine_mask (GAI.window, mask, 0,0);

    else 
#  endif
	gtk_widget_shape_combine_mask (GAI.widget, mask, 0,0);

    gdk_window_set_back_pixmap(GAI.window, pixmap, FALSE);


    if(GAI.icon_window != NULL){
	gdk_window_shape_combine_mask (GAI.icon_window, mask, 0,0);
	gdk_window_set_back_pixmap(GAI.icon_window, pixmap, FALSE);
    }

    /* Make sure the changes are updated - Child widgets will get be updated too.*/
    gtk_widget_queue_draw_area(GAI.widget, 0,0,GAI.width, GAI.height);

    

    gdk_window_process_all_updates();
    gdk_flush();


    if (image != GAI.background)
	g_object_unref (image);

    GAI.draw_bg_update_done = 1;

    if (pixmap != NULL)
	g_object_unref (pixmap);

    if (mask != NULL)
	g_object_unref (mask);

    GAI.lock = 0;
    GAI_LEAVE;
}

void 
gai_draw (GdkPixbuf *src, int sx, int sy, int sw, int sh, int dx, int dy)
{
    GAI_ENTER;  gai_is_init();

    gai_general_draw(GAI.foreground,
		     gdk_pixbuf_get_pixels (src),
		     sx,sy,sw,sh,
		     dx,dy,
		     gdk_pixbuf_get_rowstride (src),
		     gdk_pixbuf_get_has_alpha (src), 
		     FALSE);
    GAI_LEAVE;
}


void 
gai_draw_raw (unsigned char *img, int x, int y, int w, int h, int rs)
{
    GAI_ENTER;  gai_is_init();

    gai_general_draw(GAI.foreground,
		     img,
		     0,0,w,h,
		     x,y,
		     rs,
		     FALSE, FALSE);

    GAI_LEAVE;
}

void 
gai_draw_raw_alpha (unsigned char *img, int x, int y, int w, int h, int rs)
{
    GAI_ENTER;  gai_is_init();
    
    gai_general_draw(GAI.foreground,
		     img,
		     0,0,w,h,
		     x,y,
		     rs,
		     TRUE, FALSE);
    GAI_LEAVE;
}


void 
gai_draw_update (void)
{
    GdkPixbuf *image;
#  ifdef GAI_WITH_GNOME
    GdkPixbuf *image2;
#  endif
    GAI_ENTER; gai_is_init();

    GAI.lock = 1;

    //#  ifdef GAI_WITH_GNOME
    if (GAI.auto_scale && (GAI.height != GAI.default_height || GAI.width != GAI.default_width))
    {
	if(GAI.orient == GAI_VERTICAL && GAI.rotate)
	    image = gdk_pixbuf_scale_simple(GAI.foreground,
					    GAI.height,
					    GAI.width,
					    GDK_INTERP_BILINEAR);
	else

	    image = gdk_pixbuf_scale_simple(GAI.foreground,
					    GAI.width,
					    GAI.height,
					    GDK_INTERP_BILINEAR);
    }
    else
	//#  endif
	image = GAI.foreground;

#  ifdef GAI_WITH_GNOME
    if (GAI.rotate && GAI.orient==GAI_VERTICAL && 
	GAI.default_width != GAI.default_height)
    {
	image2 = gai_rotate (image);
	if (image != GAI.foreground)
	    g_object_unref (image);
	image = image2;
    }
#  endif

    if (gdk_pixbuf_get_has_alpha (image))
	gdk_draw_rgb_32_image (GAI.window,GAI.gc,
			       0,0, gdk_pixbuf_get_width (image),
			       gdk_pixbuf_get_height (image),
			       GDK_RGB_DITHER_NONE,
			       gdk_pixbuf_get_pixels (image),
			       gdk_pixbuf_get_rowstride (image));
    else
      gdk_draw_rgb_image(GAI.window,GAI.gc,
			 0,0, gdk_pixbuf_get_width (image),
			 gdk_pixbuf_get_height (image),
			 GDK_RGB_DITHER_NONE,
			 gdk_pixbuf_get_pixels (image),
			 gdk_pixbuf_get_rowstride (image));
    
    gdk_flush();
    if (image != GAI.foreground)
	g_object_unref (image);


    if (GAI.foreground_alpha)
    {
	g_object_unref (GAI.foreground);
	GAI.foreground = gdk_pixbuf_copy (GAI.background);
    }

    GAI.lock = 0;
    GAI_LEAVE;
}


