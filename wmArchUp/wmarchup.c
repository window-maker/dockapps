/*
 * Copyright (c) 2002 Alban G. Hertroys
 *
 * Basic example of libDockapp usage
 *
 * This dockapp will draw a rectangle with a
 * bouncing ball in it.
 */

/* also includes Xlib, Xresources, XPM, stdlib and stdio */
#include <dockapp.h>
#include <stdio.h>

/* include the pixmap to use */
#include "archlinux.xpm"
#include "archlinux_bw.xpm"

#define TRUE 1
#define FALSE 0
#define MAX 256

/*
 * Prototypes for local functions
 */
void destroy(void);
int check_for_updates();

/*
 * Global variables
 */
Pixmap arch, arch_mask, arch_bw, arch_bw_mask;
unsigned short	height, width;
XEvent ev;

/*
 * M A I N
 */

int
main(int argc, char **argv)
{
    DACallbacks eventCallbacks = {
        destroy, /* destroy */
        NULL, /* buttonPress */
        NULL, /* buttonRelease */
        NULL, /* motion (mouse) */
        NULL, /* mouse enters window */
        NULL, /* mouse leaves window */
        NULL  /* timeout */
    };

    /* provide standard command-line options */
    DAParseArguments(
        argc, argv,	/* Where the options come from */
        NULL, 0,	/* Our list with options - none as you can see */
        "This is dockapp watch for available updates "
        "in Arch Linux packages.\n",
        "wmarchup version 1.0");

    /* Tell libdockapp what version we expect it to be (a date from
     * NEWS should do).
     */
    DASetExpectedVersion(20050716);

    DAInitialize("", "WMArchUp", 56, 56, argc, argv);

    DAMakePixmapFromData(
        archlinux,
        &arch,
        &arch_mask,
        &height,
        &width);

    DAMakePixmapFromData(
        archlinux_bw,
        &arch_bw,
        &arch_bw_mask,
        &height,
        &width);

    DASetShape(arch_bw_mask);
    DASetPixmap(arch_bw);

    /* Respond to destroy and timeout events (the ones not NULL in the
     * eventCallbacks variable.
     */
    DASetCallbacks(&eventCallbacks);

    DAShow(); /* Show the dockapp window. */

    while (1) {
        if (check_for_updates() == TRUE) {
            DASetShape(arch_mask);
            DASetPixmap(arch);
        } else {
            DASetShape(arch_bw_mask);
            DASetPixmap(arch_bw);
        }

        /* handle all pending X events */
        while (XPending(DADisplay)) {
            XNextEvent(DADisplay, &ev);
            DAProcessEvent(&ev);
        }
        sleep(600);
    }

    /* not reached */
    exit(EXIT_SUCCESS);
}

int
check_for_updates() {
    FILE *fp;
    char res[MAX];

    /* Read output from command */
    fp = popen("checkupdates","r");
    if( fgets (res, MAX, fp)!=NULL ) {
        fclose(fp);
        return TRUE;
    } else {
        return FALSE;
    }
}

void
destroy(void)
{
    return;
}
