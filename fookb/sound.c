/*
 * File: sound.c
 *
 * (c) 1998-2004 Alexey Vyskubov <alexey@mawhrin.net>
 */

#include <unistd.h>		/* fork() */
#include <sys/types.h>		/* pid_t */
#include <stdlib.h>		/* system */
#include <errno.h>
#include <stdio.h>		/* puts() */
#include <string.h>


#include "sound.h"
#include "params.h"

void drip()
{

	pid_t pid;

	pid = fork();

	if (pid == (pid_t) (-1)) {
		puts("Cannot fork!");
		switch (errno) {
		case ENOMEM:
			puts("Not enough memory!");
			exit(22);
			break;
		case EAGAIN:
			puts("To many processes!");
			exit(23);
			break;
		default:
			puts("Unknown error, please report!");
		}
	}

	if (pid == (pid_t) 0) {
		return;
	}

	if (!strcmp(read_param("Sound"), "Yes")) {
		exit(system(read_param("Command")));
	} else {
		exit(0);
	}

}
