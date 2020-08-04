/*
 * wmxres.c
 * by Mychel Platyny
 */

/* 
 * Les includes
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/xpm.h>
#include <X11/Xlib.h>
#include <X11/extensions/xf86dga.h>
#include <X11/extensions/xf86vmode.h>
#include "../wmgeneral/wmgeneral.h"
#include "wmxres-master.xpm"
#include "wmxres-mask.xbm"

/*
 * Les Definitions
 */

int	button_state=-1;
int	isw;
int	res_count=-1;
int	res_selected;
int	res_i_active=-1;
char	res_active[10];
char	res_list[100][20];
XF86VidModeModeInfo **res_modelines; 
XEvent  Event;

/*
 * Les Fonctions
 */
void ButtonUp(int);
void ButtonDown(int);
void DrawLight(int);
void DrawResMode(int);
void GetXModes(void);
void ActiveXNewMode(void);
void GetXActiveMode(void);

/*
 * Yalla
 */
int main(int argc,char *argv[])
{
	openXwindow(argc, argv, wmxres_master_xpm, wmxres_mask_bits,
				wmxres_mask_width, wmxres_mask_height);

	AddMouseRegion(0,43,44,55,55);		/* Bouton d'activation */
	AddMouseRegion(1,19,44,31,55);		/* Bouton scan gauche  */
	AddMouseRegion(2,31,44,43,55);		/* Bouton scan droit   */

	GetXModes();
	GetXActiveMode();
	res_selected=res_i_active;
	DrawResMode(res_selected);
	DrawLight(1);

	while (1)
	{
		while (XPending(display))
		{
			XNextEvent(display, &Event);
			switch (Event.type) 
			{
				case Expose:
					/* On se fait beau */
					RedrawWindow();
				break;
				case DestroyNotify:
					/* Ciao */
					XCloseDisplay(display);
					exit(0);
				break;
				case ButtonPress:
					/* Bouton enfonce */
					isw = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
					switch (isw)
					{
						case 0:			/* Activation       */
							ButtonDown(0);
							ActiveXNewMode();
							DrawLight(1);
						break;
						case 1:			/* Je scan a gauche */
							ButtonDown(1);
							DrawResMode(res_selected);
							DrawLight(0);
						break;
						case 2:			/* Je scan a droite */
							ButtonDown(2);
							DrawResMode(res_selected);
							DrawLight(0);
						break;
					}
					if (res_selected==res_i_active) { DrawLight(1); }
					button_state = isw;
				break;
				case ButtonRelease:
					/* Bouton relache donc pas enfonce */
					switch (button_state) 
					{
						case 0:
							ButtonUp(0);
						break;
						case 1:
							ButtonUp(1);
						break;
						case 2:	
							ButtonUp(2);
						break;
					}
				break;
			}
		} 	
		usleep (200000);
	}
}

/*
 * Recuperation des resolutions X possible
 */
void GetXModes(void)
{
int	c;

	if (!(res_count > 0)) {
		XF86VidModeGetAllModeLines( display, XDefaultScreen(display), &res_count, &res_modelines);
	
		if (res_count < 2) {
			printf("Error : X must be configured with more than one mode.\n");
			exit(1);
		}
	}

	/* fix bounds on res_count -- Todd Troxell <ttroxell@debian.org */	
	if(res_count > 100) {
		res_count =100;
	}

	for(c=0; c < res_count ; c++) {
		sprintf(res_list[c], "%dx%d", res_modelines[c]->hdisplay, res_modelines[c]->vdisplay);
	}
}

/*
 * Recuperation de la resolution X active
 */
void GetXActiveMode(void)
{
XF86VidModeModeLine vm_modelines; 
int a, i;

	XF86VidModeGetModeLine( display, XDefaultScreen(display), &a, &vm_modelines);
	sprintf(res_active, "%dx%d", vm_modelines.hdisplay, vm_modelines.vdisplay);

	for(i=0; i < res_count; i++) {
		if (!strcmp(res_active, res_list[i])) {
			res_i_active=i;
		}
	}
}

/*
 * Activation d'une nouvelle resolution X
 */
void ActiveXNewMode()
{
	XF86VidModeSwitchToMode( display, XDefaultScreen(display), res_modelines[res_selected]);
   	XFlush(display);
	res_i_active=res_selected;
}

/*
 * Affichage d'une resolution
 */
void DrawResMode(int show_mode)
{
int	c, i, k;
char	*res_width, *res_height;
char	buf[1024];
char	*strtmp;

	strtmp=strdup(res_list[show_mode]);

	res_width=strtok(strtmp, "x");
	res_height=strtok((char *) NULL, "x"); 

	k=atoi(res_width);
	sprintf(buf, "%04i", k);
	k = 16;
	for (i=0; buf[i]; i++) {
        	c = buf[i]; 
		c -= '0';
		copyXPMArea(c * 6, 61, 6, 7, k-1, 9);
		k += 6;
	}

	k=atoi(res_height);
	sprintf(buf, "%04i", k);
	k = 16;
	for (i=0; buf[i]; i++) {
        	c = buf[i]; 
		c -= '0';
		copyXPMArea(c * 6, 61, 6, 7, k-1, 18);
		k += 6;
	}

	RedrawWindow();
}

/*
 * Allumage de la loupiote
 */
void DrawLight(int light_state)
{
	copyXPMArea (102, light_state? 35: 47, 14, 11, 1, 44);
	RedrawWindowXYWH(1, 44, 14, 11);
	RedrawWindow();
}



/*
 * Un on enfonce le bouton
 */
void ButtonDown(int button)
{
	switch (button)
	{
		case 0:
			copyXPMArea(79, 96, 12, 11, 43, 44);
			RedrawWindowXYWH(43, 44, 12, 11);
		break;
		case 1:
			copyXPMArea(55, 96, 12, 11, 19, 44);
			RedrawWindowXYWH(19, 44, 12, 11);
			res_selected--;
			if (res_selected < 0) { res_selected=res_count-1; }
		break;
		case 2:
			copyXPMArea(67, 96, 12, 11, 31, 44);
			RedrawWindowXYWH(31, 44, 12, 11);
			res_selected++;
			if (res_selected > res_count-1) { res_selected=0; }
		break;
	}
}

/*
 * Et deux on relache le bouton
 */ 
void ButtonUp(int button)
{
	switch (button)
	{
		case 0:
			copyXPMArea(79, 84, 12, 11, 43, 44);
			RedrawWindowXYWH(43, 44, 12, 11);
		break;
		case 1:
			copyXPMArea(55, 84, 12, 11, 19, 44);
			RedrawWindowXYWH(19, 44, 12, 11);
		break;
		case 2:
			copyXPMArea(67, 84, 12, 11, 31, 44);
			RedrawWindowXYWH(31, 44, 12, 11);
		break;
	}
}

