/*

wmressel.c
X11 resolution selector (for Window Maker)
by Sébastien "Slix" Liénard <slix@gcu-squad.org>, GCU Squad (http://gcu-squad.org)
based on gvidm and wmtimer
and licensed through the GNU General Public License.

Read the COPYING file for the complete GNU license.

*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/utsname.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/xf86vmode.h>

#define V_PHSYNC	0x001
#define V_NHSYNC	0x002
#define V_PVSYNC	0x004
#define V_NVSYNC	0x008
#define V_INTERLACE	0x010
#define V_DBLSCAN	0x020
#define V_CSYNC 	0x040
#define V_PCSYNC	0x080
#define V_NCSYNC	0x100

#ifdef HAVE_LIBXINERAMA
#include <X11/extensions/Xinerama.h>
#endif

#include <gtk/gtk.h>

#include "wmgeneral/wmgeneral.h"
#include "wmgeneral/misc.h"

#include "wmressel-master.xpm"
#include "wmressel-mask.xbm"

#define MY_EMAIL "slix@gcu-squad.org"
#define GCU_HOME "http://gcu-squad.org/"
#define WMRESSEL_VERSION "0.8"

typedef struct {
	int screen;
	int mode;
} MENU_CHOICE;

void usage(void);
void printVersion(void);
void BlitString(char *name, int x, int y);
void BlitNum(int num, int x, int y);
void wmressel_routine(int, char **, int);
void MenuEvent (GtkWidget *, gpointer);
void on_deactivate(void);
void create_popup(int);
GtkWidget *create_screen_submenu (int);
char *create_submenu_label(XF86VidModeModeInfo *, XF86VidModeModeLine, int);
void update_res_display(int);
void show_screen_number(int);
int get_screen_count(void);

char *ProgName;
int show_refresh_rate=0;
int show_doublescan=0;
int show_only_selected=0;

int main(int argc, char *argv[])
{

	int i, selected_screen=0;
	char *endp=NULL;

	/* Parse Command Line */

	ProgName = argv[0];
	if (strlen(ProgName) >= 5)
		ProgName += (strlen(ProgName) - 5);

	for (i=1; i<argc; i++) {
	char *arg = argv[i];

		if (*arg=='-') {
			switch (arg[1]) {
			case '-' :
				/* wmgeneral.c has been modified to allow this trick: openXwindow and gtk_init */
				/* use same option (--display) for display setting */
				if (strcmp(arg+1, "-display")) {
					usage();
					exit(1);
				}
				break;
			case 'd' :
				show_doublescan=1;
				break;
			case 'r' :
				show_refresh_rate=1;
				break;
			case 'o' :
				show_only_selected=1;
				break;
			case 's' :
				if (++i==argc) {
				    usage();
				    exit(1);
				  };
				selected_screen = strtol(argv[i], &endp, 10);
				if (*endp) {
				    usage();
				    exit(1);
				  };
				break;
			case 'v' :
				printVersion();
				exit(0);
				break;
			default:
				usage();
				exit(0);
				break;
			}
		}
	}

	wmressel_routine(argc, argv, selected_screen);
	return(0);
}

/*******************
* wmressel_routine *
*******************/

void wmressel_routine(int argc, char **argv, int selected_screen)
{
	int i, but_stat=-1;
	XEvent Event;

	int j=0, show_number=0, screen_count;

	openXwindow(argc, argv, wmressel_master_xpm, wmressel_mask_bits, wmressel_mask_width, wmressel_mask_height);

	screen_count = get_screen_count();
	if (selected_screen > screen_count-1) {
        printf("\nScreen number %i is not valid !!!\n", selected_screen);
		usage();
		exit(1);
    }

    if (screen_count > 1) {
	    show_screen_number(selected_screen);
	    show_number=1;
    } else {
	    update_res_display(selected_screen);
    }

	AddMouseRegion( 0,  5,  5, 55, 55);

	gtk_init (&argc, &argv);

	while (1) {
		/* X Events */
		while (XPending(display)) {
			XNextEvent(display, &Event);
			switch (Event.type) {
				case Expose:
					RedrawWindow();
					break;
				case DestroyNotify:
					XCloseDisplay(display);
					exit(0);
					break;
				case ButtonPress:
					i = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
					but_stat = i;
					break;
				case ButtonRelease:
					i = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
					if (but_stat == i && but_stat >= 0) {

						screen_count=get_screen_count();

						if(Event.xbutton.button==1) {
                            /* Left button pressed, popup menu */
							create_popup(selected_screen);
		    			    gtk_main();
							update_res_display(selected_screen);

						} else if (Event.xbutton.button==3 && screen_count>1) {
						    /* Right button pressed, view screen number */
                            show_screen_number(selected_screen);
							show_number=1;

						} else if (Event.xbutton.button==4 && screen_count>1) {
                            /* Mouse wheel up, select next screen */
							if(selected_screen==get_screen_count()-1) {
								selected_screen=0;
							} else {
                                selected_screen++;
							}
							show_screen_number(selected_screen);
							show_number=1;

						} else if (Event.xbutton.button==5 && screen_count>1) {
                            /* Mouse wheel down, select previous screen */
							if(selected_screen==0) {
								selected_screen=get_screen_count()-1;
							} else {
							    selected_screen--;
                            }
							show_screen_number(selected_screen);
							show_number=1;
						}
	    			}
    				but_stat = -1;
    				break;
			}
		}
		if(show_number) {
			j++;
			if (j==10) {
				update_res_display(selected_screen);
				show_number=0;
				j=0;
			}
		}
    	usleep(100000L);
	}
}

/********************
* Create popup menu *
********************/
void create_popup(int selected_screen)
{

	GtkWidget *menu_ptr;

	int i;
	char label_str[14];
	GtkWidget *submenu_ptr, *menuitem_ptr;
	int screen_count = get_screen_count();

	if (screen_count > 1 && !show_only_selected) {
		menu_ptr = gtk_menu_new();
		for (i=0; i<screen_count; i++) {
			submenu_ptr = create_screen_submenu(0);
			sprintf(label_str, "Screen %i",i);
			menuitem_ptr = gtk_menu_item_new_with_label(label_str);
			gtk_menu_item_set_submenu(GTK_MENU_ITEM (menuitem_ptr), submenu_ptr);
			gtk_menu_append(GTK_MENU (menu_ptr), menuitem_ptr);
			gtk_widget_show(menuitem_ptr);
			gtk_widget_show(submenu_ptr);
        }
    } else {
		menu_ptr = create_screen_submenu(selected_screen);
	}

	gtk_signal_connect (GTK_OBJECT (menu_ptr), "deactivate", GTK_SIGNAL_FUNC (on_deactivate), NULL);
	gtk_menu_popup(GTK_MENU(menu_ptr), NULL, NULL, NULL, NULL, 0, 0);
}

/************************************************
* Procede GTK menu choice and switch resolution *
************************************************/
void MenuEvent (GtkWidget *widget, gpointer menu_choice) {
	XF86VidModeModeInfo **vm_modelines;
	int vm_count;

	XF86VidModeGetAllModeLines(display, ((MENU_CHOICE *)menu_choice)->screen, &vm_count, &vm_modelines);
	XF86VidModeSwitchToMode(display, ((MENU_CHOICE *)menu_choice)->screen, vm_modelines[((MENU_CHOICE *)menu_choice)->mode]);
	XFlush(display);
	free(vm_modelines);
}

/***************************
* Return number of screens *
***************************/
int get_screen_count()
{
    int screen_count;
#ifdef HAVE_LIBXINERAMA
    int i;
	int screen_exists[1024];
	XineramaScreenInfo *screen;

	if (XineramaIsActive(display)) {
		screen = XineramaQueryScreens(display, screen_exists);
		for (i=0; i==screen[i].screen_number; i++){}
		screen_count=i;
		XFree(screen);
	} else {
#endif
		screen_count = XScreenCount(display);
#ifdef HAVE_LIBXINERAMA
	}
#endif
	return(screen_count);
}

void on_deactivate(void) {
	gtk_main_quit();
}

/*********************************
* Create a label for a menu item *
**********************************/
char *create_submenu_label(XF86VidModeModeInfo *modeline, XF86VidModeModeLine current, int dotclock)
{
	char *label_str;
	char def[3], res[12];
	char rr[8]="";
	char ds[3]="";

	label_str = malloc(sizeof(def)+sizeof(res)+sizeof(rr)+sizeof(ds)-3);

	if ((current.hdisplay==modeline->hdisplay) && (current.vdisplay==modeline->vdisplay) && (dotclock==modeline->dotclock)) {
			sprintf(def, "* ");
	} else {
			sprintf(def, "  ");
	}

	sprintf(res, "%ix%i", modeline->hdisplay, modeline->vdisplay);

	if (show_refresh_rate) {
		if ((modeline->flags) & V_DBLSCAN) {
			sprintf(rr, "@%iHz", modeline->dotclock*500/(modeline->htotal*modeline->vtotal));
		} else {
			sprintf(rr, "@%iHz", modeline->dotclock*1000/(modeline->htotal*modeline->vtotal));
		}
	}

	if (show_doublescan && ((modeline->flags) & V_DBLSCAN)) sprintf (ds, " D");

	sprintf(label_str, "%s%s%s%s", def, res, rr, ds);
	return (label_str);
}

/*******************************************************
* Create menu composed of available modes for a screen *
*******************************************************/
GtkWidget *create_screen_submenu (int screen)
{
	int i, dotclock, vm_count=0;
	MENU_CHOICE *menu_choice;
	char *label_str;
	GtkWidget *menuitem_ptr, *menu_ptr;
	XF86VidModeModeInfo **vm_modelines;
	XF86VidModeModeLine modeline;

	XF86VidModeGetAllModeLines(display, screen, &vm_count, &vm_modelines);
	XF86VidModeGetModeLine(display, screen, &dotclock, &modeline);

	menu_ptr = gtk_menu_new();

	for (i=0; i<vm_count; i++) {

		label_str = create_submenu_label(vm_modelines[i], modeline, dotclock);

		menu_choice = malloc(sizeof(menu_choice));
		menu_choice->screen=screen;
		menu_choice->mode=i;

		menuitem_ptr = gtk_menu_item_new_with_label(label_str);
		gtk_signal_connect(GTK_OBJECT (menuitem_ptr), "activate", GTK_SIGNAL_FUNC (MenuEvent), (gpointer)menu_choice);
		gtk_menu_append(GTK_MENU (menu_ptr), menuitem_ptr);
		gtk_widget_show(menuitem_ptr);
		free(label_str);
	}

	free(vm_modelines);

	return(menu_ptr);
}

/******************************************
* Update displayed resolution information *
******************************************/
void update_res_display(int screen)
{
	int dotclock;
	XF86VidModeModeLine modeline;
	XF86VidModeGetModeLine(display, screen, &dotclock, &modeline);

	/* Clear screen zone */
	copyXPMArea(76, 12, 35, 29, 14, 13);

	/* Show resolution */
	BlitNum(modeline.hdisplay, 41, 14);
	BlitString("X", 15, 23);
	BlitNum(modeline.vdisplay, 41, 23);

	/* Show frequency */
	if ((modeline.flags) & V_DBLSCAN) {
		BlitNum(dotclock*500/(modeline.htotal*modeline.vtotal), 29, 32);
	} else {
		BlitNum(dotclock*1000/(modeline.htotal*modeline.vtotal), 29, 32);
	}
	BlitString("Hz", 35, 32);

	RedrawWindow();
}

/*****************************
* Show screen number (SCR X) *
*****************************/
void show_screen_number(int screen)
{
	/* Clear screen zone */
	copyXPMArea(76, 12, 35, 29, 14, 13);

	BlitString("Scr", 15, 23);
	BlitNum(screen, 41, 23);
	RedrawWindow();
}

/* Blits a string at given co-ordinates */
void BlitString(char *name, int x, int y)
{
	int		i;
	int		c;
	int		k;

	k = x;
	for (i=0; name[i]; i++) {
		c = toupper(name[i]);
		if (c >= 'A' && c <= 'Z')
		{   /* its a letter */
			c -= 'A';
			copyXPMArea(c * 6, 74, 6, 8, k, y);
			k += 6;
		}
		else
		{   /* its a number or symbol */
			c -= '0';
			copyXPMArea(c * 6, 64, 6, 8, k, y);
			k += 6;
		}
	}
}


/* Blits number to give coordinates.. nine 0's, right justified */

void BlitNum(int num, int x, int y)
{
	char buf[1024];
	int newx=x;

	if ( num > 9 ) newx -= 6;
	if ( num > 99 ) newx -= 6;
	if ( num > 999 ) newx -= 6;
	if ( num > 9999 ) newx -= 6;

	sprintf(buf, "%i", num);

	BlitString(buf, newx, y);
}

/*******************************************************************************\
|* usage																	   *|
\*******************************************************************************/

void usage(void)
{
	printf ("\n wmressel version %s -- X11 resolution selector for Window Maker\n", WMRESSEL_VERSION );
	printf (" Copyright (C) 2002 Sébastien Liénard <%s>\n", MY_EMAIL);
	printf (" from GCU (%s)\n",GCU_HOME);
	printf (" This software comes with NO WARRANTY.\n\n");
	printf ("usage:\n");
	printf ("\t-r\t\t\tShow refresh rates in menu\n");
	printf ("\t-d\t\t\tShow in menu if a mode is a Doublescan mode (D)\n");
	printf ("\t-s <number>\t\tShow information about this screen\n");
	printf ("\t-o\t\t\tOnly show menu for selected screen\n");
	printf ("\t--display <display>\tSet display\n");
	printf ("\t-h\t\t\tthis help screen\n");
	printf ("\t-v\t\t\tprint the version number\n");
	printf ("\n");
}

/*******************************************************************************\
|* printVersion																   *|
\*******************************************************************************/

void printVersion(void) {
	fprintf(stderr, "wmressel v%s\n", WMRESSEL_VERSION);
}
