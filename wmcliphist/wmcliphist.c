/*
 * (c) 2001 Michal Krause <michal@krause.cz>
 */

#include "wmcliphist.h"

#define WMCLIPHIST_VERSION "2.1"

/*
 * print some help
 */
void
print_help()
{
	begin_func("print_help");

	fprintf(stderr, "Usage: wmcliphist [options]\n\n"
			"wmcliphist is small dock applet for Window Maker which "
			"keeps X clipboard history\n\n");
	fprintf(stderr, "-h         show this help\n"
			"-v         print version\n"
			"-n <num>   set number of items to keep (default 10)\n"
			"-c color   set color for locked items (default is red)\n"
			"-s <size>  choose wmcliphist icon size:\n"
 			"            0 = no icon\n"
			"           16 = tiny 16x16 px icon\n"
			"           30 = icon suitable for 32px dock/slit\n"
			"           40 = icon suitable for 48px dock/slit\n"
			"           60 = icon suitable for 64px dock/slit (default)\n");
	fprintf(stderr, "-i <num>   choose wmcliphist icon antialiasing:\n"
			"           0 = for mid tones background (default)\n"
			"           1 = for dark background\n"
			"           2 = for light background\n");
	fprintf(stderr, "-d         dumps clipboard history to stdout"
			" in pseudo XML format (no escaping etc.)\n");
	fprintf(stderr, "-b <type>  choose clipboard to manage\n"
			"           PRIMARY   = select copies, middle click pastes (default)\n"
			"           SECONDARY = not used\n"
			"           CLIPBOARD = Ctrl+C copies, Ctrl+V pastes\n\n");
	exit(1);
	return_void();
}

static void
wmcliphist_exit(gint code)
{
	begin_func("wmcliphist_exit");
	exit(code);
	return_void();
}

/* gtk3 dockapp code based on wmpasman by Brad Jorsch
 * <anomie@users.sourceforge.net>
 * http://sourceforge.net/projects/wmpasman */

GtkWidget *foo_create_main_icon_window(GtkWidget *mw, unsigned int s)
{
	Display *d;
	GdkDisplay *display;
	GtkWidget *foobox;
	unsigned int dummy3;
	Window mainwin, iw, p, dummy1, *dummy2, w;
	XWMHints *wmHints;

	display = gdk_display_get_default();
	foobox = gtk_window_new(GTK_WINDOW_POPUP);

	gtk_window_set_wmclass(GTK_WINDOW(mw), g_get_prgname(), "DockApp");
	gtk_widget_set_size_request(foobox, s, s);

	gtk_widget_realize(mw);
	gtk_widget_realize(foobox);

	d = GDK_DISPLAY_XDISPLAY(display);
	mainwin = GDK_WINDOW_XID(gtk_widget_get_window(mw));
	iw = GDK_WINDOW_XID(gtk_widget_get_window(foobox));
	XQueryTree(d, mainwin, &dummy1, &p, &dummy2, &dummy3);
	if (dummy2)
		XFree(dummy2);
	w = XCreateSimpleWindow(d, p, 0, 0, 1, 1, 0, 0, 0);
	XReparentWindow(d, mainwin, w, 0, 0);
	gtk_widget_show(mw);
	gtk_widget_show(foobox);
	wmHints = XGetWMHints(d, mainwin);
	if (!wmHints)
		wmHints = XAllocWMHints();
	wmHints->flags |= IconWindowHint;
	wmHints->icon_window = iw;
	XSetWMHints(d, mainwin, wmHints);
	XFree(wmHints);
	XReparentWindow(d, mainwin, p, 0, 0);
	XDestroyWindow(d, w);

	return foobox;
}

/*
 * main func
 */
int
main(int argc, char **argv)
{
	gint	i = 1, res;
	gchar	*arg;
	gchar   *icon_file;
	GList	*list_node;
	int	icon_number = 0;
	int	icon_size = 60;
	gboolean dump_only = FALSE;

#ifdef	FNCALL_DEBUG
	debug_init_nothreads();
#endif
	begin_func("main");

	/* load configuration */
	if ((res = rcconfig_get(rcconfig_get_name("rc"))) != 0) {
		fprintf(stderr, "~/.wmcliphistrc parse error (%d)\n", res);
		return_val(1);
	}

	/* parse command line */
	while ((arg = argv[i])) {
		if (*arg == '-') {
			if (*(arg + 1) == 'h')
				print_help();
			else if (*(arg + 1) == 'n') {
				i++;
				if (!argv[i]) {
					fprintf(stderr, "Missing value of -n\n");
					print_help();
				}
				num_items_to_keep = atol(argv[i]);
			} else if (*(arg + 1) == 'c') {
				i++;
				if (!argv[i]) {
					fprintf(stderr, "Missing value of -c\n");
					print_help();
				}
				memset(locked_color_str, 0, 32);
				strncpy(locked_color_str, argv[i], 31);
			} else if (*(arg + 1) == 'i') {
				i++;
				if (!argv[i]) {
					fprintf(stderr, "Missing value of -i\n");
					print_help();
				}
				icon_number = atoi(argv[i]);
				if (icon_number < 0 || icon_number > 2) {
					fprintf(stderr, "Invalid value of -i\n");
					print_help();
				}
			} else if (*(arg + 1) == 's') {
				i++;
				if (!argv[i]) {
					fprintf(stderr, "Missing value of -s\n");
					print_help();
				}
				icon_size = atoi(argv[i]);
				if (icon_size != 60 && icon_size != 40
						&& icon_size != 30
						&& icon_size != 16
						&& icon_size != 0) {
					fprintf(stderr, "Invalid value of -s\n");
					print_help();
				}
				if (icon_size == 0) {
					icon_size = 1;
				}
			} else if (*(arg + 1) == 'd') {
				dump_only = TRUE;
			} else if (*(arg + 1) == 'v') {
				printf("wmcliphist "WMCLIPHIST_VERSION"\n");
				exit(1);
			} else if (*(arg + 1) == 'b') {
				i++;
				if (!argv[i]) {
					fprintf(stderr, "Missing value of -b\n");
					print_help();
				}
				memset(clipboard_str, 0, 32);
				strncpy(clipboard_str, argv[i], 31);
			} else {
				fprintf(stderr, "Invalid option -%c\n", *(arg + 1));
				print_help();
			}
		} else {
			fprintf(stderr, "Invalid option %s\n", arg);
			print_help();
		}
		i++;
	}

	signal(SIGCHLD, SIG_IGN);

	/* initialize Gtk */
	gtk_init(&argc, &argv);


	/* create main window */
	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	/* creat dock icon */
	dock_app = foo_create_main_icon_window(main_window, icon_size);

	if (icon_size) {
		/* create icon_mask */
		if (icon_size == 60) {
			/* 60x60 icon */
			if (icon_number == 0) {
				icon_file = "ico_60x60_gray.png";
			} else if (icon_number == 1) {
				icon_file = "ico_60x60_black.png";
			} else {
				icon_file = "ico_60x60_white.png";
			}
		} else if (icon_size == 40) {
			/* 40x40 icon */
			/* create icon */
			if (icon_number == 0) {
				icon_file = "ico_40x40_gray.png";
			} else if (icon_number == 1) {
				icon_file = "ico_40x40_black.png";
			} else {
				icon_file = "ico_40x40_white.png";
			}
		} else if (icon_size == 30) {
			/* 30x30 icon */
			/* create icon */
			if (icon_number == 0) {
				icon_file = "ico_30x30_gray.png";
			} else if (icon_number == 1) {
				icon_file = "ico_30x30_black.png";
			} else {
				icon_file = "ico_30x30_white.png";
			}
		} else {
			/* 16x16 icon */
			/* create icon */
			icon_file = "ico_16x16.png";
		}

		icon_file = g_strconcat(DATADIR"/", icon_file, NULL);
		pixmap = gtk_image_new_from_file(icon_file);
		gtk_widget_show(pixmap);
		gtk_container_add(GTK_CONTAINER(dock_app), pixmap);
	}


	/* create clipboard history menu */
	menu_hist = gtk_menu_new();
	menu_title = gtk_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_hist), menu_title);
	gtk_widget_show(menu_title);
	gtk_widget_show(menu_hist);


	/* create application menu */
	menu_app = gtk_menu_new();

	menu_app_clip_lock = gtk_check_menu_item_new_with_label("Clipboard lock");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_app), menu_app_clip_lock);

	menu_app_clip_ignore = gtk_check_menu_item_new_with_label("Clipboard ignore");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_app), menu_app_clip_ignore);

	menu_app_save = gtk_menu_item_new_with_label("Save history");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_app), menu_app_save);
	g_signal_connect(G_OBJECT(menu_app_save),
			"activate",
			G_CALLBACK(menu_app_item_click),
			GINT_TO_POINTER(0));

	menu_app_exit = gtk_menu_item_new_with_label("Exit");
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_app), menu_app_exit);
	g_signal_connect(G_OBJECT(menu_app_exit),
			"activate",
			G_CALLBACK(menu_app_item_click),
			GINT_TO_POINTER(1));

	gtk_widget_show_all(menu_app);


	list_node = action_list;
	while (list_node) {
		ACTION	*action = list_node->data;

		if (action->action == ACT_SUBMENU &&
				strcmp(action->command, "-") != 0) {
			action->menu_item = gtk_menu_item_new_with_label(
					action->command);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu_hist),
					action->menu_item);
			action->submenu = gtk_menu_new();
			gtk_menu_item_set_submenu(
					GTK_MENU_ITEM(action->menu_item),
					action->submenu);
			menu_title = gtk_menu_item_new();
			gtk_menu_shell_append(GTK_MENU_SHELL(action->submenu), menu_title);
			gtk_widget_show(menu_title);
			gtk_widget_show(action->menu_item);
			gtk_widget_show(action->submenu);
			submenu_count++;
		}
		if (action->action == ACT_SUBMENU &&
				strcmp(action->command, "-") == 0) {
			printf("'%s'\n", action->command);
			action->submenu = menu_hist;
		}
		list_node = list_node->next;
	}
	if (submenu_count) {
		GtkWidget	*separator;

		separator = gtk_menu_item_new();
		gtk_widget_show(separator);
		gtk_menu_shell_insert(GTK_MENU_SHELL(menu_hist), separator, 1);
	}

	/* prepare colors and styles */
	if (gdk_rgba_parse(&locked_color, locked_color_str) == FALSE) {
		char	msg_str[128];

		sprintf(msg_str, "Invalid color string: '%s'.\n"
				"Falling back to default (red).",
				locked_color_str);
		show_message(msg_str, "Warning", "OK", NULL, NULL);
		strcpy(locked_color_str, DEF_LOCKED_COLOR);
		gdk_rgba_parse(&locked_color, locked_color_str);
	}

	/* set clipboard */
	if (strcmp(clipboard_str, "PRIMARY") == 0) {
		clipboard = GDK_SELECTION_PRIMARY;
	} else if (strcmp(clipboard_str, "SECONDARY") == 0) {
		clipboard = GDK_SELECTION_SECONDARY;
	} else if (strcmp(clipboard_str, "CLIPBOARD") == 0) {
		clipboard = GDK_SELECTION_CLIPBOARD;
	} else {
		char msg_str[128];

		sprintf(msg_str, "Invalid clipboard string: '%s'.\n"
			"Falling back to default ("
			DEF_CLIPBOARD_STR
			").",
			clipboard_str);
		show_message(msg_str, "Warning", "OK", NULL, NULL);
		clipboard = DEF_CLIPBOARD;
	}

	/* load previously saved history */
	if (history_load(dump_only) != 0) {
		if (errno == E_TOO_MUCH) {
			if (show_message("Number of items to keep (-n switch or "
					"keep directive in ~/.wmcliphistrc)\n"
					"is lower than actual number of items "
					"in history file.\nSome items from "
					"history will be lost. May I continue?",
					"Warning", "Yes", "No", NULL) == 1) {
				rcconfig_free();
				exit(1);
			}
		} else if (errno != E_OPEN) {
			rcconfig_free();
			fprintf(stderr, "cannot load history (%d)\n", errno);
			exit(1);
		}
	}
	if (dump_only) {
		rcconfig_free();
		exit(1);
	}


	if (icon_size) {
		cairo_region_t *region;
		cairo_surface_t *surface;

		/* connect signal for menu popup */
		gtk_widget_add_events(dock_app, GDK_BUTTON_PRESS_MASK);
		g_signal_connect(G_OBJECT(dock_app),
                               "event",
                               G_CALLBACK(button_press),
                               G_OBJECT(menu_hist));

		surface = cairo_image_surface_create_from_png(icon_file);
		region = gdk_cairo_region_create_from_surface(surface);
		gdk_window_shape_combine_region(gtk_widget_get_window(dock_app),
						region, 0, 0);


	}


	/* run clipboard monitor */
	g_signal_connect(G_OBJECT(main_window),
			"selection_received",
			G_CALLBACK(my_get_xselection),
			NULL);
	g_timeout_add(250, time_conv_select, NULL);


	/* run autosave timer */
	if (autosave_period > 0)
		g_timeout_add(autosave_period * 1000, history_autosave, NULL);


	/* setup everything for supplying selection to other apps */
	gtk_selection_add_target(dock_app,
			clipboard,
			GDK_SELECTION_TYPE_STRING,
			1);

	g_signal_connect(G_OBJECT(dock_app),
			"selection_get",
			G_CALLBACK(selection_handle),
			NULL);

	g_signal_connect(G_OBJECT(dock_app),
			"destroy",
			G_CALLBACK(wmcliphist_exit),
			NULL);


	hotkeys_init();

	gtk_main();

	return_val(0);
}
