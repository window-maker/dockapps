/*
 * (c) 2001 Michal Krause <michal@krause.cz>
 */

#include <wmcliphist.h>

#include <icon/ico_60x60_mask.xbm>
#include <icon/ico_60x60_black.xpm>
#include <icon/ico_60x60_white.xpm>
#include <icon/ico_60x60_gray.xpm>

#include <icon/ico_40x40_mask.xbm>
#include <icon/ico_40x40_black.xpm>
#include <icon/ico_40x40_white.xpm>
#include <icon/ico_40x40_gray.xpm>

#include <icon/ico_30x30_mask.xbm>
#include <icon/ico_30x30_black.xpm>
#include <icon/ico_30x30_white.xpm>
#include <icon/ico_30x30_gray.xpm>

#include <icon/ico_16x16_mask.xbm>
#include <icon/ico_16x16.xpm>

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
			" in pseudo XML format (no escaping etc.)\n\n");

	exit(1);
	return_void();
}

static void
wmcliphist_exit(gint code)
{
	begin_func("wmcliphist_exit");
	gtk_exit(code);
	return_void();
}


/*
 * main func
 */
int
main(int argc, char **argv)
{
	gint	i = 1, res;
	gchar	*arg;
	GList	*list_node;
	int	icon_number = 0;
	int	icon_size = 60;
	gchar	**icon_data;
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
	gtk_set_locale();
	gtk_init(&argc, &argv);


	/* create main window */
	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_realize(main_window);


	/* creat dock icon */
	dock_app = foo_create_main_icon_window(main_window,
			icon_size, argc, argv);

	if (icon_size) {
		/* create icon_mask */
		if (icon_size == 60) {
			/* 60x60 icon */
			icon_mask = gdk_bitmap_create_from_data(main_window->window,
					(gchar *) ico_60x60_mask_bits,
					ico_60x60_mask_width,
					ico_60x60_mask_height);
			/* create icon */
			if (icon_number == 0) {
				icon_data = ico_60x60_gray_xpm;
			} else if (icon_number == 1) {
				icon_data = ico_60x60_black_xpm;
			} else {
				icon_data = ico_60x60_white_xpm;
			}
		} else if (icon_size == 40) {
			/* 40x40 icon */
			icon_mask = gdk_bitmap_create_from_data(main_window->window,
					(gchar *) ico_40x40_mask_bits,
					ico_40x40_mask_width,
					ico_40x40_mask_height);
			/* create icon */
			if (icon_number == 0) {
				icon_data = ico_40x40_gray_xpm;
			} else if (icon_number == 1) {
				icon_data = ico_40x40_black_xpm;
			} else {
				icon_data = ico_40x40_white_xpm;
			}
		} else if (icon_size == 30) {
			/* 30x30 icon */
			icon_mask = gdk_bitmap_create_from_data(main_window->window,
					(gchar *) ico_30x30_mask_bits,
					ico_30x30_mask_width,
					ico_30x30_mask_height);
			/* create icon */
			if (icon_number == 0) {
				icon_data = ico_30x30_gray_xpm;
			} else if (icon_number == 1) {
				icon_data = ico_30x30_black_xpm;
			} else {
				icon_data = ico_30x30_white_xpm;
			}
		} else {
			/* 16x16 icon */
			icon_mask = gdk_bitmap_create_from_data(main_window->window,
					(gchar *) ico_16x16_mask_bits,
					ico_16x16_mask_width,
					ico_16x16_mask_height);
			/* create icon */
			icon_data = ico_16x16_xpm;
		}

		icon = gdk_pixmap_create_from_xpm_d(main_window->window,
				NULL, NULL, icon_data);
		pixmap = gtk_pixmap_new(icon, icon_mask);
		gtk_widget_show(pixmap);
		gtk_container_add(GTK_CONTAINER(dock_app), pixmap);
	}

	
	/* create clipboard history menu */
	menu_hist = gtk_menu_new();
	gtk_menu_set_title(GTK_MENU(menu_hist), "Clipboard history");
	menu_title = gtk_tearoff_menu_item_new();
	gtk_menu_append(GTK_MENU(menu_hist), menu_title);
	gtk_widget_show(menu_title);
	gtk_widget_show(menu_hist);


	/* create application menu */
	menu_app = gtk_menu_new();

	menu_app_clip_lock = gtk_check_menu_item_new_with_label("Clipboard lock");
	gtk_menu_append(GTK_MENU(menu_app), menu_app_clip_lock);

	menu_app_clip_ignore = gtk_check_menu_item_new_with_label("Clipboard ignore");
	gtk_menu_append(GTK_MENU(menu_app), menu_app_clip_ignore);

	menu_app_save = gtk_menu_item_new_with_label("Save history");
	gtk_menu_append(GTK_MENU(menu_app), menu_app_save);
	gtk_signal_connect(GTK_OBJECT(menu_app_save),
			"activate",
			GTK_SIGNAL_FUNC(menu_app_item_click),
			GINT_TO_POINTER(0));

	menu_app_exit = gtk_menu_item_new_with_label("Exit");
	gtk_menu_append(GTK_MENU(menu_app), menu_app_exit);
	gtk_signal_connect(GTK_OBJECT(menu_app_exit),
			"activate",
			GTK_SIGNAL_FUNC(menu_app_item_click),
			GINT_TO_POINTER(1));

	gtk_widget_show_all(menu_app);


	list_node = action_list;
	while (list_node) {
		ACTION	*action = list_node->data;

		if (action->action == ACT_SUBMENU &&
				strcmp(action->command, "-") != 0) {
			action->menu_item = gtk_menu_item_new_with_label(
					action->command);
			gtk_menu_append(GTK_MENU(menu_hist),
					action->menu_item);
			action->submenu = gtk_menu_new();
			gtk_menu_item_set_submenu(
					GTK_MENU_ITEM(action->menu_item),
					action->submenu);
			menu_title = gtk_tearoff_menu_item_new();
			gtk_menu_append(GTK_MENU(action->submenu), menu_title);
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
		gtk_menu_insert(GTK_MENU(menu_hist), separator, 1);
	}

	/* prepare colors and styles */
	if (gdk_color_parse(locked_color_str, &locked_color) == 0) {
		char	msg_str[128];

		sprintf(msg_str, "Invalid color string: '%s'.\n"
				"Falling back to default (red).",
				locked_color_str);
		show_message(msg_str, "Warning", "OK", NULL, NULL);
		strcpy(locked_color_str, DEF_LOCKED_COLOR);
		gdk_color_parse(locked_color_str, &locked_color);
	}
	style_normal = gtk_style_copy(gtk_widget_get_style(menu_hist));
	style_locked = gtk_style_copy(gtk_widget_get_style(menu_hist));
	style_locked->fg[GTK_STATE_NORMAL] = locked_color;
	style_locked->fg[GTK_STATE_PRELIGHT] = locked_color;


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
				gtk_exit(1);
			}
		} else if (errno != E_OPEN) {
			rcconfig_free();
			fprintf(stderr, "cannot load history (%d)\n", errno);
			gtk_exit(1);
		}
	}
	if (dump_only) {
		rcconfig_free();
		gtk_exit(1);
	}


	if (icon_size) {
		/* connect signal for menu popup */
		gtk_signal_connect(GTK_OBJECT(dock_app),
				"event",
				GTK_SIGNAL_FUNC(button_press),
				GTK_OBJECT(menu_hist));


		/* show icon */
		gtk_widget_show(dock_app);
		gtk_widget_show(main_window);
 
		/* Set WMHints - after gtk_widget_show() due to changes in GTK+ 2.4 */
		foo_set_wmhints(main_window, dock_app, argc, argv);

		gdk_window_shape_combine_mask(main_window->window, icon_mask, 0, 0);
		gdk_window_shape_combine_mask(dock_app->window, icon_mask, 0, 0);
	}

	
	/* run clipboard monitor */
	gtk_signal_connect(GTK_OBJECT(main_window),
			"selection_received",
			GTK_SIGNAL_FUNC(my_get_xselection),
			NULL);
	gtk_timeout_add(250, time_conv_select, NULL);
	

	/* run autosave timer */
	if (autosave_period > 0)
		gtk_timeout_add(autosave_period * 1000, history_autosave, NULL);
	

	/* setup everything for supplying selection to other apps */
	gtk_selection_add_target(dock_app,
			GDK_SELECTION_PRIMARY,
			GDK_SELECTION_TYPE_STRING,
			1);

	gtk_signal_connect(GTK_OBJECT(dock_app),
			"selection_get",
			GTK_SIGNAL_FUNC(selection_handle),
			NULL);

	gtk_signal_connect(GTK_OBJECT(dock_app),
			"destroy",
			GTK_SIGNAL_FUNC(wmcliphist_exit),
			NULL);


	hotkeys_init();
	
	gtk_main();

	return_val(0);
}
