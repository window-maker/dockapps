/*
 * wmxres.c
 * by Mychel Platyny
 */

/* 
 * Les includes
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/extensions/xf86vmode.h>
#include <libdockapp/dockapp.h>
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
char	res_active[12];
char	res_list[100][20];
XF86VidModeModeInfo **res_modelines; 
XEvent  Event;
Pixmap pixmap;

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

void pressActivate(int x, int y, DARect rect, void *data);
void pressLeft(int x, int y, DARect rect, void *data);
void pressRight(int x, int y, DARect rect, void *data);
void buttonPress(int button, int state, int x, int y);

void releaseActivate(int x, int y, DARect rect, void *data);
void releaseLeft(int x, int y, DARect rect, void *data);
void releaseRight(int x, int y, DARect rect, void *data);
void buttonRelease(int button, int state, int x, int y);

/*
 * Yalla
 */
int main(int argc,char *argv[])
{
	DACallbacks eventCallbacks = {NULL, buttonPress, buttonRelease,
				      NULL, NULL, NULL, NULL};
	Pixmap mask;
	unsigned short width, height;

	DAParseArguments(argc, argv, NULL, 0,
			 "Window Maker dockapp to select your display mode",
			 PACKAGE_STRING);

	DAInitialize(NULL, PACKAGE_NAME, 56, 56, argc, argv);
	DASetCallbacks(&eventCallbacks);

	DAMakePixmapFromData(wmxres_master_xpm, &pixmap, NULL, &width, &height);
	mask = DAMakeShapeFromData(wmxres_mask_bits,
				   wmxres_mask_width, wmxres_mask_height);
	DASetPixmap(pixmap);
	DASetShape(mask);

	GetXModes();
	GetXActiveMode();
	res_selected=res_i_active;
	DrawResMode(res_selected);
	DrawLight(1);

	DASetTimeout(200);
	DAShow();
	DAEventLoop();
}

void pressActivate(int x, int y, DARect rect, void *data)
{
	ButtonDown(0);
	ActiveXNewMode();
	DrawLight(1);
}

void pressLeft(int x, int y, DARect rect, void *data)
{
	ButtonDown(1);
	DrawResMode(res_selected);
	DrawLight(res_selected == res_i_active ? 1 : 0);
}

void pressRight(int x, int y, DARect rect, void *data)
{
	ButtonDown(2);
	DrawResMode(res_selected);
	DrawLight(res_selected == res_i_active ? 1 : 0);
}

void buttonPress(int button, int state, int x, int y)
{
	DAActionRect pressRects[] = {
		{{43, 44, 12, 12}, pressActivate},
		{{19, 44, 12, 12}, pressLeft},
		{{31, 44, 12, 12}, pressRight}
	};

	DAProcessActionRects(x, y, pressRects, 3, NULL);
}

void releaseActivate(int x, int y, DARect rect, void *data)
{
	ButtonUp(0);
}

void releaseLeft(int x, int y, DARect rect, void *data)
{
	ButtonUp(1);
}

void releaseRight(int x, int y, DARect rect, void *data)
{
	ButtonUp(2);
}

void buttonRelease(int button, int state, int x, int y)
{
	DAActionRect releaseRects[] = {
		{{43, 44, 12, 12}, releaseActivate},
		{{19, 44, 12, 12}, releaseLeft},
		{{31, 44, 12, 12}, releaseRight}
	};

	DAProcessActionRects(x, y, releaseRects, 3, NULL);
}

/*
 * Recuperation des resolutions X possible
 */
void GetXModes(void)
{
int	c;

	if (!(res_count > 0)) {
		XF86VidModeGetAllModeLines(DADisplay, XDefaultScreen(DADisplay), &res_count, &res_modelines);
	
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

	XF86VidModeGetModeLine(DADisplay, XDefaultScreen(DADisplay), &a, &vm_modelines);
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
	XF86VidModeSwitchToMode(DADisplay, XDefaultScreen(DADisplay), res_modelines[res_selected]);
	XFlush(DADisplay);
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
		XCopyArea(DADisplay, pixmap, pixmap, DAGC,
			  c * 6, 61, 6, 7, k-1, 9);
		k += 6;
	}

	k=atoi(res_height);
	sprintf(buf, "%04i", k);
	k = 16;
	for (i=0; buf[i]; i++) {
        	c = buf[i]; 
		c -= '0';
		XCopyArea(DADisplay, pixmap, pixmap, DAGC,
			  c * 6, 61, 6, 7, k-1, 18);
		k += 6;
	}

	DASetPixmap(pixmap);
}

/*
 * Allumage de la loupiote
 */
void DrawLight(int light_state)
{
	XCopyArea(DADisplay, pixmap, pixmap, DAGC,
		  102, light_state? 35: 47, 14, 11, 1, 44);
	DASetPixmap(pixmap);
}



/*
 * Un on enfonce le bouton
 */
void ButtonDown(int button)
{
	switch (button)
	{
		case 0:
			XCopyArea(DADisplay, pixmap, pixmap, DAGC,
				  79, 96, 12, 11, 43, 44);
		break;
		case 1:
			XCopyArea(DADisplay, pixmap, pixmap, DAGC,
				  55, 96, 12, 11, 19, 44);
			res_selected--;
			if (res_selected < 0) { res_selected=res_count-1; }
		break;
		case 2:
			XCopyArea(DADisplay, pixmap, pixmap, DAGC,
				  67, 96, 12, 11, 31, 44);
			res_selected++;
			if (res_selected > res_count-1) { res_selected=0; }
		break;
	}
	DASetPixmap(pixmap);
}

/*
 * Et deux on relache le bouton
 */ 
void ButtonUp(int button)
{
	switch (button)
	{
		case 0:
			XCopyArea(DADisplay, pixmap, pixmap, DAGC,
				  79, 84, 12, 11, 43, 44);
		break;
		case 1:
			XCopyArea(DADisplay, pixmap, pixmap, DAGC,
				  55, 84, 12, 11, 19, 44);
		break;
		case 2:
			XCopyArea(DADisplay, pixmap, pixmap, DAGC,
				  67, 84, 12, 11, 31, 44);
		break;
	}
	DASetPixmap(pixmap);
}

