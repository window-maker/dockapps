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
 *             Dedicated to Evelyn Reimann. - Min ss sv gp af!!
 */

#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "../config.h"
#include "gai.h"
#include "gai-private.h"

#ifdef GAI_WITH_GNOME
#include <panel-applet.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#endif

#define OAF1_SERVER  "GNOME_%sApplet"
#define OAF1_FACTORY "GNOME_%sApplet_Factory"

#define OAF2_SERVER  "GAI-%s-Applet"
#define OAF2_FACTORY "GAI-%s-Applet-Factory"


int
gai_gnome_detect_applet_type (int argc, char** argv)
{
    /* We have some command line options, let's see what it is */
	
    if (argc > 2 &&
	(strlen (argv[1]) > 30) && (strlen (argv[2]) > 12) &&
	!strncmp(argv[1], "--oaf-activate-iid=OAFIID:GAI-",30) &&
	!strncmp(argv[2], "--oaf-ior-fd=",13))
    {
#      ifndef GAI_WITH_GNOME
	gai_display_error_quit(_("You're trying to run this applet"
			       " as a Gnome panel applet\n"
			       "and there is no Gnome support"
			       " compiled in for the GAI library.\n"));
#      endif
	return GAI_GNOME2;
    }
    else if (argc > 2 &&
	(strlen (argv[1]) > 32) && (strlen (argv[2]) > 12) &&
	!strncmp(argv[1], "--oaf-activate-iid=OAFIID:GNOME_",32) &&
	!strncmp(argv[2], "--oaf-ior-fd=",13))
    {
#      ifndef GAI_WITH_GNOME
	gai_display_error_quit(_("You're trying to run this applet"
			       " as a Gnome panel applet\n"
			       "and there is no Gnome support"
			       " compiled in for the GAI library.\n"));
#      endif
	return GAI_GNOME1;
    } else {
	return GAI_DOCKAPP; 
    }
}

/* This function is used both by Gnome and Rox */
void gai_size_change(int default_size, int curr_w, int curr_h, int gai_size_init, int gnome_steals)
{
    static int old_width = -1, old_height = -1, old_orient = -1;

    if(GAI.lock){
	GAI_D("LOCK! - refuse changing(%d)\n", default_size);
	GAI_LEAVE;
	return;
    }
    GAI.lock = 1;

    if(GAI.orient == GAI_VERTICAL){
	/* Vertical - Standing */
	if(gai_size_init)
	    GAI.width = default_size;
	else
	    GAI.width  = curr_w;

	if(!GAI.rotate){
	    GAI.height = GAI.width * GAI.default_height/GAI.default_width;
	    if(GAI.max_size != -1 && GAI.max_size < GAI.height){
		GAI.height = GAI.max_size;
		GAI.width = GAI.height * GAI.default_width/GAI.default_height;
	    }
	}

	else {
	    if(GAI.max_size != -1 && GAI.max_size < GAI.width)
		GAI.width = GAI.max_size;

	    GAI.height = GAI.width * GAI.default_width/GAI.default_height;
	}

	GAI.applet_size = GAI.width;
	GAI.scale = (double)GAI.width/(double)GAI.default_width;

    } else {
	/* Horizontal - lying down */
	if(gai_size_init)
	    GAI.height = default_size;
	else
	    GAI.height  = curr_h;

	if(GAI.max_size != -1 && GAI.max_size < GAI.height)
	    GAI.height = GAI.max_size;

	GAI.width = GAI.height * GAI.default_width/GAI.default_height;

	GAI.applet_size = GAI.height;
	GAI.scale = (double)GAI.height/(double)GAI.default_height;
    }

    if(gai_size_init || old_orient != GAI.orient ||
       (GAI.orient == GAI_HORIZONTAL && (GAI.width != old_width || GAI.height != old_height)) ||
       (GAI.orient == GAI_VERTICAL && (GAI.height != old_width || GAI.width != old_height))){

	GAI_D("Set size to %d, %d (%d %d)\n", GAI.width+gnome_steals, GAI.height+gnome_steals, old_width, old_height);

	gtk_widget_set_size_request(GAI.drawingarea,
				    GAI.width+gnome_steals,
				    GAI.height+gnome_steals);
	if(GAI.widget)
	    gtk_widget_set_size_request(GAI.widget,
					GAI.width+gnome_steals,
					GAI.height+gnome_steals);

	if (GAI.on_change_callback ) /*&& widget)*/
	    GAI.on_change_callback(GAI.orient, GAI.width, GAI.height, 
				   GAI.on_change_userdata);


	if(GAI.orient == GAI_VERTICAL){
	    old_height = GAI.width;
	    old_width = GAI.height;
	} else {
	    old_height = GAI.height;
	    old_width = GAI.width;
	}
	old_orient = GAI.orient;

	gai_draw_update_bg();

    }
    GAI.lock = 0;


}



#ifdef GAI_WITH_GNOME

void gai_gnome_create_menu(void)
{
    int i, j=0;
    MenuHashItem *item;
    char *tmp = NULL;

    if(GAI.menu_verb != NULL)
	g_free(GAI.menu_verb);

    if(GAI.menu_xml != NULL)
	g_free(GAI.menu_xml);

    GAI.menu_verb = g_malloc0(sizeof(BonoboUIVerb) * g_slist_length(GAI.menu_list)+1);

    GAI.menu_xml = g_strdup("<popup name=\"button3\">\n");


    GAI_D("Number of gai entries:%d",g_slist_length(GAI.menu_list));

    for(i=0;i<g_slist_length(GAI.menu_list);i++){

	item = g_hash_table_lookup(GAI.menu_hash, g_slist_nth_data(GAI.menu_list, i));

	if(item->type  == GAI_MENU_STOCK || item->type  == GAI_MENU_NONE || item->type  == GAI_MENU_FILE ){

	    GAI.menu_verb[j].cname = g_strdup_printf("%.4d",i);
	    GAI.menu_verb[j].cb = (BonoboUIVerbFn)item->func;
	    GAI.menu_verb[j].user_data = NULL;


	    if(item->type  == GAI_MENU_STOCK){

		tmp = g_strdup_printf("%s<menuitem name=\"%.4d\" verb=\"%.4d\" \n"
			"_label=\"%s\" pixtype=\"stock\" pixname=\"%s\"/>\n",GAI.menu_xml,i,i,item->name, item->icon);
	    }

	    if(item->type  == GAI_MENU_NONE){
		tmp = g_strdup_printf("%s<menuitem name=\"%.4d\" verb=\"%.4d\" \n"
			"_label=\"%s\"/>\n", GAI.menu_xml, i, i, item->name);
	    }

	    if(item->type  == GAI_MENU_FILE){

		tmp = g_strdup_printf("%s<menuitem name=\"%.4d\" verb=\"%.4d\" \n"
				      "_label=\"%s\" pixtype=\"filename\" pixname=\"%s/%s\"/>\n", GAI.menu_xml, i, i,
				      item->name, GAI.applet.image_path, item->icon);

	    }
	    g_free(GAI.menu_xml);
	    GAI.menu_xml = tmp;
	    j++;
	}

	if(item->type == GAI_MENU_SEPARATOR){
	    tmp = g_strdup_printf("%s<separator/>\n", GAI.menu_xml);
	    g_free(GAI.menu_xml);
	    GAI.menu_xml = tmp;
	}


    }

    tmp = g_strdup_printf("%s</popup>\n", GAI.menu_xml);
    g_free(GAI.menu_xml);
    GAI.menu_xml = tmp;

    GAI_NOTE(GAI.menu_xml);
    GAI.menu_changed = FALSE;



    panel_applet_setup_menu(PANEL_APPLET(GAI.widget), 
			    GAI.menu_xml, 
			    GAI.menu_verb, NULL);

}


static void gai_gnome_handle_background(void)
{
    GdkPixmap *pixmap;

    GAI_ENTER;
    GAI.bg_type = panel_applet_get_background(
	PANEL_APPLET(GAI.widget), &GAI.bg_colour, &pixmap);

    if(GAI.bg_type == PANEL_PIXMAP_BACKGROUND) GAI_NOTE("Background: pixmap\n");
    if(GAI.bg_type == PANEL_NO_BACKGROUND) GAI_NOTE("Background: no\n");
    if(GAI.bg_type == PANEL_COLOR_BACKGROUND) GAI_NOTE("Background: colour\n");

    
#  ifdef GNOME_24
    if(GAI.bg_type == PANEL_PIXMAP_BACKGROUND)
    {
	GAI_NOTE("panel pixmap bg");
	/* Copy the full pixmap and make a pixbuf of it.*/
	GAI.bg_pixbuf = gdk_pixbuf_get_from_drawable (
	    NULL, pixmap, NULL, 0,0,0,0,-1,-1);
	GAI_INT(gdk_pixbuf_get_width (GAI.bg_pixbuf));
	GAI_INT(gdk_pixbuf_get_height (GAI.bg_pixbuf));
	
	/* Free the pixmap ?????*/
	g_object_unref(pixmap);
    }
#  endif


    if(!GAI.gnome_started){
	if(GAI.use_default_background)
	    gai_load_background(); /* Just to be sure */
	else
	    gai_draw_update_bg();
    } else
	gai_load_background();

    GAI_LEAVE;
}

int
gai_gnome_change_size (GtkWidget *widget, int size, gpointer dataptr)
{
    static int  gai_size_init = 1;

    GAI_ENTER;

    GAI_D("REQUEST: change_size: %d w:%d h:%d (%d %d) - %d\n",size,GAI.width, GAI.height,
	  GAI.drawingarea->allocation.width,
	  GAI.drawingarea->allocation.height,
	  panel_applet_get_size(PANEL_APPLET(GAI.widget)));

    /* This is an ugly hack to make changing bg in gnome work ok */
    if(size == -1) gai_size_init = 1;

    gai_size_change(panel_applet_get_size(PANEL_APPLET(GAI.widget)),
		    GAI.drawingarea->allocation.width,
		    GAI.drawingarea->allocation.height, 
		    gai_size_init,
		    GNOME_STEALS);

    gai_size_init = 0;


    GAI_LEAVE;
    return TRUE;
}

static int
gai_gnome_change_background(PanelApplet *widget,
			    PanelAppletBackgroundType type,
			    GdkColor *colour,
			    GdkPixmap *pixmap)
{
    GAI_ENTER;
    if(GAI.timer_started){
	gai_gnome_handle_background();
	if (GAI.on_change_callback)
	    GAI.on_change_callback(GAI.orient, GAI.width, GAI.height, 
				   GAI.on_change_userdata);
    }
    GAI_LEAVE;
    return TRUE;
}

static int 
gai_gnome_change_orient(PanelApplet *widget, 
			PanelAppletOrient orient, gpointer dataptr)
{
    GAI_ENTER;
    if (orient == PANEL_APPLET_ORIENT_LEFT ||                         
	orient == PANEL_APPLET_ORIENT_RIGHT)
	GAI.orient=GAI_VERTICAL;
    else
	GAI.orient=GAI_HORIZONTAL;

    if (widget != NULL)
	gai_gnome_change_size(GTK_WIDGET(widget), -1, dataptr);
    
    GAI_LEAVE;
    return TRUE;
}
static int
gai_gnome_expose(GtkWidget *widget, GdkEventExpose *event, gpointer d)
{
    static gboolean expose_lock = FALSE;
    GAI_ENTER;

    if(expose_lock){
	GAI_NOTE("Expose lock");
	return FALSE;
    }
    expose_lock = TRUE;

    GAI_D("REQUEST: expose_size: %d %d (%d %d)\n", event->area.height, event->area.width,
	  GAI.drawingarea->allocation.width,
	  GAI.drawingarea->allocation.height);

    GAI_D("size: %d %d\n", GAI.width, GAI.height);
    if(GAI.width == GAI.drawingarea->allocation.width && GAI.height == GAI.drawingarea->allocation.height)
	goto leave_expose;



    if(GAI.orient == GAI_HORIZONTAL)
	gai_size_change(event->area.height,
			GAI.drawingarea->allocation.width,
			GAI.drawingarea->allocation.height, 
			FALSE, 0);
    else
	gai_size_change(event->area.width,
			GAI.drawingarea->allocation.width,
			GAI.drawingarea->allocation.height, 
			FALSE, 0);


 leave_expose:
    GAI_LEAVE;
    expose_lock = FALSE;
    return TRUE;
}

static int
gai_gnome_realize(GtkWidget *widget, gpointer d)
{
    GAI_ENTER;

    if(!GTK_WIDGET_REALIZED(GAI.drawingarea))
	return TRUE;
/*
    GAI_D("realize size: %d %d\n", GAI.drawingarea->allocation.width, GAI.drawingarea->allocation.height);
    GAI_D("realize size: %d %d\n", GAI.drawingarea->requisition.width, GAI.drawingarea->requisition.height);
*/
    GAI_LEAVE;
    return TRUE;
}


static int 
gai_gnome_start(PanelApplet *widget)
{
#ifdef GAI_WITH_GL
/*    GdkGLContext *glcontext;
      GdkGLDrawable *gldrawable;*/
#endif

    GAI_ENTER;

    GAI.widget = GTK_WIDGET(widget);

    gtk_widget_realize (GAI.widget);
    GAI.drawingarea = gtk_drawing_area_new();

    gtk_container_add (GTK_CONTAINER(GAI.widget), GAI.drawingarea);

    gtk_widget_realize (GAI.drawingarea);


#ifdef GAI_WITH_GL
    if(GAI.open_gl)
	gai_display_error_quit(_("You're trying to run an applet that uses OpenGL.\n"
			       "Open GL doesn't work on the Gnome Panel. Or it is\n"
			       "a bug in the GAI library. Which ever it is,\n"
			       "I'm sorry..\n"));

/*    if(GAI.open_gl){

	glcontext = gtk_widget_get_gl_context (GAI.drawingarea);
	gldrawable = gtk_widget_get_gl_drawable (GAI.drawingarea);

	while(!gdk_gl_drawable_gl_begin (gldrawable, glcontext)){
	    usleep(10000);
	}



	if(GAI.gl_init_func)
	    GAI.gl_init_func(NULL);

	gdk_gl_drawable_gl_end (gldrawable);

	}*/
#endif 



    GAI.window = GAI.drawingarea->window;

    if(GAI.gc != NULL)
	g_object_unref(GAI.gc);
    GAI.gc = gdk_gc_new(GAI.window);
    
    gai_gnome_change_orient (NULL, 
			     panel_applet_get_orient(PANEL_APPLET(GAI.widget)), NULL);

    gai_gnome_change_size (NULL, panel_applet_get_size(PANEL_APPLET(GAI.widget)), NULL);    
    gai_gnome_handle_background();

    GAI_D("allocted size: %d %d\n", GAI.widget->allocation.width,GAI.widget->allocation.height);

/*    gtk_widget_set_size_request(GAI.drawingarea,
				GAI.width+GNOME_STEALS,
				GAI.height+GNOME_STEALS);


    gtk_widget_set_size_request(GAI.widget,
				GAI.width+GNOME_STEALS,
				GAI.height+GNOME_STEALS);

*/


    g_signal_connect(G_OBJECT (GAI.widget), "delete_event",
		     G_CALLBACK(gai_dies), NULL);

    g_signal_connect(G_OBJECT(GAI.widget), "destroy",
		     G_CALLBACK(gai_dies), NULL);
/*
    g_signal_connect(G_OBJECT(GAI.widget), "change_size",
		     G_CALLBACK(gai_gnome_change_size),NULL);
*/

    g_signal_connect(G_OBJECT(GAI.widget), "expose-event",
		     G_CALLBACK(gai_gnome_expose),NULL);

    g_signal_connect(G_OBJECT(GAI.widget), "realize",
		     G_CALLBACK(gai_gnome_realize),NULL);

    g_signal_connect(G_OBJECT(GAI.drawingarea), "size-allocate",
		     G_CALLBACK(gai_gnome_change_size),NULL);

    g_signal_connect(G_OBJECT(GAI.widget), "change_orient",
		     G_CALLBACK(gai_gnome_change_orient),NULL);

    g_signal_connect(G_OBJECT(GAI.widget), "change_background",
		     G_CALLBACK(gai_gnome_change_background), NULL);


    gai_gnome_create_menu();


    if(GAI.orient == GAI_HORIZONTAL){
	GAI_D("default_height:%d, panel:%d\n",GAI.default_height, panel_applet_get_size(PANEL_APPLET(GAI.widget)));
	if(GAI.default_height < panel_applet_get_size(PANEL_APPLET(GAI.widget))){
	    GAI_NOTE("None");
	    panel_applet_set_flags(PANEL_APPLET(GAI.widget),
				   PANEL_APPLET_FLAGS_NONE);
	} else {
	    GAI_NOTE("Minor");
	    panel_applet_set_flags(PANEL_APPLET(GAI.widget),
				   PANEL_APPLET_EXPAND_MINOR);
	}
    } else {
	GAI_D("default_width:%d, panel:%d\n",GAI.default_width, panel_applet_get_size(PANEL_APPLET(GAI.widget)));
	if(GAI.default_width < panel_applet_get_size(PANEL_APPLET(GAI.widget))){
	    GAI_NOTE("None");
	    panel_applet_set_flags(PANEL_APPLET(GAI.widget),
				   PANEL_APPLET_FLAGS_NONE);
	} else {
	    GAI_NOTE("Minor");
	    panel_applet_set_flags(PANEL_APPLET(GAI.widget),
				   PANEL_APPLET_EXPAND_MINOR);
	}
    }



    if (GAI.hide_mouse_ptr)
	gai_hide_mouse_ptr();
    else
	gai_show_mouse_ptr();




/*    if(GAI.default_height > GAI.height || GAI.default_width > GAI.width){
	panel_applet_set_flags (PANEL_APPLET(GAI.widget),
				PANEL_APPLET_EXPAND_MINOR);
    } else {
	panel_applet_set_flags (PANEL_APPLET(GAI.widget),
				PANEL_APPLET_FLAGS_NONE);
				}*/

    gai_hook();

    gtk_widget_show_all(GAI.widget);


//    g_free(gnome_menu_xml);
//    g_free(gnome_menu_verb);


    GAI.gnome_started = 1;

    /* Freeze updates, if requested */
    if(GAI.freeze){
	gdk_window_freeze_updates(GAI.widget->window);
	gdk_window_freeze_updates(GAI.drawingarea->window);
    }


    /* Start with updating */
    if (GAI.on_update_callback)
	GAI.on_update_callback(GAI.on_update_userdata);

    GAI_LEAVE;
    return TRUE;

}

static int 
gai_gnome_factory (PanelApplet *applet, const gchar *id, gpointer d)
{
    int retval = FALSE;
    char *iid;
    static int gnome_instance_running=0;


    GAI_ENTER;
    
    iid = g_strdup_printf(GAI.applet_type == GAI_GNOME1 ?
			  "OAFIID:"OAF1_SERVER : "OAFIID:"OAF2_SERVER, 
			  GAI.applet.name);  
    GAI_NOTE (iid);
    if (!strcmp (iid, id))
    {
	GAI_CHECK;
	if(!gnome_instance_running){
	    retval = gai_gnome_start (applet);
	    gnome_instance_running = 1;
	}
	else {
	    gai_display_error_continue(_("Sorry, it's not possible to run\n"
		                       "two instances of the same applet.\n"));
	    return TRUE;
	}
    }else{
	GAI_NOTE(iid);
    }
    g_free(iid);
    GAI_LEAVE;
    return retval;

}



void 
gai_gnome_main(void)
{
    char *iid;
    GAI_ENTER;
    /* GAI = gai_get_struct_ptr(); */

    iid = g_strdup_printf (GAI.applet_type == GAI_GNOME1 ?
			   "OAFIID:"OAF1_FACTORY : "OAFIID:"OAF2_FACTORY, 
			   GAI.applet.name);
    GAI_NOTE(iid);
    panel_applet_factory_main (iid, PANEL_TYPE_APPLET, 
			       gai_gnome_factory, NULL);
    g_free (iid);
    GAI_LEAVE;
}



GdkPixbuf *
gai_rotate(GdkPixbuf *in)
{
    int alpha,h,w,rs;

    int i,j,ypos;
    unsigned char *rot_buff, *pixs;
    GdkPixbuf *rot_pic;
    GAI_ENTER;

    alpha = gdk_pixbuf_get_has_alpha (in);
    w = gdk_pixbuf_get_width (in);
    h = gdk_pixbuf_get_height (in);
    rs = gdk_pixbuf_get_rowstride (in);
    pixs = gdk_pixbuf_get_pixels (in);

    rot_buff = g_malloc0 ((h+5) * w * (3+alpha));

    if (alpha)
    {
	for (i=0; i < h ; i++) 
	{
	    ypos = i * rs;
	    for (j=0; j < w; j++)
	    {
		rot_buff[j*h*4 + (h-i-1)*4 +0]=pixs[ypos + j*4 +0];
		rot_buff[j*h*4 + (h-i-1)*4 +1]=pixs[ypos + j*4 +1];
		rot_buff[j*h*4 + (h-i-1)*4 +2]=pixs[ypos + j*4 +2];
		rot_buff[j*h*4 + (h-i-1)*4 +3]=pixs[ypos + j*4 +3];
	    }
	}
    }
    else
    {
	for (i=0; i < h ; i++)
	{
	    ypos = i * rs;
	    for (j=0; j < w ; j++)
	    {
		rot_buff[j*h*3 + (h-i-1)*3 +0]=pixs[ypos + j*3+0];
		rot_buff[j*h*3 + (h-i-1)*3 +1]=pixs[ypos + j*3+1];
		rot_buff[j*h*3 + (h-i-1)*3 +2]=pixs[ypos + j*3+2];
	    }
	}
    }

    rot_pic = gdk_pixbuf_new_from_data(rot_buff,GDK_COLORSPACE_RGB,
				       alpha, 8, h, w, h*(3+alpha),
				       (GdkPixbufDestroyNotify)g_free, (gpointer)rot_buff);
    GAI_LEAVE;
    return rot_pic;

}

void gai_gnome_about_show (void)
{
    char *authors[3];
    GdkPixbuf *pixbuf;
    GAI_ENTER;

    if (GAI.about!=NULL) 
    {
	gtk_window_present(GTK_WINDOW(GAI.about));
	GAI_LEAVE;
	return;
    }

    authors[0] = GAI.applet.author;
    authors[1] = g_strdup(_("\nThis applet uses the GAI library\n - http://gai.sourceforge.net - \n"));
    authors[2] = NULL;

    pixbuf = gdk_pixbuf_new_from_file(GAI.applet.icon, NULL);
    GAI_NOTE(GAI.applet.icon);

    GAI.about = gnome_about_new(
	GAI.applet.nice_name,GAI.applet.version,
	GAI.applet.license,
	GAI.applet.description,
	(const char**)authors,
	NULL, NULL, pixbuf);

    if(pixbuf!=NULL)
	g_object_unref(pixbuf);

    g_signal_connect (G_OBJECT (GAI.about), "destroy",
		      G_CALLBACK(gtk_widget_destroyed),
		      &GAI.about);
    gtk_widget_show(GAI.about);

    g_free(authors[1]);
    GAI_LEAVE;
}


void gai_gnome_server_info (FILE* out)
{
    fprintf (out, "<oaf_info>\n");
    fprintf (out,"   <oaf_server type=\"exe\"\n");
    fprintf (out, ( GAI.applet_type == GAI_GNOME1 ) 
	     ? "   iid=\"OAFIID:"OAF1_FACTORY"\"\n"
	     : "   iid=\"OAFIID:"OAF2_FACTORY"\"\n",  GAI.applet.name);
    fprintf (out,"     location=\"%s\"> \n", GAI.binfile);
    fprintf (out,"     <oaf_attribute name=\"repo_ids\" type=\"stringv\">\n");
    fprintf (out,"       <item value=\"IDL:Bonobo/GenericFactory:1.0\"/>\n");
    fprintf (out,"       <item value=\"IDL:Bonobo/Unknown:1.0\"/>\n");
    fprintf (out,"     </oaf_attribute>\n");
    fprintf (out,"     <oaf_attribute name=\"name\" type=\"string\"\n");
    fprintf (out,"       value=\"%s Applet Factory\"/> \n", GAI.applet.name);
    fprintf (out,"     <oaf_attribute name=\"description\" type=\"string\"\n");
    fprintf (out,"       value=\"Factory For The %s Applet\"/> \n",
	     GAI.applet.nice_name);
    fprintf (out,"      </oaf_server> \n");
    fprintf (out," \n");
    fprintf (out,"   <oaf_server type=\"factory\"\n");
    fprintf (out, ( GAI.applet_type == GAI_GNOME1 ) 
	     ? "   iid=\"OAFIID:"OAF1_SERVER"\"\n"
	     : "   iid=\"OAFIID:"OAF2_SERVER"\"\n", GAI.applet.name);
    fprintf (out, ( GAI.applet_type == GAI_GNOME1 ) 
	     ? "   location=\"OAFIID:"OAF1_FACTORY"\">\n"
	     : "   location=\"OAFIID:"OAF2_FACTORY"\">\n", GAI.applet.name);
    fprintf (out,"   <oaf_attribute name=\"repo_ids\" type=\"stringv\">\n");
    fprintf (out,"     <item value=\"IDL:GNOME/Vertigo/PanelAppletShell:1.0\"/>\n");
    fprintf (out,"     <item value=\"IDL:Bonobo/Control:1.0\"/>\n");
    fprintf (out,"     <item value=\"IDL:Bonobo/Unknown:1.0\"/>\n");
    fprintf (out,"   </oaf_attribute> \n");
    fprintf (out,"   <oaf_attribute name=\"name\" type=\"string\"");
    fprintf (out,"      value=\"%s\"/> \n", GAI.applet.name);
    fprintf (out,"   <oaf_attribute name=\"description\" type=\"string\"");
    fprintf (out,"      value=\"%s\"/> \n", GAI.applet.nice_name);
    fprintf (out,"   <oaf_attribute name=\"panel:category\" type=\"string\"");
    fprintf (out,"       value=\"%s\"/> \n", GAI.applet.version);
    fprintf (out,"   <oaf_attribute name=\"panel:icon\" type=\"string\"");
    fprintf (out,"       value=\"%s\"/> \n", GAI.applet.icon);
    fprintf (out,"   </oaf_server> \n");
    fprintf (out,"</oaf_info> \n");
}

/* GAI_WITH_GNOME */
#endif

/*
 * Local variables:
 * c-file-style: "stroustrup"
 * End:
 */
