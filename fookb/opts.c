/*
 * File: opts.c
 *
 * (c) 1998-2004 Alexey Vyskubov <alexey@mawhrin.net>
 */

#include <stdio.h>

#include <string.h>
#include <stdlib.h>

#include <X11/Xlib.h>		/* X Window standard header */
#include <X11/Xresource.h>	/* X resource manager stuff */

#include "fookb.h"
#include "opts.h"

static int tblentr = 6;		/* There are 6 recognized
				   options */
static XrmOptionDescRec tbl[] = {
	{"-icon1", ".icon1",
	 XrmoptionSepArg,
	 (XPointer) NULL},
	{"-icon2", ".icon2",
	 XrmoptionSepArg,
	 (XPointer) NULL},
	{"-icon3", ".icon3",
	 XrmoptionSepArg,
	 (XPointer) NULL},
	{"-icon4", ".icon4",
	 XrmoptionSepArg,
	 (XPointer) NULL},
	{"-iconboom", ".iconBoom",
	 XrmoptionSepArg,
	 (XPointer) NULL},
	{"-display", ".display",
	 XrmoptionSepArg,
	 (XPointer) NULL}
};

void ParseOptions(int *argc, register char *argv[])
{
	XrmValue value;
	char *str_type[20];
	mydispname[0] = '\0';

	XrmParseCommand(&cmdlineDB, tbl, tblentr, "fookb", argc, argv);

#ifdef DEBUG
	puts("Hereiam");
#endif

	if (1 != *argc) {
		puts("Fookb v 3.0");
		puts("\tUsage: fookb [options]");
		puts("Possible options:");
		puts("-icon1 xpm_file\t\tIcon to show for the 1st Xkb group");
		puts("-icon2 xpm_file\t\tIcon to show for the 2nd Xkb group");
		puts("-icon3 xpm_file\t\tIcon to show for the 3rd Xkb group");
		puts("-icon4 xpm_file\t\tIcon to show for the 4th Xkb group");
		puts("-iconboom xpm_file\tIcon to show when Xkb system goes crazy");
		puts("-display X_display\tX display to use (normally not needed)");
		puts("");
#ifdef HAVE_WINGS_WUTIL_H
		puts("Command line parameters takes precedence over X resources or configuration file!");
		printf("Configuration file location: ");
#ifdef WMAKER
		puts("~/GNUstep/Defaults/FOOkb");
#else
		puts("~/.fookb");
#endif
#else
		puts("Command line paramaters takes precedence over X resources!");
#endif
		exit(0);
	}

	/* We should get display now -- we need it for access to other
	   databases */
	if (XrmGetResource(cmdlineDB, "fookb.display", "Fookb.Display",
			   str_type, &value) == True) {
		(void) strncpy(mydispname, value.addr, (size_t)value.size);
#ifdef DEBUG
		puts(mydispname);
#endif
	}

}

void MoreOptions(Display *dpy)
{

	XrmDatabase servDB, appDB;

	appDB = XrmGetFileDatabase
		("/usr/X11R6/lib/X11/app-defaults/Fookb");
	(void) XrmMergeDatabases(appDB, &finalDB);	/* Fookb defaults file
							   added into final
							   database */

	/* Let's look: does xrdb load server defautls? (As a property of
	   root window.) */
	if (XResourceManagerString(dpy) != NULL) {
		servDB = XrmGetStringDatabase(XResourceManagerString(dpy));
		XrmMergeDatabases(servDB, &finalDB);
	}

}
