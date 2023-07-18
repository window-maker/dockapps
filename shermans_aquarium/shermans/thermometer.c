
#include "status.h"
#include "aquarium.h"
#include "draw.h"
#include "thermometer.h"



static Thermometer_settings thermometer_settings;
static SA_Image thermometer_1, thermometer_2;
static AquariumData *ad;
static int status_list[STATUSES];

Thermometer_settings *thermometer_get_settings_ptr(void)
{
    return &thermometer_settings;
}

int thermometer_get_real_status(int status)
{
    int i,j=0;
    for(i=0;i<STATUSES;i++){
	if(status_list[i]){ 
	    if(j==status)
		return i;
	    j++;
	}
    }
    return 0;
}

void thermometer_gen_list(void)
{
    status_list[STATUS_OFF] = TRUE;

#ifdef STATUS_HAVE_FAN1
    status_list[STATUS_FAN1] = TRUE;
#else
    status_list[STATUS_FAN1] = FALSE;
#endif 

#ifdef STATUS_HAVE_FAN2
    status_list[STATUS_FAN2] = TRUE;
#else
    status_list[STATUS_FAN2] = FALSE;
#endif 

#ifdef STATUS_HAVE_TEMP1
    status_list[STATUS_TEMP1] = TRUE;
#else
    status_list[STATUS_TEMP1] = FALSE;
#endif 
#ifdef STATUS_HAVE_TEMP2
    status_list[STATUS_TEMP2] = TRUE;
#else
    status_list[STATUS_TEMP2] = FALSE;
#endif 
#ifdef STATUS_HAVE_SWAP
    status_list[STATUS_SWAP] = TRUE;
#else
    status_list[STATUS_SWAP] = FALSE;
#endif 
#ifdef STATUS_HAVE_DISC
    status_list[STATUS_DISC] = TRUE;
#else
    status_list[STATUS_DISC] = FALSE;
#endif 

#ifdef STATUS_HAVE_CPU
    status_list[STATUS_CPU] = TRUE;
#else
    status_list[STATUS_CPU] = FALSE;
#endif 

#ifdef STATUS_HAVE_MEM
    status_list[STATUS_MEM] = TRUE;
#else
    status_list[STATUS_MEM] = FALSE;
#endif 


#ifdef STATUS_HAVE_NET_ETH0
    status_list[STATUS_NET_ETH0_RECV] = TRUE;
    status_list[STATUS_NET_ETH0_SEND] = TRUE;
    status_list[STATUS_NET_ETH0_BOTH] = TRUE;
#else
    status_list[STATUS_NET_ETH0_RECV] = FALSE;
    status_list[STATUS_NET_ETH0_SEND] = FALSE;
    status_list[STATUS_NET_ETH0_BOTH] = FALSE;
#endif 


#ifdef STATUS_HAVE_NET_ETH1
    status_list[STATUS_NET_ETH1_RECV] = TRUE;
    status_list[STATUS_NET_ETH1_SEND] = TRUE;
    status_list[STATUS_NET_ETH1_BOTH] = TRUE;
#else
    status_list[STATUS_NET_ETH1_RECV] = FALSE;
    status_list[STATUS_NET_ETH1_SEND] = FALSE;
    status_list[STATUS_NET_ETH1_BOTH] = FALSE;
#endif 


#ifdef STATUS_HAVE_NET_PPP0
    status_list[STATUS_NET_PPP0_RECV] = TRUE;
    status_list[STATUS_NET_PPP0_SEND] = TRUE;
    status_list[STATUS_NET_PPP0_BOTH] = TRUE;
#else
    status_list[STATUS_NET_PPP0_RECV] = FALSE;
    status_list[STATUS_NET_PPP0_SEND] = FALSE;
    status_list[STATUS_NET_PPP0_BOTH] = FALSE;
#endif 


#ifdef STATUS_HAVE_NET_LO
    status_list[STATUS_NET_LO_RECV] = TRUE;
    status_list[STATUS_NET_LO_SEND] = TRUE;
    status_list[STATUS_NET_LO_BOTH] = TRUE;
#else
    status_list[STATUS_NET_LO_RECV] = FALSE;
    status_list[STATUS_NET_LO_SEND] = FALSE;
    status_list[STATUS_NET_LO_BOTH] = FALSE;
#endif 


}

void thermometer_exit(void)
{
    if(thermometer_1.pixbuf!=NULL){
	g_object_unref(thermometer_1.pixbuf);
	thermometer_1.pixbuf = NULL;
    }

    if(thermometer_2.pixbuf!=NULL){
	g_object_unref(thermometer_2.pixbuf);
	thermometer_2.pixbuf = NULL;
    }
}

void thermometer_change_colour(SA_Image *image, unsigned char r, unsigned char g, unsigned char b)
{
    int x,y,ypos, alpha;
    
    alpha = gdk_pixbuf_get_has_alpha(image->pixbuf);
    for(y=0;y<image->height;y++){
	ypos = y*image->rowstride;

	for(x=0;x<image->width*(3+alpha);x+=(3+alpha)){
		if(image->image[ypos+x+0] == 0xff){
		    image->image[ypos+x+0] = r;
		    image->image[ypos+x+1] = g;
		    image->image[ypos+x+2] = b;
		}

		if(image->image[ypos+x+0] == 0xbf){

		    if(r>0x20)
			image->image[ypos+x+0] = r-0x20;
		    else
		        image->image[ypos+x+0] = 0x00;

		    if(g>0x20)
			image->image[ypos+x+1] = g-0x20;
		    else
		        image->image[ypos+x+1] = 0x00;

		    if(b>0x20)
			image->image[ypos+x+2] = b-0x20;
		    else
		        image->image[ypos+x+2] = 0x00;
		}

	}
    }
       

}

void thermometer_init(void)
{
    ad = aquarium_get_settings_ptr();

    thermometer_gen_list();

    if(thermometer_1.image!=NULL || thermometer_2.image!=NULL)
	thermometer_exit();


    if(thermometer_settings.messure1 != thermometer_get_real_status(STATUS_OFF)){
	load_image("thermometer.png", &thermometer_1,1);
	thermometer_change_colour(&thermometer_1,
				  thermometer_settings.c1.r,thermometer_settings.c1.g,thermometer_settings.c1.b);

    }
    if(thermometer_settings.messure2 != thermometer_get_real_status(STATUS_OFF)){
	load_image("thermometer.png", &thermometer_2,1);
	thermometer_change_colour(&thermometer_2,
				  thermometer_settings.c2.r,thermometer_settings.c2.g,thermometer_settings.c2.b);


    }
}

int thermometer_status_level(int messure, int roof, char *mount_p)
{
    //(30 * status_level / 100)
    int status_type, level, real_roof;
    status_type = thermometer_get_real_status(messure);

    switch(status_type){

    case STATUS_OFF:
	level = 0;
	real_roof = 100;
	break;
    case STATUS_FAN1:
	level = status_sensors(SENSORS_FAN1);
	real_roof = roof;
	break;
    case STATUS_FAN2:
	level = status_sensors(SENSORS_FAN2);
	real_roof = roof;
	break;
    case STATUS_TEMP1:
	level = status_sensors(SENSORS_TEMP1);
	real_roof = roof;
	break;
    case STATUS_TEMP2:
	level = status_sensors(SENSORS_TEMP2);
	real_roof = roof;
	break;
    case STATUS_SWAP:
	level = status_swap();
	real_roof = 100;
	break;
    case STATUS_DISC:
	level = status_disc(mount_p);
	real_roof = 100;
	break;	
    case STATUS_CPU:
	level = status_cpu();
	real_roof = 100;
	break;
    case STATUS_MEM:
	level = status_mem();
	real_roof = 100;
	break;

    case STATUS_NET_ETH0_RECV:
	level = status_net(NET_ETH0, NET_RECV);
	real_roof = roof;
	break;
    case STATUS_NET_ETH0_SEND:
	level = status_net(NET_ETH0, NET_SENT);
	real_roof = roof;
	break;
    case STATUS_NET_ETH0_BOTH:
	level = (status_net(NET_ETH0, NET_RECV)  + status_net(NET_ETH0, NET_SENT))/2;
	real_roof = roof;
	break;
    case STATUS_NET_ETH1_RECV:
	level = status_net(NET_ETH1, NET_RECV);
	real_roof = roof;
	break;
    case STATUS_NET_ETH1_SEND:
	level = status_net(NET_ETH1, NET_SENT);
	real_roof = roof;
	break;
    case STATUS_NET_ETH1_BOTH:
	level = (status_net(NET_ETH1, NET_RECV)  + status_net(NET_ETH1, NET_SENT))/2;
	real_roof = roof;
	break;
    case STATUS_NET_PPP0_RECV:
	level = status_net(NET_PPP0, NET_RECV);
	real_roof = roof;
	break;
    case STATUS_NET_PPP0_SEND:
	level = status_net(NET_PPP0, NET_SENT);
	real_roof = roof;
	break;
    case STATUS_NET_PPP0_BOTH:
	level = (status_net(NET_PPP0, NET_RECV)  + status_net(NET_PPP0, NET_SENT))/2;
	real_roof = roof;
	break;
    case STATUS_NET_LO_RECV:
	level = status_net(NET_LO, NET_RECV);
	real_roof = roof;
	break;
    case STATUS_NET_LO_SEND:
	level = status_net(NET_LO, NET_SENT);
	real_roof = roof;
	break;
    case STATUS_NET_LO_BOTH:
	level = (status_net(NET_LO, NET_RECV)  + status_net(NET_LO, NET_SENT))/2;
	real_roof = roof;
	break;


    default:
	level = 30;
	real_roof = 100;
    }

    if(level > real_roof)
	return 0;

    if(level <0 || real_roof <=0)
	return 30;
    else 
	return (int)(30.0 - 30.0*(float)level / (float)real_roof);

}

void thermometer_core(SA_Image *thermometer, int mode,int draw, int vert, int horz, int split, 
		      unsigned char r, unsigned char g, unsigned char b, unsigned char alpha,
		      unsigned char r_s, unsigned char g_s, unsigned char b_s, unsigned char alpha_s,
		      int messure, int messure_s, int roof, int roof_s, char *mount_point, char *mount_point_s)
{

    int x=0, y=0, colour, status_type;
    static int status_level[2] = {0, 0}, count = 0;


    if(mode != draw)
	return;

    status_type = thermometer_get_real_status(messure);
    if(status_type == STATUS_OFF)
	return;

    if(vert == TOP)
	y = 0;
    if(vert == CENTER)
	y = ad->ymax/2 - thermometer->height/2;
    if(vert == BOTTOM)
	y = ad->ymax - thermometer->height;

    if(horz == LEFT)
	x = 0;
    if(horz == CENTER)
	x = ad->xmax/2 - thermometer->width/2;
    if(horz == RIGHT)
	x = ad->xmax - thermometer->width - 1;

    draw_pic_alpha(thermometer->image, thermometer->width,
		   thermometer->height, x, y, 0, 0x80);

    colour = (((int)r) <<16) + (((int)g) <<8) + (int)b;


    /* Status level is between 0 and 30, 0 is max */
    if(count == 0)
	status_level[0] = thermometer_status_level(messure, roof, mount_point);

    anti_line(x+3, y + 2 + status_level[0], x+3,
	      y+thermometer->height-3, 
	      1, colour, 0);

    anti_line(x+2, y+thermometer->height-4, x+2,
	      y + thermometer->height-5, 1, colour, 0);


    if(split){
	colour = (((int)r_s) <<16) + (((int)g_s) <<8) + (int)b_s;

	if(count == 0)
	    status_level[1] = thermometer_status_level(messure_s, roof_s, mount_point_s);

	anti_line(x+4, y+thermometer->height-3, x+4,
		  y + 2 + status_level[1], 1, colour, 0);

    } else {
	anti_line(x+4, y+thermometer->height-3, x+4,
		  y + 2 + status_level[0], 1, colour, 0);
    }

    anti_line(x+5, y+thermometer->height-4, x+5,
	      y + thermometer->height-5, 1, colour, 0);

    /* Update about once a second */
    if(count == 0)
	count = 20;
    else
	count --;

}

void thermometer_update(int mode)
{

    thermometer_core(&thermometer_1, mode, thermometer_settings.draw1,
		     thermometer_settings.vert1,thermometer_settings.horz1,
		     thermometer_settings.split1,
		     thermometer_settings.c1.r, thermometer_settings.c1.g,thermometer_settings.c1.b,
		     thermometer_settings.c1.alpha,
		     thermometer_settings.c1_s.r,thermometer_settings.c1_s.g,thermometer_settings.c1_s.b,
		     thermometer_settings.c1_s.alpha,
		     thermometer_settings.messure1, thermometer_settings.messure1_s, 
		     thermometer_settings.roof1, thermometer_settings.roof1_s,
		     thermometer_settings.mount_point1,
		     thermometer_settings.mount_point1_s);


    thermometer_core(&thermometer_2, mode, thermometer_settings.draw2,
		     thermometer_settings.vert2,thermometer_settings.horz2,
		     thermometer_settings.split2,
		     thermometer_settings.c2.r, thermometer_settings.c2.g,thermometer_settings.c2.b,
		     thermometer_settings.c2.alpha,
		     thermometer_settings.c2_s.r,thermometer_settings.c2_s.g,thermometer_settings.c2_s.b,
		     thermometer_settings.c2_s.alpha,
		     thermometer_settings.messure2, thermometer_settings.messure2_s, 
		     thermometer_settings.roof2, thermometer_settings.roof2_s,
		     thermometer_settings.mount_point2,
		     thermometer_settings.mount_point2_s);

}




