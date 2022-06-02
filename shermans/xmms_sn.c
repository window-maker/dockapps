
/* Displays the currently loaded/played song by XMMS */

#include <gai/gai.h>
#include <xmmsctrl.h>
#include <string.h>

#include "aquarium.h"
#include "draw.h"
#include "xmms_sn.h"


static Xmms_sn_settings xmms_sn_settings;
static SA_Image xmms_font;
static AquariumData *ad;

Xmms_sn_settings *xmms_sn_get_settings_ptr(void)
{
    return &xmms_sn_settings;
}

void xmms_sn_exit(void)
{
    if(xmms_font.pixbuf!=NULL)
	g_object_unref(xmms_font.pixbuf);

    memset(&xmms_font,0,sizeof(SA_Image));

}
void xmms_sn_init(void)
{
    ad = aquarium_get_settings_ptr();


    if(xmms_font.pixbuf!=NULL) 
	xmms_sn_exit();

    load_image("small_letters.png",&xmms_font,1);

    /* Some changes cause this image is horizontal ordered */

    xmms_font.frames = XMMS_SN_NUM_LETTERS;
    xmms_font.width = xmms_font.width/xmms_font.frames;

    change_colour_to(xmms_sn_settings.c.r,
		     xmms_sn_settings.c.g,
		     xmms_sn_settings.c.b,
		     xmms_font.image,
		     xmms_font.pixbuf, FALSE);

}

void xmms_sn_update(int beforeorafter)
{
    int i,x=0,y=0,a;
    static float start_display = 0.0;

    int session_id = 0;
    char *xmms_str, *disp_str;

    if(beforeorafter!=xmms_sn_settings.draw)
	return;

    if(!xmms_sn_settings.on)
	return;

    if(!xmms_remote_is_running(session_id)) 
	return;

    xmms_str = xmms_remote_get_playlist_title(session_id, 
					     xmms_remote_get_playlist_pos(session_id));
    if(xmms_str==NULL) 
	return;

    disp_str = g_strdup_printf(" %d. %s (%d:%.2d) *** ",
			       xmms_remote_get_playlist_pos(session_id)+1,
			       xmms_str,
			       xmms_remote_get_playlist_time(session_id, 
							     xmms_remote_get_playlist_pos(session_id))/(60*1000),
			       xmms_remote_get_playlist_time(session_id, 
							     xmms_remote_get_playlist_pos(session_id))/1000 %60);
    g_free(xmms_str);



    if(xmms_sn_settings.direction == XMMS_SN_HORIZONTAL) {

	if(xmms_sn_settings.fb == XMMS_SN_BACKWARDS){
	    if(((int)start_display) > (strlen(disp_str)-2)*xmms_font.width) start_display = 0.0;
	}
	else{
	    if(start_display <0.0) start_display = (strlen(disp_str)-1)*xmms_font.width;
	}

	switch(xmms_sn_settings.vert){
	case TOP:
	    y=2;
	    break;
	case CENTER:
	    y=ad->ymax/2-xmms_font.height/2;
	    break;
	case BOTTOM:
	    y=ad->ymax-xmms_font.height-2;
	    break;
	}
	x = -(((int)start_display) % xmms_font.width);

	for(i=0;i<=(ad->xmax/xmms_font.width);i++){
	    if((strlen(disp_str)-1)==i) break;
	    a=((int)start_display)/xmms_font.width+i;
	    if(a>=strlen(disp_str)) a-=strlen(disp_str);
	    if(a<0) a=strlen(disp_str)-1;
	    if(disp_str[a]!=' ' && disp_str[a]<='z')
		draw_image_alpha_h(x,y,((int)disp_str[a]-(int)'!'),xmms_sn_settings.c.alpha,&xmms_font);
	    x+=xmms_font.width;	
	}
    }
    else {

	if(xmms_sn_settings.fb == XMMS_SN_BACKWARDS){
	    if(((int)start_display) > (strlen(disp_str)-2)*xmms_font.height) start_display = 0.0;
	}
	else{
	    if(start_display <0.0) start_display = (strlen(disp_str)-1)*xmms_font.height;
	}

	switch(xmms_sn_settings.horz){
	case LEFT:
	    x=2;
	    break;
	case CENTER:
	    x=ad->xmax/2-xmms_font.width/2;
	    break;
	case RIGHT:
	    x=ad->ymax-xmms_font.width-2;
	    break;
	}

	y = -(((int)start_display) % xmms_font.height);

	for(i=0;i<=(ad->ymax/xmms_font.height);i++){
	    if((strlen(disp_str)-1)==i) break;
	    a=((int)start_display)/xmms_font.height+i;
	    if(a>=strlen(disp_str)) a-=strlen(disp_str);
	    if(a<0) a=strlen(disp_str)-1;

	    if(disp_str[a]!=' ' && disp_str[a]<='z')
		draw_image_alpha_h(x,y,((int)disp_str[a]-(int)'!'),xmms_sn_settings.c.alpha,&xmms_font);
	    y+=xmms_font.height;	
	}
    }

    if(xmms_sn_settings.fb == XMMS_SN_BACKWARDS)
	start_display+=0.5*(float)xmms_sn_settings.speed/100.0;
    else
	start_display-=0.5*(float)xmms_sn_settings.speed/100.0;

    g_free(disp_str);

}
