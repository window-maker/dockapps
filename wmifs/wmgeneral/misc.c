/*  wmgeneral miscellaneous functions
 *
 *  from dock.c - built-in Dock module for WindowMaker window manager
 *
 *  Copyright (c) 1997 Alfredo K. Kojima
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301
 *  USA.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "list.h"
#include "misc.h"

/*
 *----------------------------------------------------------------------
 * parse_command--
 * 	Divides a command line into a argv/argc pair.
 *----------------------------------------------------------------------
 */
#define PRC_ALPHA	0
#define PRC_BLANK	1
#define PRC_ESCAPE	2
#define PRC_DQUOTE	3
#define PRC_EOS		4
#define PRC_SQUOTE	5

typedef struct {
    short nstate;
    short output;
} DFA;


static DFA mtable[9][6] = {
    {{3,1},{0,0},{4,0},{1,0},{8,0},{6,0}},
    {{1,1},{1,1},{2,0},{3,0},{5,0},{1,1}},
    {{1,1},{1,1},{1,1},{1,1},{5,0},{1,1}},
    {{3,1},{5,0},{4,0},{1,0},{5,0},{6,0}},
    {{3,1},{3,1},{3,1},{3,1},{5,0},{3,1}},
    {{-1,-1},{0,0},{0,0},{0,0},{0,0},{0,0}}, /* final state */
    {{6,1},{6,1},{7,0},{6,1},{5,0},{3,0}},
    {{6,1},{6,1},{6,1},{6,1},{5,0},{6,1}},
    {{-1,-1},{0,0},{0,0},{0,0},{0,0},{0,0}}, /* final state */
};

char*
next_token(char *word, char **next)
{
    char *ptr;
    char *ret, *t;
    int state, ctype;

    t = ret = malloc(strlen(word)+1);
    if (ret == NULL) {
	    fprintf(stderr, "Insufficient memory.\n");
	    exit(EXIT_FAILURE);
    }
    ptr = word;

    state = 0;
    *t = 0;
    while (1) {
	if (*ptr==0)
	    ctype = PRC_EOS;
	else if (*ptr=='\\')
	    ctype = PRC_ESCAPE;
	else if (*ptr=='"')
	    ctype = PRC_DQUOTE;
	else if (*ptr=='\'')
	    ctype = PRC_SQUOTE;
	else if (*ptr==' ' || *ptr=='\t')
	    ctype = PRC_BLANK;
	else
	    ctype = PRC_ALPHA;

	if (mtable[state][ctype].output) {
	    *t = *ptr; t++;
	    *t = 0;
	}
	state = mtable[state][ctype].nstate;
	ptr++;
	if (mtable[state][0].output<0) {
	    break;
	}
    }

    if (*ret==0)
	t = NULL;
    else
	t = strdup(ret);

    free(ret);

    if (ctype==PRC_EOS)
	*next = NULL;
    else
	*next = ptr;

    return t;
}


extern void
parse_command(char *command, char ***argv, int *argc)
{
    LinkedList *list = NULL;
    char *token, *line;
    int count, i;

    line = command;
    do {
	token = next_token(line, &line);
	if (token) {
	    list = list_cons(token, list);
	}
    } while (token!=NULL && line!=NULL);

    count = list_length(list);
    *argv = malloc(sizeof(char*)*count);
    i = count;
    while (list!=NULL) {
	(*argv)[--i] = list->head;
	list_remove_head(&list);
    }
    *argc = count;
}

extern pid_t
execCommand(char *command)
{
    pid_t pid;
    char **argv;
    int argc;

    parse_command(command, &argv, &argc);

    if (argv==NULL) {
        return 0;
    }

    if ((pid=fork())==0) {
        char **args;
        int i;

        args = malloc(sizeof(char*)*(argc+1));
        if (!args)
          exit(10);
        for (i=0; i<argc; i++) {
            args[i] = argv[i];
        }
        args[argc] = NULL;
        execvp(argv[0], args);
        exit(10);
    }
    free(argv);
    return pid;
}
