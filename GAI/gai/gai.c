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
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "../config.h"
#include "gai.h"
#include "gai-private.h"


#ifdef GAI_WITH_GNOME
#include <panel-applet.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#endif

#ifdef GAI_WITH_GL
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#ifdef GAI_WITH_JOYSTICK
#include <SDL/SDL.h>
#endif


/* public */ 
GAI_struct* gai_instance = NULL;

void 
gai_is_init (void)
{
    char *msg = _("First function must be gai_init()!");

    if (gai_instance == NULL)
    {
	/* gtk_init (NULL,NULL); */
	/* recursing into a gai function when no gai_instance available... */
	/* gai_display_error("First function must be gai_init()!\n"); */
	GAI_NOTE(msg);
	gai_display_error_continue(msg);
    }
}



GtkWidget *
gai_get_drawingarea(void)
{
    GAI_CHECK;  gai_is_init();
    return GAI.drawingarea;
}

GdkWindow *
gai_get_window(void)
{
    GAI_CHECK;  gai_is_init();
    return GAI.window;
}

GdkGC *
gai_get_gc(void)
{
    GAI_CHECK;  gai_is_init();
    if(GAI.gc == NULL)
	GAI.gc = gdk_gc_new(GAI.window);

    return GAI.gc;
}

int 
gai_scale(int s)
{
    GAI_CHECK; gai_is_init();
    return (int)((float)s*GAI.scale+0.5);

}

int 
gai_get_size(void)
{
    GAI_CHECK;  gai_is_init();
#ifdef GAI_WITH_GNOME
    if(GAI.applet_type == GAI_GNOME1 || GAI.applet_type == GAI_GNOME2){
	GAI_D("size is %d\n", GAI.applet_size);
	return GAI.applet_size;
    }
    else{
#endif
	/* Always returning the smallest size in dockapp mode */
	if(GAI.height > GAI.width){
	    GAI_D("size is %d\n",GAI.width);
	    return GAI.width;
	}
	else{
	    GAI_D("size is %d\n",GAI.height);
	    return GAI.height;
	}
#ifdef GAI_WITH_GNOME
    }
#endif
}

int 
gai_applet_mode(void)
{
    GAI_CHECK; gai_is_init();
    return GAI.applet_type;
}

int 
gai_get_orient(void)
{
    GAI_CHECK; gai_is_init();
    return GAI.orient;
}

void 
gai_on_exit(int foo)
{
    if (gai_instance != NULL)
    { 
	if (! GAI.did_exit_function)
	{
	    GAI.did_exit_function = 1;
	    if (GAI.on_exit_callback)
		GAI.on_exit_callback(GAI.on_exit_userdata);
	}
	g_free (gai_instance); gai_instance = NULL;
    }
    exit(0);
}


void gai_gl_init_func(GaiCallback0 function)
{
    GAI_ENTER; gai_is_init();
#ifdef GAI_WITH_GL
    GAI.gl_init_func = function;
    GAI.open_gl = 1;
#else
    gai_display_error_quit(_("You're trying to run an applet that uses OpenGL.\n"
			   "You have not compiled with OpenGL support in GAI.\n"
			   "Please visit http://gtkglext.sf.net for GtkGlExt\n"
			   "which is required for OpenGL support.\n"));
#endif
    GAI_LEAVE;
}

#ifdef GAI_WITH_JOYSTICK
static gpointer gai_joystick_thread_func(gpointer d)
{

    int x,y;
    SDL_Joystick *joystick;

    SDL_Init(SDL_INIT_JOYSTICK);

    if(SDL_NumJoysticks() == 0)
	return NULL;

    joystick = SDL_JoystickOpen(0);


    while(1){
	GAI.jflags=0;
	SDL_JoystickUpdate();

	x = SDL_JoystickGetAxis(joystick, 0);
	y = SDL_JoystickGetAxis(joystick, 1);

	if(y < -3200)
	    GAI.jflags |= GAI_JOYSTICK_UP;

	if(y > 3200)
	    GAI.jflags |= GAI_JOYSTICK_DOWN;

	if(x < -3200)
	    GAI.jflags |= GAI_JOYSTICK_LEFT;

	if(x > 3200)
	    GAI.jflags |= GAI_JOYSTICK_RIGHT;
	    

	if(SDL_JoystickGetButton(joystick, 0))
	    GAI.jflags |= GAI_JOYSTICK_BUTTON_A;

	if(SDL_JoystickGetButton(joystick, 1))
	    GAI.jflags |= GAI_JOYSTICK_BUTTON_B;

	usleep(GAI_JOYSTICK_DELAY);
    }


    return NULL;
}
#endif

void 
gai_start(void)
{

    GAI_ENTER;  gai_is_init();

    signal(SIGTERM, gai_on_exit);

#ifdef GAI_WITH_JOYSTICK
    if(GAI.on_joystick_callback != NULL){
	if (!g_thread_supported ())
	    g_thread_init (NULL);
	g_thread_create(gai_joystick_thread_func, NULL, TRUE, NULL);
    }
#endif


#ifdef GAI_WITH_GL
    if(GAI.open_gl){
	gtk_gl_init(GAI.argc, GAI.argv);

	GAI.glconfig = gdk_gl_config_new_by_mode(GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE);

	if(GAI.glconfig == NULL){
	    GAI.glconfig = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH);
	    if(GAI.glconfig == NULL)
		gai_display_error_quit("Can't open a suiting OpenGL visual!");
	}
    }

#endif

    if(GAI.has_preferences != GAI_PREF_NONE)
	gai_menu_add(_("Preferences..."), "gtk-properties", GAI_MENU_STOCK, gai_on_preferences_activate, NULL);
    if(GAI.use_help)
	gai_menu_add(_("Help"),"gtk-help", GAI_MENU_STOCK, gai_on_help_activate, NULL);

    switch (GAI.applet_type)
    {
    case GAI_DOCKAPP:
	gai_menu_add(_("About..."),"gtk-yes", GAI_MENU_STOCK, gai_on_about_activate, NULL);
	gai_menu_add(NULL, NULL, GAI_MENU_SEPARATOR, NULL, NULL);
	gai_menu_add(_("Remove From Dock"), "gtk-remove", GAI_MENU_STOCK, gai_on_remove_activate, NULL);
	GAI.init_done = 1;
	gai_dockapp_main();
	break;
#  ifdef GAI_WITH_GNOME
    case GAI_GNOME1:
    case GAI_GNOME2:
	gai_menu_add(_("About..."),"gnome-stock-about", GAI_MENU_STOCK, gai_on_about_activate, NULL);
	GAI.init_done = 1;
	gai_gnome_main();
	break;
#  endif

#  ifdef GAI_WITH_ROX
    case GAI_ROX:
	gai_menu_add(_("About..."),"gtk-yes", GAI_MENU_STOCK, gai_on_about_activate, NULL);
	gai_menu_add(NULL, NULL, GAI_MENU_SEPARATOR, NULL, NULL);
	gai_menu_add(_("Remove from Rox Panel"), "gtk-remove", GAI_MENU_STOCK, gai_on_remove_activate, NULL);
	GAI.init_done = 1;
	gai_dockapp_main();
	break;
#  endif
#  ifdef GAI_WITH_KDE
    case GAI_KDE:
	gai_menu_add(_("About..."),"gtk-yes", GAI_MENU_STOCK, gai_on_about_activate, NULL);
	gai_menu_add(NULL, NULL, GAI_MENU_SEPARATOR, NULL, NULL);
	gai_menu_add(_("Remove from KDE Panel"), "gtk-remove", GAI_MENU_STOCK, gai_on_remove_activate, NULL);
	GAI.init_done = 1;
	gai_dockapp_main();

	break;
#endif
    default:
	break;
    }

    GAI_LEAVE;

}


static void menu_hash_free_key(int *key)
{
    g_free(key);
}

static void menu_hash_free_data(MenuHashItem *item)
{
    if(item->name != NULL)
	g_free(item->name);
    if(item->icon != NULL)
	g_free(item->icon);
    g_free(item);
}



static void 
gai_init_instance (GaiApplet *applet, const char *name, const char *version,  const char *image_path,
		   int *argc, char ***argv)
{
    char *gnome_dir = NULL;
#ifdef ENABLE_NLS
    gint i, count;
    gchar *locale_start;
#endif

    /* Make sure ~/.gnome2 exists */
    gnome_dir = g_strdup_printf("%s/.gnome2",getenv("HOME"));
    /* Make the owner have full access, and nothing else */
    mkdir(gnome_dir, S_IRWXU);
    g_free(gnome_dir);

    gai_instance = g_new0(GAI_struct, 1);



#ifdef ENABLE_NLS
    /* For GAI itself. */
    bindtextdomain(GETTEXT_PACKAGE, GAILOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);

    /* For the applet - Only for gai_init2 */
    if(applet != NULL){

	/* until next version of the API, use image path minus last two dirs */

	count = 0;
	for(i = strlen(applet->image_path)-1; i > 0 ;i--){
	    if(applet->image_path[i] == '/'){
		count++;
		if(count == 2)
		    break;
	    }
	}
	locale_start = g_malloc0(i+1);
	memcpy(locale_start, applet->image_path, i);
	GAI.locale = g_strdup_printf("%s/locale/", locale_start);
	g_free(locale_start);


	/* 
	   
	dcgettext(applet->name, msg, locale);
	*/

	/* Set it! */
	bindtextdomain(applet->name, GAI.locale);
	bind_textdomain_codeset(applet->name, "UTF-8");
	/*textdomain(applet->name);*/

    }




#endif




#ifdef GAI_WITH_GL
    GAI.argc = argc;
    GAI.argv = argv;
#endif

    /* Check if data shall be collected from applet instead */
    if(applet != NULL){

	/* applet.name has to wait */
	GAI.applet.version = g_strdup(applet->version);
	GAI.applet.nice_name = g_strdup(applet->nice_name);
	GAI.applet.author = g_strdup(applet->author);
	GAI.applet.license = g_strdup(applet->license);
	GAI.applet.description = g_strdup(applet->description);
	GAI.applet.icon = g_strdup(applet->icon);
	GAI.applet.image_path = g_strdup(applet->image_path);
	/* Just for debug */
	GAI.binfile = g_strdup(applet->name);

    } else {
	/* Guido, you're responsible for this unreadable code! :-) */
	GAI.binfile = g_strdup (( argv && *argv && **argv )? **argv 
				:( name )? name : "?gai-applet?");

	GAI.applet.image_path = NULL;
	GAI.applet.author = g_strdup ("Someone has begun to do it...\n");
	GAI.applet.description = g_strdup ("This new applet has some function,\n"
					   "it was written to fulfill a purpose.\n");
	if(name==NULL)
	    GAI.applet.nice_name = g_strdup("New applet");
	else
	    GAI.applet.nice_name = g_strdup(name);

	if(image_path!=NULL)
	    GAI.applet.image_path = g_strdup(image_path);

	GAI.applet.icon = NULL;
	GAI.applet.license = g_strdup ("Released under GNU GPL (default)");
	GAI.applet.version = g_strdup (version ? version : "0.1\n");

    }

    GAI.simple_fontmap = NULL;
    GAI.simple_context = NULL;


    GAI.default_width = GAI_DEFAULT_W;
    GAI.default_height = GAI_DEFAULT_H;
    GAI.update_interval = GAI_DEFAULT_UPDATE_INTERVAL;

    GAI.did_exit_function = 0;

    GAI.lock = 0; /* The lock is open */
    GAI.restarting = 0;		/* We're not restarting the applet */
    GAI.use_default_background = 1;
    GAI.background_has_border = 1;
    GAI.background = NULL;
    GAI.orig_background = NULL;
    GAI.draw_bg_update_done = 0;
    GAI.gc = NULL;

    /* Start-up with no icon window */
    GAI.icon_window = NULL;

    GAI.behind_applet = NULL;

    GAI.orient = GAI_HORIZONTAL;

    GAI.hide_mouse_ptr = 0;

    GAI.auto_scale = 1;
    GAI.scale = 1.0;
    GAI.timer_started = 0;

    GAI.init_done = 0;

#ifdef GAI_WITH_GL
    GAI.open_gl = 0;
    GAI.gl_init_func = NULL;
#endif

#ifdef GAI_WITH_GNOME
    GAI.gnome_started = 0;
#endif

    GAI.applet_size = GAI_DEFAULT_H;

    /* Should always be accepted */
    GAI.mask |= GDK_BUTTON_PRESS_MASK; 

    /* Menu help - Off as default */
    GAI.use_help = 0;

    /* Update as normal by default */
    GAI.freeze = FALSE;

    GAI.max_size = -1;

    GAI.parent_window = -1;

    GAI.pref_mem_usage = g_hash_table_new((GHashFunc)g_int_hash, (GEqualFunc)g_int_equal);
    GAI.has_preferences = GAI_PREF_NONE;

    GAI.menu_help_text = g_strdup (_("Sorry, no help available.\n"));

    GAI.help_text = g_strdup (_("Sorry, this new applet has no help"
			      " how to use its preference window.\n"
			      "I can only recommend trial and error.\n"));
    GAI.tooltips = NULL;
    GAI.tooltips_msg = NULL;
    GAI.about = NULL;

    GAI.rotate = 1;

    /* Set bg to default */
    GAI.bg_type = 0;
    GAI.bg_pixbuf = NULL;

    GAI.foreground_alpha = 0;


    GAI.menu_hash = g_hash_table_new_full(g_str_hash, g_str_equal, 
					  (GDestroyNotify)menu_hash_free_key, 
					  (GDestroyNotify)menu_hash_free_data);
    GAI.menu_list = NULL;
    GAI.menu_entries = 0;
    GAI.menu_changed = FALSE;



#   ifdef GAI_WITH_GNOME
    if(name !=NULL)
	gnome_program_init (name, version, LIBGNOMEUI_MODULE,
			    (*argc), (*argv), 
			    GNOME_CLIENT_PARAM_SM_CONNECT, FALSE,
			    GNOME_PARAM_NONE);
    else
	gnome_program_init (applet->name, applet->version, LIBGNOMEUI_MODULE,
			    (*argc), (*argv),
			    GNOME_CLIENT_PARAM_SM_CONNECT, FALSE, 
			    GNOME_PARAM_NONE);
#   else
    gtk_init(argc,argv);
#   endif





}


static void set_all(char *str1, char *str2, int value)
{
	    
    int j;
    char *tmp, *tmp2;
    gai_save_int(str1,value);

    for(j=0;j<gai_load_int_with_default("gai/num_applets",0);j++){
	tmp = g_strdup_printf("gai/applet%.2d",j);
	tmp2 = gai_load_string_with_default(tmp,"");
	g_free(tmp);
	tmp = g_strdup_printf(str2,tmp2);
	gai_save_int(tmp,value);
	g_free(tmp);
	g_free(tmp2);
    }

}

static void set_one(char *str, char *name, int value)
{
    char *tmp;
    tmp = g_strdup_printf(str,name);
    gai_save_int(tmp,value);
    g_free(tmp);
}


static int equal(char *str)
{
    char *cmds[] = {"--gai-list-settings",
		    "--gai-list-settings-this",
		    "--gai-debug-on",
		    "--gai-debug-on-this",
		    "--gai-debug-off",
		    "--gai-debug-off-this",
		    "--gai-broken-wm",
		    "--gai-broken-wm-this",
		    "--gai-working-wm",
		    "--gai-working-wm-this",
		    "--gai-size",
		    "--gai-size-this",
		    "--gai-help",
		    "--gai-kde",
		    "--gai-rox",
		    "--gai-gnome-server",
		    NULL};
    int i;

    for(i=0;cmds[i]!=NULL;i++){
	if(strlen(str) != strlen(cmds[i]))
	    continue;

	if(!strcmp(str, cmds[i]))
	    return i;

    }
    return -1;


}

static void show_help(void)
{

	    printf(
		"\nGAI library v%s command line options:\n"
		"\t--gai-debug-on\t\tStores debug info in /tmp/gai-debug-output\n"
		"\t\t\t\tabout all applets.\n"
		"\t--gai-debug-on-this\tStores debug info about this applet only.\n"

		"\t\t\t\tNotice that debug sometimes eats a lot of\n"
		"\t\t\t\tprocessor power.\n"
		"\t--gai-debug-off\t\tTurns off debug mode for all applets.\n"
		"\t--gai-debug-off-this\tTurns off debug mode for this applet.\n"

		"\t--gai-broken-wm\t\tLets all applets run as a normal X program.\n"
		"\t--gai-broken-wm-this\tLets this applet run as a normal X program.\n"
		"\t--gai-size\t\tSet the wished size for all GAI applet.\n" 
		"\t\t\t\tOnly for Dockapps.\n"
		"\t--gai-size-this\tSet the wished size for this applet.\n"
		"\t--gai-working-wm\tLets all applets run as\n"
		"\t\t\t\tdockapps/wmapplets.\n"
		"\t--gai-working-wm-this\tLets this applets run as\n"
		"\t\t\t\tdockapps/wmapplets.\n"

#		ifdef GAI_WITH_GNOME		
		"\t--gai-gnome-server\tDumps the current applet configuration\n"
		"\t\t\t\tinto a file in xml style. Useful for debugging.\n"
#		endif		
		"\t--gai-list-settings\tLists current global settings.\n"
		"\t--gai-list-settings-this Lists settings for this applet.\n"
		"\t--gai-help\t\tShows this text. (Strange, isn't it? ;-)\n\n",
		VERSION);
}

#define GAI_LIST_SETTINGS 0
#define GAI_LIST_SETTINGS_THIS 1
#define GAI_DEBUG_ON 2
#define GAI_DEBUG_ON_THIS 3
#define GAI_DEBUG_OFF 4
#define GAI_DEBUG_OFF_THIS 5
#define GAI_BROKEN_WM 6
#define GAI_BROKEN_WM_THIS 7
#define GAI_WORKING_WM 8
#define GAI_WORKING_WM_THIS 9
#define GAI_SIZE 10
#define GAI_SIZE_THIS 11
#define GAI_HELP 12
#define GAI_KDE_OPT 13
#define GAI_ROX_OPT 14
#define GAI_GNOME_SERVER 15

void 
gai_init_arguments (const char* name, int argc, char** argv)
{
    int i,j, num_apps;
    char *tmp, *tmp2;
    GAI_ENTER;

    /* Do this as early as possible for debug purpose. */
    GAI.applet.name = g_strdup_printf ("gai");

    /* Check for this applets setting */
    tmp = g_strdup_printf("gai/%s-broken-wm",name);
    GAI.broken_wm = gai_load_int_with_default(tmp,-1);
    if(GAI.broken_wm == -1){
	num_apps = gai_load_int_with_default("gai/num_applets",0);
	tmp2 = g_strdup_printf("gai/applet%.2d",num_apps);
	gai_save_string(tmp2,(char *)name);
	g_free(tmp2);

	num_apps++;
	gai_save_int("gai/num_applets",num_apps);

	GAI.broken_wm = gai_load_int_with_default ("gai/broken-wm",0);
	gai_save_int(tmp,GAI.broken_wm);
    }
    
    g_free(tmp);




    tmp = g_strdup_printf("gai/%s-debug",name);
    GAI.debug = gai_load_int_with_default(tmp,-1);
    if(GAI.debug == -1){
	GAI.debug = gai_load_int_with_default("gai/debug",0);
	gai_save_int(tmp,GAI.debug);
    }
    g_free(tmp);



    tmp = g_strdup_printf("gai/%s-size",name);
    GAI.scale = (float)gai_load_int_with_default(tmp, -GAI_DEFAULT_H)/(float)GAI_DEFAULT_H;

    if(GAI.scale == -1.0){
	GAI.scale = (float)gai_load_int_with_default("gai/size",GAI_DEFAULT_H)/(float)GAI_DEFAULT_H;
	gai_save_int(tmp, gai_load_int_with_default("gai/size",GAI_DEFAULT_H));
    }
    g_free(tmp);

    /* Start debug */
    gai_log_debug_init();


    /* Automagically detects if windowmaker is running */
    GAI.window_maker = gai_detect_window_maker();

    /* Default is dockapp mode */
    GAI.applet_type = gai_gnome_detect_applet_type(argc, argv);

    GAI_NOTE(name);
    for(i=1; i < argc ; i++)
    {

	j=equal(argv[i]);

	switch(j){
	case GAI_LIST_SETTINGS:
	    printf("\nGAI library v"VERSION" - Overall settings:\n\n"
		   " * Debug mode:\t\t%d\n"
		   " * Broken-wm:\t\t%d\n"
		   "\n\n",
		   gai_load_int_with_default("gai/debug",0),
		   gai_load_int_with_default("gai/broken-wm",0)
		);
	    exit(0);

	    break;
	case GAI_LIST_SETTINGS_THIS:

	    tmp = g_strdup_printf("gai/%s-debug",name);

	    printf("\nGAI library v"VERSION" - Settings for this applet:\n\n"
		   " * Debug mode:\t\t%d\n", gai_load_int_with_default(tmp,0));
	    g_free(tmp);
	    tmp = g_strdup_printf("gai/%s-broken-wm",name);
	    printf(" * Broken-wm:\t\t%d\n\n\n",gai_load_int_with_default(tmp,0));
	    g_free(tmp);
	    exit(0);

	    break;
	case GAI_DEBUG_ON:
	    set_all("gai/debug", "gai/%s-debug",1);
	    printf(_("** GAI: Debug mode ON for ALL applets."
		   " The output will be in /tmp/gai-debug-output\n"));
	    exit(0);
	    break;
	case GAI_DEBUG_ON_THIS:
	    set_one("gai/%s-debug",(char *)name,1);
	    printf(_("** GAI: Debug mode ON for THIS applets."
		   " The output will be in /tmp/gai-debug-output\n"));
	    exit(0);

	    break;
	case GAI_DEBUG_OFF:
	    set_all("gai/debug", "gai/%s-debug",0);
	    printf(_("** GAI: Debug mode OFF for ALL applets.\n"));
	    exit(0);

	    break;
	case GAI_DEBUG_OFF_THIS:
	    set_one("gai/%s-debug",(char *)name,0);
	    printf(_("** GAI: Debug mode OFF for THIS applets.\n"));
	    exit(0);


	    break;
	case GAI_BROKEN_WM:
	    set_all("gai/broken-wm", "gai/%s-broken-wm",1);
	    printf(_("** GAI: Setting broken Window Manager mode for ALL applets.\n"
		   "To turn off, use the switch --gai-working-wm\n"));
	    exit(0);

	    break;
	case GAI_BROKEN_WM_THIS:
	    set_one("gai/%s-broken-wm",(char *)name,0);
	    printf(_("** GAI: Setting broken Window Manager mode for THIS applet.\n"
		   "To turn off, use the switch --gai-working-wm-this\n"));
	    exit(0);
	    break;
	case GAI_WORKING_WM:
	    set_all("gai/broken-wm", "gai/%s-broken-wm",0);
	    printf(_("** GAI: Removing broken Window Mananger mode for ALL applets.\n"
		   "To turn on, use the switch --gai-broken-wm\n"));
	    exit(0);

	    break;
	case GAI_WORKING_WM_THIS:
	    set_one("gai/%s-broken-wm",(char *)name,0);
	    printf(_("** GAI: Removing broken Window Mananger mode for THIS applets.\n"
		   "To turn on, use the switch --gai-broken-wm-this\n"));
	    exit(0);

	    break;
	case GAI_SIZE:
	    set_all("gai/size", "gai/%s-size", atoi(argv[i+1]));
	    printf(_("** GAI: Changes the default Dockapp/wmapplet size to %d for all applets\n"
		   "Original size is 64\n"),atoi(argv[i+1]));
	    exit(0);

	    break;
	case GAI_SIZE_THIS:
	    set_one("gai/%s-size", (char *)name, atoi(argv[i+1]));
	    printf(_("** GAI: Changes the default Dockapp/wmapplet size to %d for this applet\n"
		   "Original size is 64\n"), atoi(argv[i+1]));
	    exit(0);

	    break;
#ifdef GAI_WITH_ROX
	case GAI_ROX_OPT:
	    GAI_D("i:%d argc:%d\n",i,argc);

	    /* No xid, break and go dockapp */
	    if(argc == 2)
		break;
	    GAI.applet_type = GAI_ROX;
	    GAI_D("rox window: %d\n",atoi(argv[i+1]));
	    GAI.parent_window = atoi(argv[i+1]);
	    GAI.window_maker = 0;
	    GAI.broken_wm = 1;
	    break;
#endif
#ifdef GAI_WITH_KDE
	case GAI_KDE_OPT:
	    GAI_D("i:%d argc:%d\n",i,argc);

	    /* No xid, break and go dockapp */
	    if(argc == 2)
		break;
	    GAI.applet_type = GAI_KDE;
	    GAI_D("kde window: %d\n",atoi(argv[i+1]));
	    GAI.parent_window = atoi(argv[i+1]);
	    GAI.window_maker = 0;
	    GAI.broken_wm = 1;
	    break;
#endif

	case GAI_HELP:
	    show_help();
	    exit(0);
	    break;
#ifdef GAI_WITH_GNOME
	case GAI_GNOME_SERVER:
	    gai_gnome_server_info (stdout);
	    exit(0);
	    break;
#endif
	default:
	    break;
	}
    }


    /* This is for docking help and opptions */


    if(getenv(GAI_ENV_APPLET_SIZE) !=NULL && GAI.applet_type == GAI_DOCKAPP){
	GAI.scale = ((float)atoi(getenv(GAI_ENV_APPLET_SIZE)))/((float)GAI_DEFAULT_H);
	GAI.parent_window = atoi(getenv(GAI_ENV_APPLET_XWINDOW));
	GAI.window_maker = 0;
	GAI.broken_wm = 0;
    }



    /* Free the gai_library name */
    g_free(GAI.applet.name);

    if(name == NULL)
	GAI.applet.name = g_strdup ("Test applet\n");
    else
	GAI.applet.name = g_strdup (name);

    GAI_LEAVE;
}



int 
gai_init(const char *name, const char *version, const char *image_path,
	 int *argc_p, char ***argv_p)
{
    g_assert(name !=NULL);
    g_assert(version !=NULL);
    g_assert((*argc_p) >0);
    g_assert((*argv_p) !=NULL);

    gai_init_instance(NULL, name, version, image_path, argc_p, argv_p);
    gai_init_arguments(name, (*argc_p), (*argv_p));
    return 0;
}

int gai_init2(GaiApplet *applet, int *argc_p, char ***argv_p)
{
    g_assert(applet !=NULL);
    g_assert((*argc_p) >0);
    g_assert((*argv_p) !=NULL);

    gai_init_instance(applet, NULL, NULL, NULL, argc_p, argv_p);
    gai_init_arguments(applet->name, (*argc_p), (*argv_p));
    return GAI.applet_type;

}



GdkPixbuf *
gai_load_image (const char *fname)
{

    GdkPixbuf *image;
    GError *imerr=NULL;
    char *name_buff;
    GAI_ENTER;  gai_is_init();

    g_assert(fname !=NULL);

    name_buff = g_strdup_printf ("%s/%s", GAI.applet.image_path, fname);

    image = gdk_pixbuf_new_from_file(name_buff, &imerr);
    g_free(name_buff);

    if (! image)
    {
	gai_display_error_quit(imerr->message);
	return NULL;
    }


    GAI_LEAVE;
    return image;
}

#ifdef GTK24

GdkPixbuf *gai_load_image_at_size(const char *fname, int width, int height)
{
    GdkPixbuf *image;
    char *name_buff;
    GError *imerr=NULL;
    GAI_ENTER;  gai_is_init();

    g_assert(fname != NULL);

    name_buff = g_strdup_printf ("%s/%s", GAI.applet.image_path, fname);

    image = gdk_pixbuf_new_from_file_at_size(name_buff, width,height, &imerr);

    g_free(name_buff);

    if (! image)
    {
	gai_display_error_quit(imerr->message);
	return NULL;
    }
	
    GAI_LEAVE;
    return image;
}

#else

GdkPixbuf *gai_load_image_at_size(const char *fname, int width, int height)
{

    GdkPixbuf *image, *image2;
    GAI_ENTER; gai_is_init();

    image = gai_load_image(fname);
    if(image == NULL)
	return NULL;

    image2 = gdk_pixbuf_scale_simple(image, width, height, GDK_INTERP_BILINEAR);
    g_object_unref(image);

    GAI_LEAVE;
    return image2;

}
#endif

void
gai_signal_on_exit (GaiCallback0 function, gpointer userdata)
{
    GAI_ENTER;  gai_is_init();
    g_assert(function !=NULL);
    GAI.on_exit_callback = function;
    GAI.on_exit_userdata = userdata;
    GAI_LEAVE;
}


void 
gai_signal_on_update_interval_change (int delay)
{
    GAI_ENTER;
    g_assert(delay >0);

    if (! GAI.init_done)
	gai_display_error_quit (_("You can only change the updating interval"
				" after the init stage!"));
    if (GAI.on_update_callback)
    {
	if (GAI.timer)
	{
	    GAI.update_interval = delay;
	    gtk_timeout_remove (GAI.timer);
	    GAI.timer = gtk_timeout_add(
		GAI.update_interval, gai_timer, GAI.on_update_userdata);
	}
    }
    GAI_LEAVE;
}

void 
gai_signal_on_change(GaiCallback3 *function, gpointer userdata)
{
    GAI_ENTER;  gai_is_init();
    g_assert(function != NULL);
    GAI.on_change_callback = function;
    GAI.on_change_userdata = userdata;
    GAI_LEAVE;
}


void 
gai_signal_on_mouse_move(GaiCallback2 *function, gpointer userdata)
{
    GAI_ENTER;  gai_is_init();
    g_assert(function != NULL);
    GAI.on_mouse_move_callback = function;
    GAI.on_mouse_move_userdata = userdata;
    GAI.mask |= GDK_POINTER_MOTION_MASK;
    GAI_LEAVE;
}


void 
gai_signal_on_update (GaiCallback0 *function, int interval, gpointer userdata)
{
    GAI_ENTER;  gai_is_init();
    g_assert(function != NULL);
    GAI.on_update_callback = function;
    GAI.on_update_userdata = userdata;
    GAI.update_interval = interval;
    GAI_LEAVE;
}


void 
gai_signal_on_enter (GaiCallback0 *function, gpointer userdata)
{
    GAI_ENTER;  gai_is_init();
    g_assert(function != NULL);
    GAI.mask |= GDK_ENTER_NOTIFY_MASK;
    GAI.on_enter_callback = function;
    GAI.on_enter_userdata = userdata;
    GAI_LEAVE;
}

void 
gai_signal_on_leave (GaiCallback0 *function, gpointer userdata)
{
    GAI_ENTER;  gai_is_init();
    g_assert(function != NULL);
    GAI.mask |= GDK_LEAVE_NOTIFY_MASK;
    GAI.on_leave_callback = function;
    GAI.on_leave_userdata = userdata;
    GAI_LEAVE;
}

void 
gai_signal_on_keypress (GaiCallback1 *function, gpointer userdata)
{
    GAI_ENTER;  gai_is_init();
    g_assert(function != NULL);
    GAI.mask |= GDK_KEY_PRESS_MASK;
    GAI.on_keypress_callback = function;
    GAI.on_keypress_userdata = userdata;
    GAI_LEAVE;
}


void 
gai_signal_on_scroll_buttons (GaiCallback1 *function, gpointer userdata)
{
    GAI_ENTER;  gai_is_init();
    g_assert(function != NULL);
    GAI.mask |= GDK_SCROLL_MASK;
    GAI.on_scroll_buttons_callback = function;
    GAI.on_scroll_buttons_userdata = userdata;
    GAI_LEAVE;
}

void
gai_signal_on_joystick(GaiCallback1 *function, gpointer userdata)
{
    GAI_ENTER;  gai_is_init();
    g_assert(function != NULL);
#ifdef GAI_WITH_JOYSTICK
    GAI.on_joystick_callback = function;
    GAI.on_joystick_userdata = userdata;
#endif
    GAI_LEAVE;
}

void 
gai_signal_on_mouse_button_click (GaiCallback2 *function, int button, gpointer userdata)
{
    GAI_ENTER;  gai_is_init();
    g_assert(function != NULL);
    g_assert((button == GAI_MOUSE_BUTTON_1) || (button == GAI_MOUSE_BUTTON_2));
    GAI.mask |= GDK_BUTTON_PRESS_MASK; 
    switch(button){
    case GAI_MOUSE_BUTTON_1:
	GAI.on_mouse_click1_callback = function;
	GAI.on_mouse_click1_userdata = userdata;
	break;
    case GAI_MOUSE_BUTTON_2:
	GAI.on_mouse_click2_callback = function;
	GAI.on_mouse_click2_userdata = userdata;
	break;
    default:
	gai_display_error_quit(_("Only mouse button one and two can be connected!"));
    }
    GAI_LEAVE;
}


void 
gai_signal_on_mouse_button_release(GaiCallback2 *function, int button, gpointer userdata)
{
    GAI_ENTER;  gai_is_init();

    g_assert(function != NULL);
    g_assert((button == GAI_MOUSE_BUTTON_1) || (button == GAI_MOUSE_BUTTON_2));

    GAI.mask |= GDK_BUTTON_RELEASE_MASK; 
    switch(button){
    case GAI_MOUSE_BUTTON_1:
	GAI.on_mouse_release1_callback = function;
	GAI.on_mouse_release1_userdata = userdata;
	break;
    case GAI_MOUSE_BUTTON_2:
	GAI.on_mouse_release2_callback = function;
	GAI.on_mouse_release2_userdata = userdata;
	break;
    default:
	gai_display_error_quit(_("Only mouse button one and two can be connected!"));
	break;
    }

    GAI_LEAVE;
}



void 
gai_tooltip_set(const char *msg)
{
    GtkTooltipsData *ttd;

    g_assert(msg != NULL);

    GAI_ENTER;  gai_is_init();


    if(GAI.init_done){
	if (GAI.tooltips == NULL){
	    GAI.tooltips = gtk_tooltips_new ();
	    gtk_tooltips_set_tip (GAI.tooltips, GAI.widget, msg, NULL);
	}
	else {
	    gtk_tooltips_enable (GAI.tooltips);

	    ttd = gtk_tooltips_data_get(GAI.widget);
	    g_free(ttd->tip_text);
	    ttd->tip_text = NULL;
	    gtk_tooltips_set_tip (GAI.tooltips, GAI.widget, msg, NULL);
	}
    } else {
	if(GAI.tooltips_msg != NULL)
	    g_free(GAI.tooltips_msg);
	GAI.tooltips_msg = g_strdup(msg);
    }
    GAI_LEAVE;
}


void 
gai_tooltip_remove(void)
{
    GAI_ENTER;  gai_is_init();

    if(GAI.init_done) {
	if (GAI.tooltips != NULL)
	    gtk_tooltips_disable (GAI.tooltips);
    } else {
	g_free(GAI.tooltips_msg);
	GAI.tooltips_msg = NULL;
    }
    
    GAI_LEAVE;
}

void 
gai_menu_add_help_text(const char *help_text)
{
    GAI_ENTER;  gai_is_init();
    g_assert(help_text !=NULL);
    GAI.use_help = 1;
    if (GAI.menu_help_text !=NULL)
	g_free (GAI.menu_help_text);
    GAI.menu_help_text = g_strdup(help_text);

    GAI_LEAVE;
}

static MenuHashItem *gai_menu_create_menuitem(const char *name, const char *icon, int type, void *func, void *ptr)
{
    MenuHashItem *hash_item;
    char *key;

    g_assert(type==GAI_MENU_SEPARATOR || 
	     type==GAI_MENU_STOCK || 
	     type==GAI_MENU_NONE ||
	     type==GAI_MENU_FILE);

    key = g_strdup_printf("%d", GAI.menu_entries);
 
    hash_item = g_malloc0(sizeof(MenuHashItem));

    if(name!=NULL)
	hash_item->name = g_strdup(name);

    if(icon!=NULL)
	hash_item->icon = g_strdup(icon);   
     
    hash_item->type = type;
    hash_item->func = func;
    hash_item->ptr = ptr;
    hash_item->key = key;

    return hash_item;
}

static void gai_menu_update(void)
{
    GAI.menu_changed = TRUE;

#ifdef GAI_WITH_GNOME
    if(GAI.init_done && (GAI.applet_type == GAI_GNOME2 || GAI.applet_type == GAI_GNOME1))
	gai_gnome_create_menu();
#endif

}

int
gai_menu_add(const char *name, const char *icon, int type, void *func, void *ptr)
{
    MenuHashItem *hash_item;

    GAI_ENTER; gai_is_init();

    hash_item = gai_menu_create_menuitem(name, icon, type, func, ptr);

    g_hash_table_insert(GAI.menu_hash, hash_item->key, hash_item);

    GAI.menu_list = g_slist_append(GAI.menu_list, hash_item->key);

    GAI.menu_entries++;

    gai_menu_update();

    GAI_LEAVE;
    return GAI.menu_entries;
}

void gai_menu_remove(int id)
{

    char *key;
    MenuHashItem *hash_item;

    GAI_ENTER; gai_is_init();

    g_assert(id <= GAI.menu_entries);

    key = g_strdup_printf("%d", id);

    hash_item = g_hash_table_lookup(GAI.menu_hash, key);

    if(hash_item != NULL){
	GAI.menu_list = g_slist_remove(GAI.menu_list, hash_item->key);
	g_hash_table_remove(GAI.menu_hash, hash_item->key);
	/* key and hash_item gets free'd by g_hash_table remove */
	gai_menu_update();
    }
    g_free(key);
    GAI_LEAVE;
}

int gai_menu_insert(int num, const char *name, const char *icon, int type, void *func, void *ptr)
{
    MenuHashItem *hash_item;
    GAI_ENTER; gai_is_init();

    hash_item = gai_menu_create_menuitem(name, icon, type, func, ptr);

    GAI.menu_list = g_slist_insert(GAI.menu_list, hash_item->key, num);

    GAI.menu_entries++;

    gai_menu_update();

    GAI_LEAVE;
    return GAI.menu_entries;


    
}

gboolean gai_menu_change(int id, const char *name, const char *icon, int type, void *func, void *ptr)
{

    char *key;
    MenuHashItem *hash_item;

    GAI_ENTER; gai_is_init();

    g_assert(type==GAI_MENU_SEPARATOR || 
	     type==GAI_MENU_STOCK || 
	     type==GAI_MENU_NONE ||
	     type==GAI_MENU_FILE);


    key = g_strdup_printf("%d", id);

    hash_item = g_hash_table_lookup(GAI.menu_hash, key);

    g_free(key);

    if(hash_item == NULL){
	GAI_LEAVE;
	return FALSE;
    }

    if(hash_item->name != NULL)
	g_free(hash_item->name);

    if(hash_item->icon != NULL)
	g_free(hash_item->icon);

    if(name!=NULL)
	hash_item->name = g_strdup(name);

    if(icon!=NULL)
	hash_item->icon = g_strdup(icon);   
     
    hash_item->type = type;
    hash_item->func = func;
    hash_item->ptr = ptr;

    gai_menu_update();

    GAI_LEAVE;
    return TRUE;

}

void 
gai_signal_on_preferences(GaiCallback1 *function, gpointer userdata)
{

    GAI_ENTER;  gai_is_init();
    g_assert(function !=NULL);
    GAI.has_preferences = GAI_PREF_OWN;
    GAI.on_preferences_callback = function;
    GAI.on_preferences_userdata = userdata;
    GAI.mask |= GDK_BUTTON_PRESS_MASK; 
    GAI_LEAVE;
}

void 
gai_preferences(const char *name, GaiNoteBook *gn, const char *help_text, 
		GaiCallback1 *function, gpointer userdata)
{
    GAI_ENTER;  gai_is_init();

    g_assert(name !=NULL);
    g_assert(gn !=NULL);


    GAI.has_preferences = GAI_PREF_GEN;
    GAI.on_preferences_callback = function;
    GAI.on_preferences_userdata = userdata;
    GAI.gn = gn;
    if(GAI.pref_name !=NULL)
	g_free(GAI.pref_name);
    GAI.pref_name = g_strdup(name);
    GAI.mask |= GDK_BUTTON_PRESS_MASK; 

    if(help_text !=NULL){
	if (GAI.help_text) 
	    g_free (GAI.help_text);
	GAI.help_text = g_strdup(help_text);
    }
    GAI_LEAVE;
}


void gai_preferences2(const char *name, GaiPI *pref_instr, const char *help_text, 
		GaiCallback1 *function, gpointer userdata)
{
    GAI_ENTER;  gai_is_init();

    g_assert(name !=NULL);
    g_assert(pref_instr !=NULL);


    GAI.has_preferences = GAI_PREF_GEN2;
    GAI.on_preferences_callback = function;

    GAI.on_preferences_userdata = userdata;
    GAI.pref_instr = pref_instr;
    if(GAI.pref_name !=NULL)
	g_free(GAI.pref_name);
    GAI.pref_name = g_strdup(name);
    GAI.mask |= GDK_BUTTON_PRESS_MASK; 

    if(help_text != NULL){
	if (GAI.help_text) 
	    g_free (GAI.help_text);
	GAI.help_text = g_strdup(help_text);
    }
    GAI_LEAVE;
}


void
gai_hide_mouse_ptr (void)
{
    GdkCursor *cursor;
    GdkBitmap *b;
    GdkColor fg = {0, 0, 0, 0};
    GdkColor bg = {0, 0, 0, 0};

    GAI_ENTER; gai_is_init();

    b = gdk_bitmap_create_from_data(GAI.window, "\0\0\0", 1, 1);
    cursor = gdk_cursor_new_from_pixmap(b, b, &fg, &bg, 0,0);

    gdk_window_set_cursor(GAI.window, cursor);

    gdk_cursor_unref(cursor);
    gdk_drawable_unref(b);

    GAI_LEAVE;
}


void 
gai_show_mouse_ptr(void)
{
    GAI_ENTER; gai_is_init();
    gdk_window_set_cursor (GAI.window, NULL);
    GAI_LEAVE;
}


void 
gai_flags_set(GaiFlagsType gf)
{
    GAI_ENTER;  gai_is_init();

    if(gf& GAI_FLAGS_MOUSE_PTR_HIDE){
	GAI.hide_mouse_ptr = 1;
	if(GAI.init_done)
	    gai_hide_mouse_ptr();
    }
    if(gf&GAI_FLAGS_MOUSE_PTR_SHOW){
	GAI.hide_mouse_ptr = 0;
	if(GAI.init_done)
	    gai_show_mouse_ptr();
    }

    if(gf& GAI_FLAGS_ALLOW_ROTATE)
	GAI.rotate = 1;

    if(gf& GAI_FLAGS_NEVER_ROTATE)
	GAI.rotate = 0;

    if(gf& GAI_FLAGS_FREEZE_UPDATES){
	if(GAI.init_done && !GAI.freeze){
	    gdk_window_freeze_updates(GAI.widget->window);
	    gdk_window_freeze_updates(GAI.drawingarea->window);
	}
	GAI.freeze = TRUE;
    }

    if(gf& GAI_FLAGS_THAW_UPDATES){
	if(GAI.init_done && GAI.freeze){
	    gdk_window_thaw_updates(GAI.widget->window);
	    gdk_window_thaw_updates(GAI.drawingarea->window);
	}
	GAI.freeze = FALSE;
    }
    if(gf & GAI_FLAGS_TRANSPARENT)
	GAI.transparent_bg = TRUE;
    if(gf & GAI_FLAGS_PANEL)
	GAI.panel = TRUE;


    if(gf&GAI_FLAGS_OPEN_GL_WINDOW){
#ifdef GAI_WITH_GL
	GAI.open_gl = 1;
#else
	gai_display_error_quit(_("You're trying to run an applet that uses OpenGL.\n"
			       "You have not compiled with OpenGL support in GAI.\n"
			       "Please visit http://gtkglext.sf.net for GtkGlExt\n"
			       "which is required for OpenGL support.\n"));
#endif
    }
    GAI_LEAVE;
}

GaiFlagsType 
gai_flags_get(void)
{
    GaiFlagsType gf=0;

    GAI_ENTER;  gai_is_init();

    if(GAI.hide_mouse_ptr)
	gf |= GAI_FLAGS_MOUSE_PTR_HIDE;
    else
	gf |= GAI_FLAGS_MOUSE_PTR_SHOW;

    if(GAI.rotate)
	gf |= GAI_FLAGS_ALLOW_ROTATE;
    else
	gf |= GAI_FLAGS_NEVER_ROTATE;

    if(GAI.freeze)
	gf |= GAI_FLAGS_FREEZE_UPDATES;
    else
	gf |= GAI_FLAGS_THAW_UPDATES;

    if(GAI.transparent_bg)
	gf |= GAI_FLAGS_TRANSPARENT;
    if(GAI.panel)
	gf |= GAI_FLAGS_PANEL;


#ifdef GAI_WITH_GL
    if(GAI.open_gl)
	gf |= GAI_FLAGS_OPEN_GL_WINDOW;
#endif
    GAI_LEAVE;
    return gf;

}

/* Text rendering using PangoFreeType2.
 *
 * Based upon:
 *
 * pangoft2topgm.c: Example program to view a UTF-8 encoding file
 *                  using Pango to render result.
 *
 * Copyright (C) 1999 Red Hat Software
 * Copyright (C) 2001 Sun Microsystems
 */


GdkPixbuf *
gai_text_create(const char *text, const char *font, 
		int font_size, int font_features, 
		char r, char g, char b)
{
    int height, width,i,j;
    unsigned char *buff, *buff2;
    PangoContext *context;
    PangoLayout *layout;
    PangoFontDescription *font_description;
    PangoRectangle logical_rect;
    GdkPixbuf *rendered_text;
    FT_Bitmap bitmap;

    GAI_ENTER; gai_is_init();

    g_assert(text != NULL);
    g_assert(font != NULL);
    g_assert(font_size >0);

    context = pango_ft2_get_context (100, 100);
    layout = pango_layout_new (context);
    pango_layout_set_text (layout, text, strlen(text));

    /* FIXME: Always defaulting to en_US is probably no good! */
    pango_context_set_language(context, pango_language_from_string ("en_US"));

    font_description = pango_font_description_new ();

    pango_font_description_set_family(font_description, font);
    pango_font_description_set_variant(font_description, 
				       PANGO_VARIANT_NORMAL);
    pango_font_description_set_style(font_description, 
				     font_features & GAI_TEXT_ITALIC ?
				     PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL);
    pango_font_description_set_weight(font_description, 
				      font_features & GAI_TEXT_BOLD ?
				      PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL);
    pango_font_description_set_stretch(font_description, 
				       PANGO_STRETCH_NORMAL);
    pango_font_description_set_size(font_description, 
				    font_size * PANGO_SCALE);

    pango_context_set_font_description(context, font_description);

    pango_layout_get_extents (layout, NULL, &logical_rect);
    height = PANGO_PIXELS (logical_rect.height);
    width = PANGO_PIXELS(logical_rect.width);

    buff = g_malloc0(width * height);
      
    bitmap.rows = height;
    bitmap.width = width;
    bitmap.pitch = bitmap.width;
    bitmap.buffer = buff;
    bitmap.num_grays = 256;
    bitmap.pixel_mode = ft_pixel_mode_grays;
	  
    pango_ft2_render_layout (&bitmap, layout, 0, 0);

    /* Smooth */
    if (font_features & GAI_TEXT_SMOOTH)
    {
	for(i=1;i<height-1;i++) for(j=1;j<width-1;j++)
	{
	    buff[i*width+j] = (unsigned char)
		( ((int)buff[i*width+j] + 
		   (int)buff[i*width+j-1] + 
		   (int)buff[i*width+j+1] +
		   (int)buff[(i-1)*width+j] + 
		   (int)buff[(i+1)*width+j]) / 5);
	}
    }

    /* FreeType 2 only likes 256 gray (and black), gdk_pixbuf only likes RGB,
       so we have to convert..*/

    /* This buffer shall not be deallocated, since it will built up the gdk_pixbuf */
    buff2 = g_malloc0(width*4*height);

    for(i=0;i<height;i++) 
	for(j=0;j<width;j++){
	    buff2[i*width*4+j*4+0] = r;
	    buff2[i*width*4+j*4+1] = g;
	    buff2[i*width*4+j*4+2] = b;
	    buff2[i*width*4+j*4+3] = buff[i*width+j];
	}

    g_free(buff);

    rendered_text = gdk_pixbuf_new_from_data(buff2, 
					     GDK_COLORSPACE_RGB, TRUE,8, 
					     width, height,
					     width*4, (GdkPixbufDestroyNotify)g_free, (gpointer)buff2);

    g_object_unref(context);
    g_object_unref(layout);
    pango_font_description_free(font_description);

    GAI_LEAVE;
    return rendered_text;
}

GdkPixbuf *
gai_text_create_simple(const char *text, char r, char g, char b)
{
    GdkPixbuf *rendered_text;
    PangoLayout *layout;

    int height, width,i,j;
    unsigned char *buff, *buff2;
    PangoRectangle logical_rect;
    FT_Bitmap bitmap;
    GtkStyle *style;

    GAI_ENTER;

    g_assert(text != NULL);

    if(GAI.simple_fontmap == NULL){
	GAI.simple_fontmap = (PangoFT2FontMap *)pango_ft2_font_map_new();
	GAI.simple_context = pango_ft2_font_map_create_context(GAI.simple_fontmap);
	pango_ft2_font_map_set_resolution(GAI.simple_fontmap, GAI_DPI_X, GAI_DPI_Y);
    }

    style = gtk_style_new();
    pango_context_set_font_description(GAI.simple_context, style->font_desc);
    g_object_unref(style);

    layout = pango_layout_new(GAI.simple_context);
    pango_layout_set_text(layout, text, -1);

    pango_layout_get_extents(layout, NULL, &logical_rect);
    height = PANGO_PIXELS(logical_rect.height);
    width = PANGO_PIXELS(logical_rect.width);

    buff = g_malloc0(width * height);
      
    bitmap.rows = height;
    bitmap.width = width;
    bitmap.pitch = bitmap.width;
    bitmap.buffer = buff;
    bitmap.num_grays = 256;
    bitmap.pixel_mode = ft_pixel_mode_grays;
	  
    pango_ft2_render_layout (&bitmap, layout, 0, 0);

    /* FreeType 2 only likes 256 gray (and black), gdk_pixbuf only likes RGB,
       so we have to convert..*/

    /* This buffer shall not be deallocated, since it will built up the gdk_pixbuf */
    buff2 = g_malloc0(width*4*height);

    for(i=0;i<height;i++) 
	for(j=0;j<width;j++){
	    buff2[i*width*4+j*4+0] = r;
	    buff2[i*width*4+j*4+1] = g;
	    buff2[i*width*4+j*4+2] = b;
	    buff2[i*width*4+j*4+3] = buff[i*width+j];
	}
    g_free(buff);

    rendered_text = gdk_pixbuf_new_from_data(buff2, 
					     GDK_COLORSPACE_RGB, TRUE, 8, 
					     width, height,
					     width*4, (GdkPixbufDestroyNotify)g_free, (gpointer)buff2);

    g_object_unref(layout);

    GAI_LEAVE;

    return rendered_text;

}


void gai_exec(const char *prg)
{
    GError *exec_error=NULL;

    GAI_ENTER; gai_is_init();

    /* Is there a program to execute? */
    g_assert(prg != NULL);

    /* Execute the program and not wait for it to finish */
    g_spawn_command_line_async(prg, &exec_error);

    /* If some error occured, display it and continue */
    if(exec_error!=NULL){
	gai_display_error_continue(exec_error->message);
	g_free(exec_error);
    }
    GAI_LEAVE;
}

/*
 * Local variables:
 * c-file-style: "stroustrup"
 * End:
 */
