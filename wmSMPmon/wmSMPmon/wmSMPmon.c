/***************************************************************************
                 wmSMPmon III - Window Maker system monitor
VERSION :           3.2
DATE    :           2005-11-06
ORIGINAL AUTHORS :  redseb <redseb@goupilfr.org> and
                    PhiR <phir@gcu-squad.org>
CONTRIBUTORS :      Alain Schröder <alain@parkautomat.net>
CURRENT MAINTAINER: Thomas Ribbrock <emgaron@gmx.net>
****************************************************************************
    This file is placed under the conditions of the GNU Library
    General Public License, version 2, or any later version.
    See file COPYING for information on distribution conditions.
***************************************************************************/

#include	<string.h>
#include	<X11/Xlib.h>
#include	<X11/xpm.h>
#include	<X11/extensions/shape.h>
#include	<libdockapp/wmgeneral.h>
#include	"wmSMPmon_master.xpm"
#include	"wmSMPmon_mask.xbm"
#include	"general.h"
#include	"standards.h"

#define		VERSION		"3.2"

/*###### Dividers for redraw-loops ######################################*/
#define		DIV1		6
#define		DIV2		10

/*###### Messages #######################################################*/
#define		MSG_NO_SWAP	"No swap mode : Swap is not monitored.\n"

/*###### Funcition definitions ##########################################*/
void usage(int cpus, const char *str);


/*###### MAIN PROGRAM ###################################################*/
int main(int argc, char **argv)
{
	XEvent		Event;

	unsigned int	t0[TAILLE_T], /* history for CPU 0 -> Graph */
			t1[TAILLE_T], /* history for CPU 1 -> Graph */
			tm[TAILLE_T], /* history for CPU 0+1 -> Graph */
			c1 = DIV1,
			c2 = DIV2,
			etat = 1,
			lecture = 1,
			delay = 250000,
			delta = 0,
			load = 0,
			load0o = 0,
			load1o = 0,
			no_swap = FAUX,
			draw_graph = VRAI,
			NumCPUs,      /* number of CPUs */
			i = 0,        /* counter */
			mem = 0, /* current memory/swap scaled to 0-100 */
			prec_mem = 0, /* memory from previous round */
			prec_swap = 0, /* swap from previous round */
			load_width = 3; /* width of load bar: 3 for SMP, 8 for UP */

	unsigned long	load0t = 0, load1t = 0;

	unsigned int	*CPU_Load; /* CPU load per CPU array */
	unsigned int	t_idx = 0; /* Index to load history tables */

	/********** Initialisation **********/
	NumCPUs = NumCpus_DoInit();
	CPU_Load = alloc_c((NumCPUs) * sizeof(int));

	if(NumCPUs == 1) {
		load_width = 8;
	} else {
		load_width = 3;
	}

	Myname = strrchr(argv[0], '/');
	if (Myname)
		++Myname;
	else
		Myname = argv[0];

	/* process command line args */
	i = 1; /* skip program name (i=0) */
	while (argc > i) {
		if (!strncmp(argv[i], "-r", 2)) {
			i++;
			if (i == argc) {
				/* parameter missing! */
				usage(NumCPUs,
				    "no refresh rate given when using -r!");
			} else {
				delay = atol(argv[i]) ;
			}
			i++;
			continue;
		}
		if (!strncmp(argv[i], "-h", 2)) {
			usage(NumCPUs, NULL);
		}
		if (!strncmp(argv[i], "-g", 2) && NumCPUs > 1) {
			/* we only support this on SMP systems */
			i++;
			if (i == argc) {
				/* parameter missing! */
				usage(NumCPUs,
				    "no graph style given when using -g!");
			} else {
			    etat = atoi(argv[i]);
			}

			if (1 > etat || etat > 3)
				usage(NumCPUs, "Unknown graph style");
			i++;
			continue;
		}
		if (!strncmp(argv[i], "-no-swap", 8)) {
			puts(MSG_NO_SWAP);
			no_swap = VRAI;
			i++;
			continue;
		}

		/* if we get here, we found an illegal option */
		usage(NumCPUs, "Illegal option!");
	}

	/* open initial window */
	if (NumCPUs == 1) {
		/* we only have a single CPU - change the mask accordingly
		 * NOTE: The for loop was derived from the differences between
		 * wmSMPmon_mask.xbm and wmSMPmon_mask-single.xbm.
		 * wmSMPmon_mask-single.xbm as such is NOT used in this
		 * program!
		 */
		for (i = 33; i <= 289; i = i+8) {
			wmSMPmon_mask_bits[i] = 0xDF;
		}
	}

	openXwindow(argc, argv, wmSMPmon_master_xpm, wmSMPmon_mask_bits,
	    wmSMPmon_mask_width, wmSMPmon_mask_height);

	if(NumCPUs >= 2) {
		/* we have two CPUs -> draw separator between CPU load bars */
		copyXPMArea(12, 4, 2, HAUTEUR + 2, 7, 4);
	}

	delay = delay / 2 ;

	for (i = 0; i < TAILLE_T; i ++) {
		t0[i] = 0;
		t1[i] = 0;
		tm[i] = 0;
	}

	/* -no-swap option was given */
	if (no_swap)
	    copyXPMArea(60, 63, 60, 10, 6, 50);

	/* MAIN LOOP */
	while (VRAI) {
		if (lecture) {
			CPU_Load = Get_CPU_Load(CPU_Load, NumCPUs);

			load = CPU_Load[0];
			for (i = 1; i < NumCPUs >> 1; i++) {
				load += CPU_Load[i];
			}
			load = load / i;
			load0t = load0t + load;
			if (load != load0o) {
				/* redraw only if cpu load changed */
				delta = HAUTEUR - load;
				copyXPMArea(108, 0, load_width, HAUTEUR, 4, 5);
				copyXPMArea(108, delta + 32, load_width, load,
				    4, 5 + delta);
				load0o = load;
			}

			if (NumCPUs >= 2) {
				/* we have two CPUs -> do CPU 1 */
				load = 0;
				for (; i < NumCPUs; i++) {
					load += CPU_Load[i];
				}
				load = load / (NumCPUs >> 1);

				if (load != load1o) {
					/* redraw only if cpu load changed */
					delta = HAUTEUR - load;
					copyXPMArea(108, 0, 3, HAUTEUR, 9, 5);
					copyXPMArea(108, delta + 32, 3, load,
					    9, 5 + delta);
					load1o = load;
				}
			}

			/* we have to set load1t in any case to get the correct
			 * graph below. With only one CPU, 'load' will still be
			 * CPU_Load[0], on a SMP system, it will be CPU_Load[1].
			 */
			load1t = load1t + load;

			if (c1 > DIV1) {
				mem = Get_Memory();

				if (mem != prec_mem) {
					/* redraw only if mem changed */
					copyXPMArea(30, 63, 30, 8, 29, 39);
					copyXPMArea(0, 63, (mem * 30 / 100), 8,
					    29, 39);
					prec_mem = mem;
				}

				if (!no_swap) {
					mem = Get_Swap();

					if (mem != prec_swap) {
						/* redraw if there was a change */
						if (mem == 999) {
							/* swap is disabled => show "none" */
							copyXPMArea(60, 63, 60, 10, 6, 50);
						} else {
							/* draw swap usage */
							copyXPMArea(30, 63, 30, 8, 29, 50);
							copyXPMArea(0, 63, (mem * 30 / 100), 8, 29, 50);
						}
						prec_swap = mem;
					}
				}
				c1 = 0;
			}

			if (c2 > DIV2) {
				if ((t0[t_idx] = load0t / c2) > HAUTEUR)
					t0[t_idx] = HAUTEUR;
				t0[t_idx] /= 2;
				if ((t1[t_idx] = load1t / c2) > HAUTEUR)
					t1[t_idx] = HAUTEUR;
				t1[t_idx] /= 2;
				if ((tm[t_idx] = (load0t + load1t) / (2 * c2)) > HAUTEUR)
					tm[t_idx] = HAUTEUR;
				load0t = 0;
				load1t = 0;
				t_idx = (t_idx + 1) % TAILLE_T;
				draw_graph = VRAI;
				c2 = 0;
			}

			if (draw_graph) {
				/* draw graph */
				switch (etat) {
				case 1 :
					copyXPMArea(64, 32, TAILLE_T, HAUTEUR, 15, 5);
					for (i = 0, load = t_idx; i < TAILLE_T; i ++, load++)
						copyXPMArea(116, 0, 1, tm[load % TAILLE_T], 15 + i, HAUTEUR + 5 - tm[load % TAILLE_T]);
					break;
				case 2 :
					copyXPMArea(64, 0, TAILLE_T, HAUTEUR, 15, 5);
					for (i = 0, load = t_idx; i < TAILLE_T; i ++, load++) {
						copyXPMArea(116, 0, 1, t0[load % TAILLE_T], 15 + i, HAUTEUR/2 + 5 - t0[load % TAILLE_T]);
						copyXPMArea(116, 0, 1, t1[load % TAILLE_T], 15 + i, HAUTEUR/2 + 21 - t1[load % TAILLE_T]);
					}
					break;
				case 3 :
					copyXPMArea(64, 0, TAILLE_T, HAUTEUR, 15, 5);
					for (i = 0, load = t_idx; i < TAILLE_T; i ++, load++) {
						copyXPMArea(116, 0, 1, t0[load % TAILLE_T], 15 + i, HAUTEUR/2 + 5 - t0[load % TAILLE_T]);
						copyXPMArea(117, HAUTEUR/2 - t1[load % TAILLE_T], 1, t1[load % TAILLE_T], 15 + i, HAUTEUR/2 + 6);
					}
					break;
				}
				draw_graph = FAUX;
			}
			c1++;
			c2++;
		}
		lecture = 1 - lecture ;
		RedrawWindow();
		if (NumCPUs >= 2 &&
		    XCheckMaskEvent(display, ButtonPressMask, &Event)) {
			/* changing graph style not supported on single CPU systems */
			if (Event.type == ButtonPress) {
				if ((etat++) >= 3)
					etat = 1;
				draw_graph = VRAI;
			}
		}
		usleep(delay);
	}
}

/*###### Usage Message ##################################################*/
void usage(int cpus, const char *str)
{
	fflush(stdout);

	if (str) {
		fprintf(stderr, "\nERROR: %s\n", str);
	}

	fputs("\nwmSMPmon "VERSION" - display system load (", stderr);
	if(cpus == 1) {
		fputs("uniprocessor system)\n\n", stderr);
	} else {
		fputs("multiprocessor system)\n\n", stderr);
	}
	fputs("Options : -h        this help screen.\n"
	    "          -r RATE   refresh rate (in microseconds, default 250000).\n",
	    stderr);

	if(cpus > 1) {
		fputs("          -g STYLE  graph style (try 2 or 3, default is 1).\n",
		    stderr);
	}

	fputs("          -no-swap  don't monitore swap size.\n\n"
	    "<redseb@goupilfr.org> http://goupilfr.org\n"
	    "<phir@gcu-squad.org> http://gcu-squad.org\n"
	    "<emgaron@gmx.net> http://www.ribbrock.org\n",
	    stderr);

	exit(OK);
}
