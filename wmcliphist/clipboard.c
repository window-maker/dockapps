#include <wmcliphist.h>
#include <time.h>

/* when true, clipboard will be automatically taken up by wmcliphist */
gint	auto_take_up = 0;

/* number of items to keep (may be overriden from command line) */
gint	num_items_to_keep = 10;

/* number if items kept */
gint	num_items = 0;

/* current number of locked items */
gint	locked_count;

/* list of clipboard history items */
GList	*history_items = NULL;

/* selected item */
HISTORY_ITEM	*selected = NULL;



/*
 * dumps history list to stderr
 * for debugging purposes only
 */
void
dump_history_list_fn(char *header)
{
	gint		indent = 1, i;
	GList		*node = history_items;
	HISTORY_ITEM	*data;
	gchar		*converted;
	
	fprintf(stderr, "%s\n", header);
	while (node) {
		data = (HISTORY_ITEM *)node->data;
		for (i = 0; i < indent; i++)
			putc('-', stderr);
		converted = from_utf8(data->content);
		fprintf(stderr, " %s\n", converted);
		g_free(converted);
		indent++;
		node = node->next;
	}
	fprintf(stderr, "==========\n");
}

void
my_get_selection_text(GtkClipboard *clipboard, const gchar *text, gpointer
		data)
{
	/* previous clipboard content */
	static gchar	*old_content = "";
	static gint	has_old_content = 0;
	gchar		*content;
	gchar		*converted;

	begin_func("my_get_selection_text");

	if (text == NULL) {
		return_void();
	}
	
	if (g_utf8_collate(text, old_content) != 0 &&
			!GTK_CHECK_MENU_ITEM(menu_app_clip_ignore)->active) {
		/* new data in clipboard */
		/* store new content for future comparation */
		content = g_strdup(text);

		converted = from_utf8(content);
		/* fprintf(stderr, ">>> %s\n", converted); */
		g_free(converted);

		if (has_old_content > 0)
			g_free(old_content);
		old_content = content;
		has_old_content = 1;

		/* process item */
		process_item(content, 0, TRUE);
	}

	/* when clipboard is locked, set selection owener to myself */
	if (GTK_CHECK_MENU_ITEM(menu_app_clip_ignore)->active ||
			GTK_CHECK_MENU_ITEM(menu_app_clip_lock)->active) {
		if (gtk_selection_owner_set(dock_app,
					GDK_SELECTION_PRIMARY,
					GDK_CURRENT_TIME) == 0)
			selected = NULL;
	}

	return_void();
}


/*
 * get clipboard content - partialy inspired by Downloader for X
 */
gboolean
my_get_xselection(GtkWidget *window, GdkEvent *event)
{
	GdkAtom		atom;
	gint		format;
	size_t		length;
	gchar		*content = NULL;

	/* previous clipboard content */
	static size_t	old_content_len = 0;
	static gchar	*old_content = "";


	begin_func("my_get_xselection");

	gtk_clipboard_request_text(gtk_clipboard_get(GDK_SELECTION_PRIMARY),
			my_get_selection_text, NULL);

	return_val(TRUE);
	
	length = (size_t) gdk_selection_property_get(window->window,
			(guchar **) &content, &atom, &format);

	if (length > 0) {
		if ((length != old_content_len ||
				memcmp(content, old_content, length) != 0) &&
				!GTK_CHECK_MENU_ITEM(menu_app_clip_ignore)->active) {
			/* new data in clipboard */
			/* store new content for future comparation */
			if (old_content_len > 0)
				g_free(old_content);
			old_content = content;
			old_content_len = length;

			/* process item */
			/* process_item(content, length, 0, TRUE); */
		} else {
			/* no new data */
			g_free(content);
		}

		/* when clipboard is locked, set selection owener to myself */
		if (GTK_CHECK_MENU_ITEM(menu_app_clip_ignore)->active ||
				GTK_CHECK_MENU_ITEM(menu_app_clip_lock)->active) {
			if (gtk_selection_owner_set(dock_app,
						GDK_SELECTION_PRIMARY,
						GDK_CURRENT_TIME) == 0)
				selected = NULL;
		}

	}

	return_val(TRUE);
}


/*
 * clipboard conversion - inspired by Downloader for X too :)
 */
gboolean
time_conv_select()
{
	begin_func("time_conv_select");

	gtk_selection_convert(main_window,
			GDK_SELECTION_PRIMARY,
			GDK_TARGET_STRING,
			GDK_CURRENT_TIME);
	return_val(TRUE);
}


/*
 * handles request for selection from other apps
 */
gboolean
selection_handle(GtkWidget *widget, 
		GtkSelectionData *selection_data,
		guint info,
		guint time_stamp,
		gpointer data)
{
	static gchar	*converted = NULL;
	
	begin_func("selection_handle");

	if (converted != NULL) {
		g_free(converted);
	}
	
	if (selected) {
		converted = from_utf8(selected->content);
		gtk_selection_data_set(selection_data,
				GDK_SELECTION_TYPE_STRING,
				8,
				(guchar *) converted,
				strlen(converted));
	} else {
		gtk_selection_data_set(selection_data,
				GDK_SELECTION_TYPE_STRING,
				8,
				(guchar *)"",
				0);
	}

	return_val(TRUE);
}
