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
#include <ctype.h>
#include <getopt.h>
#include <limits.h>

#include <sys/soundcard.h>

#include "include/common.h"
#include "include/config.h"
#include "include/misc.h"

#define VERSION_TEXT \
	"WMixer " VERSION " by timecop@japan.co.jp + skunk@mit.edu\n"

#define HELP_TEXT \
	"usage:\n" \
        "  -a <api>  use this sound api (oss or alsa) [alsa]\n" \
	"  -d <dsp>  connect to remote X display\n" \
	"  -e <name> exclude channel, can be used many times\n" \
	"  -f <file> parse this config [~/.wmixrc]\n" \
	"  -h        print this help\n" \
	"  -k        disable grabing volume control keys\n" \
	"  -m <dev>  oss mixer device [/dev/mixer]\n" \
	"            or alsa card name [default]\n" \
	"  -o <num>  display osd on this monitor number or name [0]\n" \
	"            use -1 to disable osd\n" \
	"  -v        verbose -> id, long name, name\n" \

/* The global configuration */
struct _Config config;

/* The default device used for Mixer control */
static const char default_mixer_device[] = "/dev/mixer";
static const char default_card_name[] = "default";
/* Default color for OSD */
const char default_osd_color[] = "green";


/*
 * Sets the default values in configuration
 */
void config_init(void)
{
	memset(&config, 0, sizeof(config));
        config.api = 0;
	config.mixer_device = NULL;
	config.mousewheel = 1;
	config.scrolltext = 1;
	config.mmkeys = 1;
	config.wheel_button_up = 4;
	config.wheel_button_down = 5;
	config.scrollstep = 0.03;
	config.osd = 1;
	config.osd_color = (char *) default_osd_color;
	config.osd_monitor_number = 0;
	config.osd_monitor_name = NULL;
}

/*
 * Release memory associated with configuration
 *
 * This does not concern the complete configuration, only the parameters
 * that are needed during startup but are not useful during run-time
 */
void config_release(void)
{
	int i;

	if (config.file)
		free(config.file);

	if (config.display_name)
		free(config.display_name);

	if (config.mixer_device != default_mixer_device
            && config.mixer_device != default_card_name)
		free(config.mixer_device);

	if (config.osd_color != default_osd_color)
		free(config.osd_color);

	for (i = 0; i < EXCLUDE_MAX_COUNT; i++) {
		if (config.exclude_channel[i])
			free(config.exclude_channel[i]);
		else
			break;
	}
}

void parse_monitor_value(char *value)
{
	char *end;
	long mon = strtol(value, &end, 10);
	if (end == value + strlen(value)) {
		if ((mon > INT_MAX) || (mon < -1)) {
			fprintf(stderr, "wmix:warning: unreasonable monitor number provided, falling back to default\n");
			config.osd_monitor_number = 0;
		} else {
			if (mon == -1)
				config.osd = 0;
			else
				config.osd_monitor_number = (int)mon;
		}
	} else {
		config.osd_monitor_name = strdup(value);
	}
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
		opt = getopt(argc, argv, ":a:d:e:f:hkm:o:v");
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
		case 'a':
			if(!strcmp("oss", optarg))
				config.api = 1;
			else if (strcmp("alsa", optarg))
				fprintf(stderr, "Warning: Incorrect sound api specified, defaulting to alsa\n");
			break;
		case 'd':
			if (config.display_name)
				free(config.display_name);
			config.display_name = strdup(optarg);
			break;

		case 'e':
			if (count_exclude < EXCLUDE_MAX_COUNT) {
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
			fputs(VERSION_TEXT, stdout);
			fputs(HELP_TEXT, stdout);
			exit(0);
			break;

		case 'k':
			config.mmkeys = false;
			break;

		case 'm':
			if (config.mixer_device != default_mixer_device)
				free(config.mixer_device);
			config.mixer_device = strdup(optarg);
			break;

		case 'o':
			parse_monitor_value(optarg);
			break;

		case 'v':
			config.verbose = true;
			break;

		default:
			break;
		}
	}
	config.exclude_channel[count_exclude] = NULL;

	if (!config.mixer_device) {
		if (config.api == 0)
			config.mixer_device = (char *)default_card_name;
		else if (config.api == 1)
			config.mixer_device = (char *)default_mixer_device;
	}

	if (optind < argc) {
		fprintf(stderr, "wmix:error: argument '%s' not understood\n", argv[optind]);
		error_found = true;
	}

	if (error_found)
		exit(EXIT_FAILURE);

	if (config.verbose)
		fputs(VERSION_TEXT, stdout);
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
	int line;
	char buf[512];

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

	line = 0;
	while (fgets(buf, 512, fp)) {
		char *ptr;
		char *keyword;
		char *value;

		line++;

		ptr = buf;
		while (isspace(*ptr))
			ptr++;

		if ((*ptr == '\0') || (*ptr == '#'))
			continue;

		/* Isolate the keyword */
		keyword = ptr;
		if (*ptr == '=') {
			fprintf(stderr, "wmix:warning: syntax error at line %d in \"%s\", no keyword before '='\n",
			        line, filename);
			continue;
		}
		value = NULL;
		while (*ptr) {
			if (*ptr == '=') {
				value = ptr + 1;
				break;
			}
			if (*ptr == '#')
				break;
			ptr++;
		}
		if (value == NULL) {
			fprintf(stderr, "wmix:warning: syntax error at line %d in \"%s\", missing '='\n",
			        line, filename);
			continue;
		}
		while (isspace(ptr[-1]))
			ptr--;
		*ptr = '\0';

		/* Isolate the value */
		while (isspace(*value))
			value++;
		ptr = value;
		while (*ptr) {
			if (*ptr == '#')
				break;
			ptr++;
		}
		while (isspace(ptr[-1]))
			ptr--;
		*ptr = '\0';

		/* Check what keyword we have */
		if (strcmp(keyword, "device") == 0) {
			if (config.mixer_device == default_mixer_device)
				config.mixer_device = strdup(value);
			/* If not the default, keep the previous value because it was provided in the command-line */

		} else if (strcmp(keyword, "exclude") == 0) {
			int i;

			for (i = 0; i < EXCLUDE_MAX_COUNT; i++) {
				if (config.exclude_channel[i] == NULL) {
					config.exclude_channel[i] = strdup(value);
					config.exclude_channel[i+1] = NULL;
					break;
				}

				if (strcmp(value, config.exclude_channel[i]) == 0)
					break;
			}
		} else if (strcmp(keyword, "mousewheel") == 0) {
			config.mousewheel = atoi(value);

		} else if (strcmp(keyword, "osd") == 0) {
			config.osd = atoi(value);

		} else if (strcmp(keyword, "osdcolor") == 0) {
			if (config.osd_color != default_osd_color)
				free(config.osd_color);
			config.osd_color = strdup(value);

		} else if (strcmp(keyword, "osdmonitor") == 0) {
			parse_monitor_value(value);

		} else if (strcmp(keyword, "scrolltext") == 0) {
			config.scrolltext = atoi(value);

		} else if (strcmp(keyword, "wheelbtn1") == 0) {
			config.wheel_button_up = atoi(value);

		} else if (strcmp(keyword, "wheelbtn2") == 0) {
			config.wheel_button_down = atoi(value);

		} else if (strcmp(keyword, "wheelstep") == 0) {
			double val;

			val = atof(value);
			if (val < 0.0 || val > 100.0)
				fprintf(stderr, "wmix:error: value %f is out of range for wheelstep in %s at line %d\n",
				        val, filename, line);
			else if (val >= 1.0)
				config.scrollstep = val / 100.0;
			else if (val > 0.0)
				config.scrollstep = val;
			else
				fprintf(stderr, "wmix:error: value '%s' not understood for wheelstep in %s at line %d\n",
				        value, filename, line);
		} else {
			fprintf(stderr, "wmix:warning: unknow keyword '%s' at line %d of \"%s\", ignored\n",
			        keyword, line, filename);
		}
	}
	fclose(fp);
}
