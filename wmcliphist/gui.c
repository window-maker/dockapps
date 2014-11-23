#include <wmcliphist.h>
#include <gdk/gdkkeysyms.h>


/* color of locked item */
gchar		locked_color_str[32] = DEF_LOCKED_COLOR;
GdkColor	locked_color;
GtkStyle	*style_locked,
		*style_normal;

/* Exec on middle click? */
int exec_middleclick = 1;

/* main window widget */
GtkWidget	*main_window;

/* dock icon widget */
GtkWidget	*dock_app;

/* clipboard history menu */
GtkWidget	*menu_hist;
GtkWidget	*menu_title;
gint		submenu_count = 0;

/* application menu */
GtkWidget	*menu_app;
GtkWidget	*menu_app_clip_lock;
GtkWidget	*menu_app_clip_ignore;
GtkWidget	*menu_app_exit;
GtkWidget	*menu_app_save;

/* button */
GtkWidget	*button;

/* pixmap */
GtkWidget	*pixmap;
GdkPixmap	*icon;
GdkBitmap	*icon_mask;
GdkBitmap	*mask;


/* ==========================================================================
 *                                                     clipboard history menu
 */

/*
 * history menu item button click function
 */
static gboolean
menu_item_button_released(GtkWidget *widget,
		GdkEvent *event,
		gpointer user_data)
{
	GdkEventButton	*bevent = (GdkEventButton *)event;
	HISTORY_ITEM	*data = user_data;

	begin_func("menu_item_button_released");
	
	/* button 2 or 3 - exec or (un)lock item respectively */
	if (bevent->button == 2) {
		if (exec_middleclick) {
			gtk_menu_popdown(GTK_MENU(menu_hist));
			exec_item(data->content, NULL);
		}
		return_val(TRUE);
	} else if (bevent->button == 3) {
		if (data->locked == 0) {

			/* cannot lock all records */
			if (locked_count == num_items_to_keep - 1) {
				show_message("There must remain at least one "
					"unlocked item\nwhen menu is full!",
					"Warning", "OK", NULL, NULL);
				return_val(TRUE);
			}

			gtk_widget_set_style(GTK_BIN(data->menu_item)->child,
					style_locked);
			data->locked = 1;
			locked_count++;

		} else {
			gtk_widget_set_style(GTK_BIN(data->menu_item)->child,
					style_normal);
			data->locked = 0;
			locked_count--;
		}
	} else {
		return_val(FALSE);
	}

	return_val(TRUE);
}


/*
 * history menu item left click or keypress function
 */
static gboolean
menu_item_activated(GtkWidget *widget, gpointer user_data)
{
	move_item_to_begin((HISTORY_ITEM *) user_data);
	return_val(TRUE);
}


/*
 * checks, if there is already such item in menu,
 * in which case it moves it to the begining
 */
HISTORY_ITEM *
menu_item_exists(gchar *content, GtkWidget *submenu)
{
	HISTORY_ITEM	*hist_item;
	GList		*list_node;

	begin_func("menu_item_exists");

	list_node = g_list_last(history_items);
	while (list_node) {
		hist_item = (HISTORY_ITEM *)list_node->data;
		if (hist_item->menu == submenu
				&& g_utf8_collate(hist_item->content, content)
				== 0) {
			gtk_menu_reorder_child(GTK_MENU(hist_item->menu),
					hist_item->menu_item, 1);
			history_items = g_list_remove_link(history_items,
					list_node);
			history_items = g_list_concat(list_node, history_items);

			return_val(hist_item);
		}
		list_node = g_list_previous(list_node);
	}
	
	return_val(NULL);
}


/*
 * add new item to menu
 */
HISTORY_ITEM *
menu_item_add(gchar *content, gint locked, GtkWidget *target_menu)
{
	GList		*list_node;
	gint		i;
	gchar		*menu_item_name;
	gchar		*fixed_menu_item_name;
	gsize		length;
	HISTORY_ITEM	*hist_item;

	begin_func("menu_item_add");
	
	hist_item = menu_item_exists(content, target_menu);
	if (hist_item != NULL) {
		dump_history_list("reorder");
		return_val(hist_item);
	}

	if (num_items == num_items_to_keep) {
		/* max item limit reached, destroy oldest one */
		list_node = g_list_last(history_items);
		while (1) {
			hist_item = (HISTORY_ITEM*)
				list_node->data;
			if (hist_item->locked == 0)
				break;
			list_node = g_list_previous(list_node);
			g_assert((list_node != NULL));
		}

		history_items = g_list_remove_link(history_items, list_node);
		gtk_container_remove(GTK_CONTAINER(hist_item->menu),
				hist_item->menu_item);
		gtk_widget_destroy(hist_item->menu_item);
		g_free(hist_item->content);
		g_free(hist_item);
		g_list_free_1(list_node);
		num_items--;
		dump_history_list("remove oldest");
	}
	
	/* prepare menu item name */
	menu_item_name = g_new0(char, MAX_ITEM_LENGTH * 2 + 1);
	memset(menu_item_name, 0, MAX_ITEM_LENGTH * 2 + 1);
	length = g_utf8_strlen(content, -1);
	if (length > (size_t) (MAX_ITEM_LENGTH)) {
		g_utf8_strncpy(menu_item_name, content, MAX_ITEM_LENGTH - 4);
		strcat(menu_item_name, "...");
	} else {
		strcpy(menu_item_name, content);
	}

	/* do some menu item name cleanups */
	fixed_menu_item_name = g_new0(char, strlen(menu_item_name) + 1);
	for (i = 0; i < g_utf8_strlen(menu_item_name, -1); i++) {
		gchar *uchar_ptr = g_utf8_offset_to_pointer(menu_item_name, i);
		gunichar uchar = g_utf8_get_char(uchar_ptr);
		if (g_unichar_isprint(uchar)) {
			gchar *decoded_char = g_ucs4_to_utf8(&uchar, 1, NULL,
					NULL, NULL);
			strcat(fixed_menu_item_name, decoded_char);
			g_free(decoded_char);
		} else  {
			strcat(fixed_menu_item_name, "_");
		}
	}
	g_free(menu_item_name);
	menu_item_name = fixed_menu_item_name;

	/* create menu item */
	hist_item = g_new0(HISTORY_ITEM, 1);
	hist_item->menu_item = gtk_menu_item_new_with_label(menu_item_name);
	hist_item->content = g_strdup(content);
	hist_item->locked = locked;
	hist_item->menu = target_menu;

	if (locked == 1) {
		gtk_widget_set_style(GTK_BIN(hist_item->menu_item)->child,
				style_locked);
		locked_count++;
	}
	
	/* add to menu */
	gtk_menu_insert(GTK_MENU(hist_item->menu), hist_item->menu_item, 1);


	/* connect actions to signals */
	gtk_signal_connect(GTK_OBJECT(hist_item->menu_item),
			"button-release-event",
			GTK_SIGNAL_FUNC(menu_item_button_released),
			(gpointer)hist_item);

	gtk_signal_connect(GTK_OBJECT(hist_item->menu_item),
			"activate",
			GTK_SIGNAL_FUNC(menu_item_activated),
			(gpointer)hist_item);

	gtk_widget_show(hist_item->menu_item);

	history_items = g_list_insert(history_items, hist_item, 0);

	num_items++;

	return_val(hist_item);
}



/* ==========================================================================
 *                                                           application menu
 */

/*
 * application main menu handler
 */
gboolean
menu_app_item_click(GtkWidget *menuitem, gpointer data)
{
	gint	button;

	begin_func("menu_app_item_click");

	switch (GPOINTER_TO_INT(data)) {
		/* save history menu */
		case 0:
			if (history_save() != 0) {
				button = show_message("History was NOT saved.\n",
						"Warning", "OK", NULL, NULL);
			}
			return_val(TRUE);

		/* exit menu */
		case 1:
			if (history_save() != 0) {
				button = show_message("History was NOT saved.\n"
						"Do you really want to exit?",
						"Error", "Yes", "No", NULL);
				if (button != 0)
					return_val(TRUE);
			}
			history_free();
			rcconfig_free();

			gtk_exit(0);
			return_val(TRUE);
	}
	return_val(FALSE);
}



/* ==========================================================================
 *                                                          dock button press
 */

/*
 * dock button click response
 */
gboolean
button_press(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	begin_func("button_press");

	if (event->type == GDK_BUTTON_PRESS) {
		GdkEventButton	*bevent = (GdkEventButton *)event;

		switch (bevent->button) {
			case 1:
				/* popup history menu */
				gtk_menu_popup(GTK_MENU(menu_hist),
						NULL, NULL,
						NULL, NULL,
						bevent->button,
						bevent->time);
				return_val(TRUE);

			case 3:
				/* popup application menu */
				gtk_menu_popup(GTK_MENU(menu_app),
						NULL, NULL,
						NULL, NULL,
						bevent->button,
						bevent->time);
				return_val(TRUE);
		}
	}

	return_val(FALSE);
}



/* ==========================================================================
 *                                                            message dialogs
 */

static GMainLoop	*loop;
static gint		button_pressed;


static gboolean
dialog_button_press(GtkWidget *button, gpointer data)
{
	begin_func("dialog_button_press");

	button_pressed = GPOINTER_TO_INT(data);
	g_main_quit(loop);

	return_val(TRUE);
}



static gboolean
dialog_handle_key_press_event(GdkEventKey *event, gpointer data, guint key)
{
	if(event->type != GDK_KEY_PRESS)
		return_val(FALSE);
	if(event->keyval != key)
		return_val(FALSE);
	button_pressed = GPOINTER_TO_INT(data);
	g_main_quit(loop);

	return_val(TRUE);
}



static gboolean
dialog_key_press_yes(GtkWidget *button, GdkEventKey *event, gpointer data)
{
	begin_func("dialog_key_press_yes");
	return dialog_handle_key_press_event(event, data, GDK_Return);
}



static gboolean
dialog_key_press_no(GtkWidget *button, GdkEventKey *event, gpointer data)
{
	begin_func("dialog_key_press_no");
	return dialog_handle_key_press_event(event, data, GDK_Escape);
}



/*
 * open dialog with specified message andbuttons
 * and return number of button pressed
 */
gint
show_message(gchar *message, char *title,
		char *b0_text, char *b1_text, char *b2_text)
{
	GtkWidget	*dialog,
			*label,
			*button_0,
			*button_1,
			*button_2;

	begin_func("show_message");

	/* create the main widgets */
	dialog = gtk_dialog_new();
	label = gtk_label_new(message);

	/* create buttons and set signals */
	button_0 = gtk_button_new_with_label(b0_text);
	gtk_signal_connect(GTK_OBJECT(button_0), "clicked",
			GTK_SIGNAL_FUNC(dialog_button_press),
			GINT_TO_POINTER(0));
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),
			button_0);
	if (!b2_text) {
		gtk_signal_connect(GTK_OBJECT(dialog), "key-press-event",
				GTK_SIGNAL_FUNC(dialog_key_press_yes),
				GINT_TO_POINTER(0));
	}

	if (b1_text != NULL) {
		button_1 = gtk_button_new_with_label(b1_text);
		gtk_signal_connect(GTK_OBJECT(button_1), "clicked",
				GTK_SIGNAL_FUNC(dialog_button_press),
				GINT_TO_POINTER(1));
		gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),
				button_1);
		if (!b2_text) {
			gtk_signal_connect(GTK_OBJECT(dialog), "key-press-event",
					GTK_SIGNAL_FUNC(dialog_key_press_no),
					GINT_TO_POINTER(1));
		}
	}

	if (b2_text) {
		button_2 = gtk_button_new_with_label(b2_text);
		gtk_signal_connect(GTK_OBJECT(button_2), "clicked",
				GTK_SIGNAL_FUNC(dialog_button_press),
				GINT_TO_POINTER(2));
		gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),
				button_2);
	}

	/* add the label, and show everything we've added to the dialog. */
	gtk_misc_set_padding(&GTK_LABEL(label)->misc, 10, 10);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), label);
	gtk_widget_show_all(dialog);

	/* set window title */
	gtk_window_set_title(GTK_WINDOW(dialog), title);
	
	loop = g_main_new(FALSE);
	g_main_run(loop);
	g_main_destroy(loop);
	gtk_widget_destroy(dialog);
	
	return_val(button_pressed);
}
