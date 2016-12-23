/*
 * Copyright (C) 12 Jun 2003 Tomas Cermak
 *
 * This file is part of wmradio program.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "fifo.h"
#include "wmradio.h"
#include<stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define FIFO_FILE_PREFIX "/tmp/wmradio_"
FILE *fifo;

#define MAX_STRINGS 40
char *read_line(FILE *file)
{
    char *str=(char *)malloc(MAX_STRINGS);
    int fin=0;
    int size=MAX_STRINGS;
    int retries;
    struct timespec pause = { 0, 100000000 };

    if (fgets(str,MAX_STRINGS,file)==NULL)
    {
	free(str);
	return NULL;
    }
    for(retries=3; retries; retries--) {
	if(!fin) {
	    fin=1;
	    if (strchr(str,'\n')==NULL) {
		char *tmp=(char *)malloc(size+MAX_STRINGS-1);
		nanosleep(&pause, &pause);
		strcpy(tmp,str);
		fgets(tmp+size-1,MAX_STRINGS,file);
		free(str);
		str=tmp;
		size+=MAX_STRINGS-1;
		fin=0;
	    }
	}
    }
    if(!fin) {
	free(str);
	return NULL;
    }
    return str;
}

char * fifo_filename() {
    static char *name = NULL;
    char buffer[100];

    if (name == NULL) {
        snprintf(buffer,sizeof(buffer),"%i",getuid());
	name = malloc(sizeof(char)*strlen(buffer)+
		      sizeof(FIFO_FILE_PREFIX)+1);
	sprintf(name, "%s%s", FIFO_FILE_PREFIX, buffer);
    }
    return name;
}

void fifo_init() {
    int fifo_fd;
    mkfifo(fifo_filename(), 0666);
    fifo_fd = open(fifo_filename(), O_NONBLOCK | O_RDONLY);
    fifo = fdopen(fifo_fd, "r");
}

void fifo_close() {
    fclose(fifo);
    unlink(fifo_filename());
}

void fifo_parse () {
    char *line = read_line(fifo);
    char *command;
    char *rest=NULL;

    if (line) {
	command=(char *)malloc(strlen(line)+1);
	sscanf(line,"%s",command);
	rest=line+strlen(command);

	if (!strcasecmp(command,"TUNE_PLUS")) {
	    wmradio_command(TUNE_PLUS, 0);
	}
	else if (!strcasecmp(command,"TUNE_MINUS"))
	{
	    wmradio_command(TUNE_MINUS, 0);
	}
	if (!strcasecmp(command,"FINE_TUNE_PLUS")) {
	    wmradio_command(TUNE_PLUS, 1);
	}
	else if (!strcasecmp(command,"FINE_TUNE_MINUS"))
	{
	    wmradio_command(TUNE_MINUS, 1);
	}
	else if (!strcasecmp(command,"POWER"))
	{
	    wmradio_command(POWER_SWITCH, 0);
	}
	else if (!strcasecmp(command,"SET_PRESET"))
	{
	    int i;
	    if (sscanf(rest,"%i",&i)==1) {
                if (i>=0 && i<6)
		    wmradio_command(SET_PRESET, i);
	    }
	}
	else if (!strcasecmp(command,"SAVE_PRESET"))
	{
	    int i;
	    if ((sscanf(rest,"%i",&i))==1) {
                if (i>=0 && i<6)
		    wmradio_command(SAVE_PRESET, i);
	    }
	}
	else if (!strcasecmp(command,"SCAN"))
	{
	    wmradio_command(SCAN, 0);
	}
	else if (!strcasecmp(command,"TUNE_NAME_PREV"))
	{
	    wmradio_command(TUNE_NAME_PREV, 0);
	}
	else if (!strcasecmp(command,"TUNE_NAME_NEXT"))
	{
	    wmradio_command(TUNE_NAME_NEXT, 0);
	}
	else if (!strcasecmp(command,"READ_CONFIG"))
	{
	    wmradio_command(READ_CONFIG, 0);
	}
	free(command);
        free(line);
    }
}
