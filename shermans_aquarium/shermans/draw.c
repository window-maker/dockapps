#include <stdlib.h>
#include "draw.h"
#include "aquarium.h"


/* This is a special version of draw_image() that draws parts of a 
   horizontal stored multi image */

void draw_image_alpha_h(int x, int y, int idx, int alpha,SA_Image *image)
{

    AquariumData *ad;
    /* bounding box of the clipped sprite */
    int dw, di, dh, ds;
			
    /* loop counters */
    int w, h, pos, fidx;
    int ypos,pic_pos;



    ad = aquarium_get_settings_ptr();

    /* completely off the screen, don't bother drawing */
    if ((y < (-image->height)) || (y > (ad->ymax)) || (x > (ad->xmax)) || (x < -(image->width)))
	return;


    /* do clipping for top side */
    ds = 0;
    if (y < 0)
	ds = -(y);

    /* do clipping for bottom side */
    dh = image->height;
    if ((y + image->height) > ad->ymax)
	dh = ad->ymax - y;
    
    /* do clipping for right side */
    dw = image->width;
    if ((x + image->width) > ad->xmax)
	dw = image->width - ((x + image->width) - ad->xmax);

    /* do clipping for left side */
    di = 0;
    if (x < 0)
	di = -(x);

    fidx = idx*image->width*4;




    for (h = ds; h < dh; h++) {
	/* offset to beginning of current row */
	ypos = (h + y) * ad->xmax * 3;
	for (w = di; w < dw; w++) {
	    pic_pos = h * image->rowstride + w * 4 + fidx;
	    if (image->image[pic_pos + 3] != 0) {
		pos = ypos + w * 3 + x * 3;
		ad->rgb[pos] = ((256-alpha) * (int) ad->rgb[pos]
				+ alpha * (int) image->image[pic_pos]) >> 8;

		ad->rgb[pos + 1] = ((256-alpha) * (int) ad->rgb[pos + 1]
				    +  alpha * (int) image->image[pic_pos +
									 1]) >> 8;

		ad->rgb[pos + 2] = ((256-alpha) * (int) ad->rgb[pos + 2]
				    + alpha * (int) image->image[pic_pos +
									 2]) >> 8;

	    }
	}
    }
}




static void draw_image_base(unsigned char *buff, int x, int y, int idx, int rev, SA_Image *image)
{

    AquariumData *ad;
    /* bounding box of the clipped sprite */
    int dw, di, dh, ds;
			

    int w, h, pos, fidx;
    int q, ypos, pic_pos;

    ad = aquarium_get_settings_ptr();

    /* completely off the screen, don't bother drawing */
    if ((y < (-image->height)) || (y > (ad->ymax)) || (x > (ad->xmax)) || (x < -(image->width)))
	return;


    /* do clipping for top side */
    ds = 0;
    if (y < 0)
	ds = -(y);

    /* do clipping for bottom side */
    dh = image->height;
    if ((y + image->height) > ad->ymax)
	dh = ad->ymax - y;
    
    /* do clipping for right side */
    dw = image->width;
    if ((x + image->width) > ad->xmax)
	dw = image->width - ((x + image->width) - ad->xmax);

    /* do clipping for left side */
    di = 0;
    if (x < 0)
	di = -(x);

    
    fidx = (int) ((float)idx * (float)image->full_height / (float)image->frames) * 
	gdk_pixbuf_get_rowstride(image->pixbuf);

	/*image->width * image->height * 4 * idx;*/

    if (rev) {

	/* The fish is moving in different direction */
	for (h = ds; h < dh; h++) {
	    /* offset to beginning of current row */
	    ypos = (h + y) * ad->xmax * 3;
	    q = 0;
	    for (w = dw; w > di; w--) {
		pic_pos =
		    h * image->width * 4 + (q + (image->width - dw)) * 4 + fidx;
		if (image->image[pic_pos + 3] != 0) {
		    pos = ypos + w * 3 + x * 3 - 3;
		    buff[pos] = image->image[pic_pos];
		    buff[pos + 1] = image->image[pic_pos + 1];
		    buff[pos + 2] = image->image[pic_pos + 2];
		}
		q++;
	    }


	}
    } else {

	for (h = ds; h < dh; h++) {
	    /* offset to beginning of current row */
	    ypos = (h + y) * ad->xmax * 3;
	    for (w = di; w < dw; w++) {
		pic_pos = h * image->width * 4 + w * 4 + fidx;
		pos = ypos + w * 3 + x * 3;
		if (image->image[pic_pos + 3] != 0) {
		    buff[pos] = image->image[pic_pos];
		    buff[pos + 1] = image->image[pic_pos + 1];
		    buff[pos + 2] = image->image[pic_pos + 2];
		}
	    }

	}

    }
}

void draw_image(int x, int y, int idx, int rev, SA_Image *image)
{
    AquariumData *ad;
    ad = aquarium_get_settings_ptr();

    draw_image_base(ad->rgb,x,y,idx,rev,image);
}

void draw_image_bg(int x, int y, int idx, int rev, SA_Image *image)
{
    AquariumData *ad;
    ad = aquarium_get_settings_ptr();

    draw_image_base(ad->bgr,x,y,idx,rev,image);
}




/* draw a fish into ad.rgb with alpha-blend */
void
draw_pic_alpha(unsigned char *buff, int width, int height, int x,
	       int y, int frame, int alpha)
{
    /* bounding box of the clipped sprite */
    int dw, di, dh, ds;
    int w, h, pos, fidx;
    int ypos, pic_pos;

    AquariumData *ad;

    ad = aquarium_get_settings_ptr();

    /* completely off the screen, don't bother drawing */
    if ((y < -(height)) || (y > ad->ymax) || (x > ad->xmax) || (x < -(width)))
	return;

    /* do clipping for top side */
    ds = 0;
    if (y < 0)
	ds = -(y);

    /* do clipping for bottom side */
    dh = height;
    if ((y + height) > ad->ymax)
	dh = ad->ymax - y;

    /* do clipping for right side */
    dw = width;
    if (x > (ad->xmax - width))
	dw = width - (x - (ad->xmax - width));

    /* do clipping for left side */
    di = 0;
    if (x < 0)
	di = -(x);

    fidx = width * 4 * height * frame;

    for (h = ds; h < dh; h++) {

	/* offset to beginning of current row */


	ypos = (h + y) * ad->xmax;
	for (w = di; w < dw; w++) {
	    pic_pos = h * width * 4 + w * 4 + fidx;
	    if (buff[pic_pos + 3] != 0) {
		pos = (ypos + w + x) * 3;
		ad->rgb[pos] = ((256-alpha) * (int) ad->rgb[pos] + alpha * (int) buff[pic_pos]) >> 8;
		ad->rgb[pos + 1] = ((256-alpha) * (int) ad->rgb[pos + 1]
				    + alpha * (int) buff[pic_pos +
								 1]) >> 8;
		ad->rgb[pos + 2] = ((256-alpha) * (int) ad->rgb[pos + 2]
				    + alpha * (int) buff[pic_pos +
								 2]) >> 8;

	    }
	}
    }
}



/* draw antialiased line from (x1, y1) to (x2, y2), with width linewidth
 * colour is an int like 0xRRGGBB */
void anti_line(int x1, int y1, int x2, int y2, int linewidth, int colour, int shaded)
{

    int dx,dy;
    int error, sign, tmp;
    float ipix;
    int step = linewidth;

    char af[]={0x45,0x76,0x65,0x6C,0x79,0x79,0x79,0x6E,0x65,0x6E,0x2C,0x6D,
	       0x69,0x6E,0x61,0x6C,0x70,0x66,0x6C,0x69,0x63,0x6B,0x61,0x5C,
	       0x21};

    dx = abs(x1 - x2);
    dy = abs(y1 - y2);

    if (dx >= dy) {
	if (x1 > x2) {
	    tmp = x1;
	    x1 = x2;
	    x2 = tmp;
	    tmp = y1;
	    y1 = y2;
	    y2 = tmp;
	}
	error = dx / 2;
	if (y2 > y1)
	    sign = step;
	else
	    sign = -step;

	putpixel(x1, y1, 1, linewidth, colour);

	while (x1 < x2) {
	    if ((error -= dy) < 0) {
		y1 += sign;
		error += dx;
	    }
	    x1 += step;
	    ipix = (float) error / dx;

	    if (sign == step)
		ipix = 1 - ipix;

	    if(shaded){
		putpixel(x1, y1 - step, (1 - ipix), linewidth, colour);
		putpixel(x1, y1 + step, ipix, linewidth, colour);
	    }
	    putpixel(x1, y1, 1, linewidth, colour);

	}
	putpixel(x2, y2, 1, linewidth, colour);
    } else {
	if (y1 > y2) {
	    tmp = x1;
	    x1 = x2;
	    x2 = tmp;
	    tmp = y1;
	    y1 = y2;
	    y2 = tmp;
	}
	error = dy / 2;

	if (x2 > x1)
	    sign = step;
	else
	    sign = -step;

	putpixel(x1, y1, 1, linewidth, colour);

	while (y1 < y2) {
	    if ((error -= dx) < 0) {
		x1 += sign;
		error += dy;
	    }
	    y1 += step;
	    ipix = (float) error / dy;

	    if (sign == step)
		ipix = 1 - ipix;
	    if(shaded){
		putpixel(x1 - step, y1, (1 - ipix), linewidth, colour);
		putpixel(x1 + step, y1, ipix, linewidth, colour);
	    }

	    putpixel(x1, y1, 1, linewidth, colour);
	}
	putpixel(x2, y2, 1, linewidth, colour);
    }
    if(step >= linewidth)
	af[0]=0x65;
}


void putpixel(int x, int y, float i, int linewidth, int colour)
{
  
    AquariumData *ad;
    int dx, dy;
    unsigned char r,g,b;
    int pos;

    ad = aquarium_get_settings_ptr();


    pos = (y * ad->xmax * 3) + x * 3;


    r = ((colour >> 16) & 0xff) * i + (ad->rgb[pos]) * (1 - i);
    g = ((colour >> 8) & 0xff) * i + (ad->rgb[pos + 1]) * (1 - i);
    b = (colour & 0xff) * i + (ad->rgb[pos + 2]) * (1 - i);


    if (linewidth == 1) {
	ad->rgb[pos] = r;
	ad->rgb[pos + 1] = g;
	ad->rgb[pos + 2] = b;
    } else {
	for (dx = x; dx < x + linewidth; dx++) {
	    for (dy = y; dy < y + linewidth; dy++) {
		pos = (dy * ad->xmax * 3) + dx * 3;
		ad->rgb[pos] = r;
		ad->rgb[pos + 1] = g;
		ad->rgb[pos + 2] = b;
	    }
	}
    }
}



/* Change the colour of an image */
void change_colour_to(int r, int g,int b,unsigned char *image,GdkPixbuf *pixbuf, int feat)
{
    int i,j, rows,alpha;
    int h,w, pos,ypos;

    rows = gdk_pixbuf_get_rowstride(pixbuf);
    h = gdk_pixbuf_get_height(pixbuf);
    w = gdk_pixbuf_get_width(pixbuf);

    for(i=0;i<h;i++){
	ypos = rows*i;
	for(j=0;j<w;j++){
	    pos = ypos+j*4;

	    if(feat){
		alpha = (unsigned int)image[pos+3];
		image[pos+0]=(unsigned char) ((r*alpha)>>8);
		image[pos+1]=(unsigned char) ((g*alpha)>>8);
		image[pos+2]=(unsigned char) ((b*alpha)>>8);
	    } else {
 
		image[pos+0]=(unsigned char) (r);
		image[pos+1]=(unsigned char) (g);
		image[pos+2]=(unsigned char) (b);
	    }

	}

    }


}
