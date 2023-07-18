/*
 * GAI - General Applet Interface Library
 * Copyright (C) 2003-2004 Jonas Aaberg <cja@gmx.net>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 *             Dedicated to Evelyn Reimann - Min ss sv gp af!!
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "../config.h"
#include "gai.h"
#include "gai-private.h"

#define GAI_DEBUG_FILE "/tmp/gai-debug-output"

const char GAI_spaces[] = "                                                               ";
static GSList *error_list = NULL;

void 
gai_log_debug_init(void)
{
    struct tm *my_tm;
    time_t my_t;

    if(! GAI.debug) 
	return;

    GAI.debug_depth = 0;
    
    my_t = time (NULL);

    my_tm = localtime (&my_t);
    
    GAI.debug_output = fopen(GAI_DEBUG_FILE, "a");
    if(GAI.debug_output <= 0) return;

    fprintf (GAI.debug_output,"\n*** %s starting new applet ***\n",
	     asctime (my_tm));
    fflush (GAI.debug_output);
}

static void gai_display_error_box(const char *str, void *cb)
{
    GtkWidget *w;

    w = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_ERROR,GTK_BUTTONS_OK, str);
    gtk_widget_show_all(w);
    g_signal_connect_swapped(G_OBJECT(w), "response",
			     G_CALLBACK(cb), G_OBJECT (w));

}


static void gai_queue_error(const char *str)
{
    if(error_list == NULL)
	error_list =  g_slist_alloc();
    error_list = g_slist_append(error_list, g_strdup(str));
}

void gai_display_queued_errors(void)
{
    int i;
    char *str;

    if(error_list != NULL){
	for(i=0;i<g_slist_length(error_list);i++){
	    str = g_slist_nth_data(error_list, i);
	    if(str != NULL){
		gai_display_error_box(str, gtk_widget_destroy);
		g_free(str);
	    }
	}
	g_slist_free(error_list);
	error_list = NULL;
    }
}

void 
gai_display_error_quit(const char *str)
{
    g_assert(str !=NULL);

    if(gai_instance != NULL)
	GAI_NOTE (str);

    fprintf(stderr, " *** GAI Error: %s\n", str);

    if(gai_instance == NULL)
	gtk_init(NULL, NULL);

    gai_display_error_box(str, gtk_exit);

    if (gai_instance == NULL){
	gai_display_queued_errors();
	gtk_main();
	exit(-1);
    }
    else {
	if(!GAI.init_done){
	    gai_display_queued_errors();
	    gtk_main();
	    exit(-1);
	}
    }
}


void 
gai_display_error_continue(const char *str)
{
    g_assert(str !=NULL);

    if(gai_instance != NULL)
	GAI_NOTE (str);

    fprintf(stderr, " *** GAI Error: %s\n", str);

    if(gai_instance != NULL){
	if(GAI.init_done){
	    gai_display_error_box(str, gtk_widget_destroy);
	} 
	else 
	    gai_queue_error(str);
	
    } else
	gai_queue_error(str);

}
