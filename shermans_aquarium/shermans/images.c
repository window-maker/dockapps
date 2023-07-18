
#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "aquarium.h"

extern int fullscreen, window_id;

/* Declare it here to avoid warnings */
GdkPixbuf *gai_load_image(const char *);

void load_image(char *fname, SA_Image *image, int frames)
{

    image->pixbuf = gai_load_image(fname);
    image->frames = frames;
    image->rev = NULL;
    image->width = gdk_pixbuf_get_width(image->pixbuf);
    image->full_height = gdk_pixbuf_get_height(image->pixbuf);

    image->rowstride = gdk_pixbuf_get_rowstride(image->pixbuf);
    image->height = (int)((float)image->full_height / (float)frames+0.5);
    image->image = gdk_pixbuf_get_pixels(image->pixbuf);
}



void load_image_n_scale(char *fname, SA_Image *image,
			int frames, int scale)
{

    int w, h, newh, neww,i;
    GdkPixbuf *tmpbuff, *workbuff1, *workbuff2;

    if(scale==100){
	load_image(fname,image,frames);
	return;
    }


    tmpbuff = gai_load_image(fname);
    image->frames = frames;
    image->rev = NULL;

    w = gdk_pixbuf_get_width(tmpbuff);
    h = gdk_pixbuf_get_height(tmpbuff);

    image->width = neww =(int)((((float)w * (float)scale) / 100.0)+0.5);
    image->height = newh = (int)((((float)h / (float)frames) * (float)scale) / 100.0 + 0.5);
    image->full_height = newh * frames; 

    image->pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE,8,
				   neww,
				   frames*newh);

    for(i=0;i<frames;i++){
	workbuff1 = gdk_pixbuf_new_subpixbuf(tmpbuff,0,i*h/frames,w,h/frames);
	workbuff2 = gdk_pixbuf_scale_simple(workbuff1, neww, newh, GDK_INTERP_BILINEAR);
	g_object_unref(workbuff1);
	gdk_pixbuf_copy_area(workbuff2,0,0,neww,newh,image->pixbuf,0,newh*i);
	g_object_unref(workbuff2);

    }

    


    image->image = gdk_pixbuf_get_pixels(image->pixbuf);
    image->rowstride = gdk_pixbuf_get_rowstride(image->pixbuf);

    g_object_unref(tmpbuff);

}
