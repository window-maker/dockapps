/* WMix -- a mixer using the OSS mixer API.
 * Copyright (C) 2014 Christophe CURIS for the WindowMaker Team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*
 * config.c: functions related to loading the configuration, both from
 * command line options and from file
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <sys/soundcard.h>

#include "include/common.h"
#include "include/config.h"


#define HELP_TEXT	  \
	"WMixer " VERSION " by timecop@japan.co.jp + skunk@mit.edu\n" \
	"usage:\n" \
	"  -d <dsp>  connect to remote X display\n" \
	"  -e <name> exclude channel, can be used many times\n" \
	"  -f <file> parse this config [~/.wmixrc]\n" \
	"  -h        print this help\n" \
	"  -m <dev>  mixer device [/dev/mixer]\n" \
	"  -v        verbose -> id, long name, name\n" \

/* The global configuration */
struct _Config config;


/*
 * Sets the default values in configuration
 */
void config_init(void)
{
	memset(&config, 0, sizeof(config));

	config.mousewheel = 1;
	config.scrolltext = 1;
	config.wheel_button_up = 4;
	config.wheel_button_down = 5;
	config.scrollstep = 0.03;
	config.osd = 1;
	config.osd_color = strdup("green");
}

/*
 * Parse Command-Line options
 *
 * Supposed to be called before reading config file, as there's an
 * option to change its name
 */
void parse_cli_options(int argc, char **argv)
{
	int opt;
	int count_exclude = 0;
	bool error_found;

	opterr = 0;	/* We take charge of printing the error message */
	config.verbose = false;
	error_found = false;
	for (;;) {
		opt = getopt(argc, argv, ":d:e:f:hm:v");
		if (opt == -1)
			break;

		switch (opt) {
		case '?':
			fprintf(stderr, "wmix:error: unknow option '-%c'\n", optopt);
			error_found = true;
			break;

		case ':':
			fprintf(stderr, "wmix:error: missing argument for option '-%c'\n", optopt);
			error_found = true;
			break;

		case 'd':
			config.display_name = strdup(optarg);
			break;

		case 'e':
			if (count_exclude < SOUND_MIXER_NRDEVICES) {
				config.exclude_channel[count_exclude] = strdup(optarg);
				count_exclude++;
			} else
				fprintf(stderr, "Warning: You can't exclude this many channels\n");
			break;

		case 'f':
			if (config.file != NULL)
				free(config.file);
			config.file = strdup(optarg);
			break;

		case 'h':
			fputs(HELP_TEXT, stdout);
			exit(0);
			break;

		case 'm':
			config.mixer_device = strdup(optarg);
			break;

		case 'v':
			config.verbose = true;
			break;

		default:
			break;
		}
	}
	config.exclude_channel[count_exclude] = NULL;

	if (optind < argc) {
		fprintf(stderr, "wmix:error: argument '%s' not understood\n", argv[optind]);
		error_found = true;
	}

	if (error_found)
		exit(EXIT_FAILURE);
}

/*
 * Read configuration from a file
 *
 * The file name is taken from CLI if available, of falls back to
 * a default name.
 */
void config_read(void)
{
	const char *filename;
	char buffer_fname[512];
	FILE *fp;
	char buf[512];
	char *ptr;

	if (config.file != NULL) {
		filename = config.file;
	} else {
		const char *home;

		home = getenv("HOME");
		if (home == NULL) {
			fprintf(stderr, "wmix: warning, could not get $HOME, can't load configuration file\n");
			return;
		}
		snprintf(buffer_fname, sizeof(buffer_fname), "%s/.wmixrc", home);
		filename = buffer_fname;
	}

	fp = fopen(filename, "r");
	if (fp == NULL) {
		if (config.file != NULL) {
			/* The config file was explicitely specified by user, tell him there's a problem */
			fprintf(stderr, "wmix: error, could not load configuration file \"%s\"\n", filename);
			exit(EXIT_FAILURE);
		}
		/* Otherwise, it is acceptable if the file does not exist */
		return;
	}
	if (config.verbose)
		printf("Using configuration file: %s\n", filename);

	while (fgets(buf, 512, fp)) {
		if ((ptr = strstr(buf, "mousewheel="))) {
			ptr += 11;
			config.mousewheel = atoi(ptr);
		}
		if ((ptr = strstr(buf, "scrolltext="))) {
			ptr += 11;
			config.scrolltext = atoi(ptr);
		}
		if ((ptr = strstr(buf, "osd="))) {
			ptr += 4;
			config.osd = atoi(ptr);
		}
		if ((ptr = strstr(buf, "osdcolor="))) {
			char *end;
			ptr += 9;
			end = strchr(ptr, '\n');
			ptr[end - ptr] = '\0';
			if (config.osd_color)
				free(config.osd_color);
			config.osd_color = strdup(ptr);
		}
		if ((ptr = strstr(buf, "wheelstep="))) {
			ptr += 10;
			/* detect old style config */
			if (atoi(ptr) > 1)
				config.scrollstep = (float)atoi(ptr) / 100.0;
			else
				config.scrollstep = atof(ptr);
		}
		if ((ptr = strstr(buf, "wheelbtn1="))) {
			ptr += 10;
			config.wheel_button_up = atoi(ptr);
		}
		if ((ptr = strstr(buf, "wheelbtn2="))) {
			ptr += 10;
			config.wheel_button_down = atoi(ptr);
		}
	}
	fclose(fp);
}
