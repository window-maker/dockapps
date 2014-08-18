/*
 * acpi-ng: command line acpi battery status tool.
 *
 * Written by Simon Fowler <simon@dreamcraft.com.au>, 2003-06-20.
 * Copyright 2003-06-20 Dreamcraft Pty Ltd.
 *
 * This file is distributed under the GNU General Public License, 
 * version 2. Please see the COPYING file for details.
 */

/*
 * 2003-06-20.
 * I'm getting sick of not having a convenient way to query battery
 * status on the command line, so I'm hacking up this - a quick little
 * command line tool to display current battery status, using the same
 * libacpi code as wmacpi-ng.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>

#include "libacpi.h"

#define ACPI_NG_VER "0.99"

global_t *globals;

void usage(char *name)
{
	printf("%s: query battery status on ACPI enabled systems.\n"
	       "Usage:\n"
	       "%s [-h] [-a samples]\n"
	       " h - display this help information\n"
	       " a - average remaining time over some number of samples\n"
	       "     much more accurate than using a single sample\n"
	       " v - increase verbosity\n",
	       name, name);
}

void print_version(void)
{
	printf("acpi-ng version %s\n", ACPI_NG_VER);
	printf(" Using libacpi version %s\n", LIBACPI_VER);
}

int main(int argc, char *argv[])
{
	int i, j, ch;
	int sleep_time = 0;
	int samples = 1;
	battery_t *binfo;
	adapter_t *ap;

	while((ch = getopt(argc, argv, "hvVa:")) != EOF) {
		switch(ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 'v':
			verbosity++;
			break;
		case 'V':
			print_version();
			return 0;
		case 'a':
			if(optarg != NULL) {
				samples = atoi(optarg);
				if(samples > 1000 || samples <= 0) {
					printf("Please specify a reasonable number of samples\n");
					exit(1);
			}
			}
			printf("samples: %d\n", samples);
			sleep_time = 1000000/samples;
			break;
		default:
			usage(argv[0]);
			return 1;
		}
	}

	globals = (global_t *) malloc(sizeof(global_t));

	power_init();
	/* we want to acquire samples over some period of time, so . . . */
	for(i = 0; i < samples + 2; i++) {
		for(j = 0; j < batt_count; j++)
			acquire_batt_info(j);
		acquire_global_info();
		usleep(sleep_time);
	}
	
	ap = &globals->adapter;
	if(ap->power == AC) {
		printf("On AC Power");
		for(i = 0; i < batt_count; i++) {
			binfo = &batteries[i];
			if(binfo->present && (binfo->charge_state == CHARGE)) {
				printf("; Battery %s charging", binfo->name);
				printf(", currently at %2d%%", binfo->percentage);
				if(binfo->charge_time >= 0) 
					printf(", %2d:%02d remaining", 
					       binfo->charge_time/60,
					       binfo->charge_time%60);
			}
		}
		printf("\n");
	} else if(ap->power == BATT) {
		printf("On Battery");
		for(i = 0; i < batt_count; i++) {
			binfo = &batteries[i];
			if(binfo->present && (binfo->percentage >= 0))
				printf(", Battery %s at %d%%", binfo->name,
				       binfo->percentage);
		}
		if(globals->rtime >= 0)
			printf("; %d:%02d remaining", globals->rtime/60, 
			       globals->rtime%60);
		printf("\n");
	}
	return 0;
}

