#include <wmcliphist.h>


int	autosave_period = 120;
int	confirm_exec = 0;
int	exec_immediately = 1;

/*
 * process new history item
 */
void
process_item(char *content, gint locked, gboolean exec)
{
	GList		*list_node;
	ACTION		*action;
	gboolean	processed = FALSE;
	HISTORY_ITEM	*hist_item;

	begin_func("process_item");

	list_node = g_list_first(action_list);
	while (list_node) {

		action = (ACTION *)list_node->data;

		/* check if some action is requested */
		if (regexec(&action->expression, content, 0, NULL, 0) != 0) {
			list_node = g_list_next(list_node);
			continue;
		}

		/* match - execute requested action */

		if (action->action == ACT_IGNORE) {
			processed = TRUE;
			break;
		}
		if (action->action == ACT_EXEC && exec_immediately == TRUE
				&& exec == TRUE) {
			exec_item(content, action);
		} 
		if (action->action == ACT_SUBMENU) {
			/* test if such item already exists in this menu */
			processed = TRUE;

			/* add item to menu and item list */
			hist_item = menu_item_add(content, locked,
					action->submenu);

			/* when auto_take_up is true, set selection owner to myself */
			if (auto_take_up == 1) {
				selected = hist_item;
				if (gtk_selection_owner_set(dock_app,
							GDK_SELECTION_PRIMARY,
							GDK_CURRENT_TIME) == 0) {
					selected = NULL;
				}
			}
			
			dump_history_list("added item");
			break;
		}

		list_node = g_list_next(list_node);
	}

	if (processed == FALSE) {
		hist_item = menu_item_add(content, locked, menu_hist);
		
		/* when auto_take_up is true, set selection owner to myself */
		if (auto_take_up == 1) {
			selected = hist_item;
			if (gtk_selection_owner_set(dock_app,
						GDK_SELECTION_PRIMARY,
						GDK_CURRENT_TIME) == 0) {
				selected = NULL;
			}
		}
	}

	return_void();
}

void
move_item_to_begin(HISTORY_ITEM *item) {
	GList	*list_node;

	begin_func("menu_item_activated");

	if (!(list_node = g_list_find(history_items, item))) {
		g_assert((list_node != NULL));
	}

	gtk_menu_popdown(GTK_MENU(menu_hist));
	/* move previously stored item to beginning */
	gtk_menu_reorder_child(GTK_MENU(item->menu),
			item->menu_item, 1);
	history_items = g_list_remove_link(history_items, list_node);
	history_items = g_list_concat(list_node, history_items);
	selected = item;
	if (gtk_selection_owner_set(dock_app,
				GDK_SELECTION_PRIMARY,
				GDK_CURRENT_TIME) == 0)
		selected = NULL;
}


/*
 * Exec's an action on item.
 */
void
exec_item(char *content, ACTION *action)
{
	int	msg_result = 0, res;
	gchar	*msg_buf;
	gchar	*exec_buf;
	gchar	*converted;

	converted = from_utf8(content);
	
	/* If we're not given an action to perform, find the first matching
	 * exec action, and perform it */
	if (!action) {
		GList	*list_node;
		ACTION  *a;
		list_node = g_list_first(action_list);
		while (list_node) {
			a = (ACTION *)list_node->data;
			/* check if some action is requested */
			if ((regexec(&a->expression, converted, 0, NULL, 0)
						== 0)
			    && (a->action == ACT_EXEC)) {
				action = a;
				break;
			}
			list_node = g_list_next(list_node);
		}
	}

	if (!action || action->action != ACT_EXEC) {
		g_free(converted);
		return;
	}

	exec_buf = g_new0(char, strlen(converted) +
			strlen(action->command) + 1);
	sprintf(exec_buf, action->command, converted);
	if (confirm_exec) {
		msg_buf = g_new0(char, strlen(exec_buf) + 256);
		sprintf(msg_buf, "Do you want to perform the "
				"following action?\n\n%s",
				exec_buf);
		msg_result = show_message(msg_buf,
				"wmcliphist", "Yes", "No", NULL);
		g_free(msg_buf);
	}

	/* create child and exec command */
	if (msg_result == 0 && fork() == 0) {
		/* child */
		res = system(exec_buf);
		if (res == -1)
			fprintf(stderr, "Cannot exec '%s'\n", exec_buf);
		else if (res == 127)
			fprintf(stderr, "/bin/sh not found\n");
		g_free(exec_buf);
		g_free(converted);
		_exit(0);
	} else {
		/* parent */
		g_free(exec_buf);
		g_free(converted);
	}
}

/*
 * loads history from file
 */
int
history_load(gboolean dump_only)
{
	gchar		*buf;
	gint		len;
	gint		ver;
	FILE		*f;
	gchar		*fname;
	gint		locked;
	int		tmp_errno = 0;

	begin_func("history_load");

	fname = rcconfig_get_name(".data");
	if (!(f = fopen(fname, "r"))) {
		errno = E_OPEN;
		return_val(-1);
	}

	if (fread(&ver, sizeof(gint), 1, f) != 1) {
		fclose(f);
		return_val(0);
	}

	/* delete old history file */
	if (ver == 0x0001) {
		fclose(f);
		if (remove(rcconfig_get_name(".data"))) {
			errno = E_REMOVE;
			return_val(-1);
		}
		return_val(0);
	}

	if (dump_only) {
		printf("<history>\n");
	}
	while (!feof(f)) {

		if (fread(&len, sizeof(gint), 1, f) != 1)
			break;

		if (num_items == num_items_to_keep && !dump_only) {
			tmp_errno = E_TOO_MUCH;
			break;
		}

		buf = g_new0(gchar, len + 1);
		if (fread(buf, len, 1, f) != 1) {
			g_free(buf);
			tmp_errno = E_INVALID;
			break;
		}
		buf[len] = '\0';

		if (fread(&locked, sizeof(gint), 1, f) != 1) {
			g_free(buf);
			tmp_errno = E_INVALID;
			break;
		}

		if (dump_only) {
			printf("<item>%s</item>\n", buf);
		} else {
			process_item(buf, locked, FALSE);
		}
		g_free(buf);

	}
	fclose(f);

	if (dump_only) {
		printf("</history>\n");
	} else {
		dump_history_list("load_history()");
	}

	errno = tmp_errno;
	
	if (errno == 0)
		return_val(0);
	else
		return_val(-1);
}


/*
 * store history to file
 */
int
history_save()
{
	char		*fname;
	gint		version = VERSION;
	FILE		*f;
	HISTORY_ITEM	*hist_item;
	GList		*list_node;
	int		tmp_errno = 0;

	begin_func("history_save");

	fname = g_strdup(rcconfig_get_name(".data.tmp"));

	if (!(f = fopen(fname, "w"))) {
		perror("fopen");
		g_free(fname);
		errno = E_OPEN;
		return_val(-1);
	}

	if ((chmod(fname, S_IRUSR|S_IWUSR)) != 0) {
		perror("chmod");
		fclose(f);
		unlink(fname);
		g_free(fname);
		errno = E_OPEN;
		return_val(-1);
	}

	if (fwrite(&version, sizeof(gint), 1, f) != 1) {
		perror("fwrite version");
		fclose(f);
		unlink(fname);
		g_free(fname);
		errno = E_WRITE;
		return_val(-1);
	}

	list_node = g_list_last(history_items);
	while (list_node) {
		int length;
		hist_item = (HISTORY_ITEM *)list_node->data;
		length = strlen(hist_item->content);
		if (fwrite(&length, sizeof(gint), 1, f) != 1) {
			tmp_errno = E_WRITE;
			break;
		}
		if (fwrite(hist_item->content, length, 1, f) != 1) {
			tmp_errno = E_WRITE;
			break;
		}
		if (fwrite(&hist_item->locked, sizeof(gint), 1, f) != 1) {
			tmp_errno = E_WRITE;
			break;
		}
		list_node = g_list_previous(list_node);
	}

	fclose(f);

	if (!list_node) {
		if (rename(fname, rcconfig_get_name(".data")) != 0) {
			perror("rename");
			unlink(fname);
			g_free(fname);
			errno = E_RENAME;
			return_val(-1);
		}
		g_free(fname);
		return_val(0);
	}

	errno = tmp_errno;
	unlink(fname);
	g_free(fname);

	return_val(-1);
}


/*
 * free history data
 */
void
history_free()
{
	HISTORY_ITEM	*hist_item;
	GList		*list_node;

	begin_func("history_free");

	list_node = g_list_last(history_items);
	while (list_node) {
		hist_item = (HISTORY_ITEM *)list_node->data;
		gtk_container_remove(GTK_CONTAINER(hist_item->menu),
				hist_item->menu_item);
		gtk_widget_destroy(hist_item->menu_item);
		g_free(hist_item->content);
		g_free(hist_item);
		list_node = g_list_previous(list_node);
	}
	g_list_free(history_items);

	return_void();
}


/*
 * autosave timer function
 */
gboolean
history_autosave()
{
	begin_func("history_autosave");

	history_save();
	return_val(TRUE);
}

