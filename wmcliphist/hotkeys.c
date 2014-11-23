#include <wmcliphist.h>
#include <gdk/gdkkeysyms.h>

/* Exec on hotkey? */
int exec_hotkey = 1;

/* hotkeys */
gchar	menukey_str[32] = DEF_MENUKEY;
guint	menukey;
guint	menukey_mask;

gchar	prev_item_key_str[32] = DEF_PREV_ITEM_KEY;
guint	prev_item_key;
guint	prev_item_mask;

gchar	exec_item_key_str[32] = DEF_EXEC_ITEM_KEY;
guint	exec_item_key;
guint	exec_item_mask;

/*
 * filter grabbed hotkeys
 */
GdkFilterReturn
global_keys_filter(GdkXEvent *gdk_xevent, GdkEvent *event, gpointer data)
{
	XEvent	*xevent = (XEvent *)gdk_xevent;

	begin_func("global_keys_filter");

	if (xevent->type == KeyPress) {
		if (xevent->xkey.keycode ==
				XKeysymToKeycode(GDK_DISPLAY(), menukey) &&
				xevent->xkey.state & menukey_mask) {
			/* popup history menu */
			gtk_menu_popup(GTK_MENU(menu_hist),
					NULL, NULL,
					NULL, NULL,
					0,
					GDK_CURRENT_TIME);
			return_val(GDK_FILTER_REMOVE);
		} else if (xevent->xkey.keycode ==
				XKeysymToKeycode(GDK_DISPLAY(), prev_item_key)
				&& xevent->xkey.state & prev_item_mask) {
			/* switch first two history items */
			GList *second;
			if (history_items == NULL) {
				return_val(GDK_FILTER_REMOVE);
			}
			second = g_list_first(history_items)->next;
			if (second == NULL) {
				return_val(GDK_FILTER_REMOVE);
			}
	
			move_item_to_begin((HISTORY_ITEM *) second->data);

			return_val(GDK_FILTER_REMOVE);
		} else if (xevent->xkey.keycode ==
				XKeysymToKeycode(GDK_DISPLAY(), exec_item_key) &&
				xevent->xkey.state & exec_item_mask) {
			/* exec command on current item */
			if (exec_hotkey) {
				HISTORY_ITEM *hist_item;
				hist_item = (HISTORY_ITEM *) g_list_first(history_items)->data;
				exec_item(hist_item->content, NULL);
			}
			return_val(GDK_FILTER_REMOVE);
		}
	}

	return_val(GDK_FILTER_CONTINUE);
}


/*
 * parse key string
 */
int
hotkey_parse(char *hotkey, guint *key, guint *mask)
{
	char	c;
	char	*tmp = g_new0(char, strlen(hotkey));
	int	i, idx = 0;

	begin_func("hotkey_parse");

	*mask = 0;
	
	for (i = 0; i < strlen(hotkey); i++) {
		c = hotkey[i];
		if (isalpha(c)) {
			tmp[idx++] = c;
			tmp[idx] = '\0';
		} else if (c == '+' || c == '-') {
			idx = 0;
			if (strcasecmp(tmp, "control") == 0 ||
					strcasecmp(tmp, "ctrl") == 0)
				*mask |= ControlMask;
			else if (strcasecmp(tmp, "alt") == 0)
				*mask |= Mod1Mask;
			else if (strcasecmp(tmp, "shift") == 0)
				*mask |= ShiftMask;
			else {
				fprintf(stderr, "Invalid key modifier: %s\n",
						tmp);
				g_free(tmp);
				return_val(-1);
			}
		}
	}

	if ((*key = gdk_keyval_from_name(tmp)) == GDK_VoidSymbol) {
		g_free(tmp);
		return_val(-1);
	}
	
	g_free(tmp);
	return_val(0);
}



#define grab_key(keysym, basemask) \
	XGrabKey(GDK_DISPLAY(), XKeysymToKeycode(GDK_DISPLAY(), keysym), \
			basemask, GDK_ROOT_WINDOW(), True, GrabModeAsync, \
			GrabModeAsync); \
	XGrabKey(GDK_DISPLAY(), XKeysymToKeycode(GDK_DISPLAY(), keysym), \
			basemask | LockMask, GDK_ROOT_WINDOW(), True, \
			GrabModeAsync, GrabModeAsync); \
	XGrabKey(GDK_DISPLAY(), XKeysymToKeycode(GDK_DISPLAY(), keysym), \
			basemask | Mod2Mask, GDK_ROOT_WINDOW(), True, \
			GrabModeAsync, GrabModeAsync); \
	XGrabKey(GDK_DISPLAY(), XKeysymToKeycode(GDK_DISPLAY(), keysym), \
			basemask | Mod2Mask | LockMask, GDK_ROOT_WINDOW(), \
			True, GrabModeAsync, GrabModeAsync);


#define ungrab_key(keysym, basemask) \
	XUngrabKey(GDK_DISPLAY(), XKeysymToKeycode(GDK_DISPLAY(), keysym), \
			basemask, GDK_ROOT_WINDOW()); \
	XUngrabKey(GDK_DISPLAY(), XKeysymToKeycode(GDK_DISPLAY(), keysym), \
			basemask | LockMask, GDK_ROOT_WINDOW()); \
	XUngrabKey(GDK_DISPLAY(), XKeysymToKeycode(GDK_DISPLAY(), keysym), \
			basemask | Mod2Mask, GDK_ROOT_WINDOW()); \
	XUngrabKey(GDK_DISPLAY(), XKeysymToKeycode(GDK_DISPLAY(), keysym), \
			basemask | Mod2Mask | LockMask, GDK_ROOT_WINDOW());


/*
 * initialize hotkeys
 */
void
hotkeys_init()
{
	char	msg_str[128];

	begin_func("hotkeys_init");

	if (hotkey_parse(menukey_str, &menukey, &menukey_mask) != 0) {
		sprintf(msg_str, "Invalid menu hotkey '%s'.\nFalling back to "
				"default (" DEF_MENUKEY ")\n", menukey_str);
		show_message(msg_str, "Warning", "OK", NULL, NULL);
		strcpy(menukey_str, DEF_MENUKEY);
		hotkey_parse(menukey_str, &menukey, &menukey_mask);
	}
	if (hotkey_parse(prev_item_key_str, &prev_item_key, &prev_item_mask) != 0) {
		sprintf(msg_str, "Invalid previous item hotkey '%s'.\n"
				"Falling back to default (" DEF_PREV_ITEM_KEY
				")\n", prev_item_key_str);
		show_message(msg_str, "Warning", "OK", NULL, NULL);
		hotkey_parse(DEF_PREV_ITEM_KEY, &prev_item_key,
				&prev_item_mask);
	}
	if (hotkey_parse(exec_item_key_str, &exec_item_key, &exec_item_mask) != 0) {
		sprintf(msg_str, "Invalid exec hotkey '%s'.\n"
				"Falling back to default (" DEF_EXEC_ITEM_KEY
				")\n", exec_item_key_str);
		show_message(msg_str, "Warning", "OK", NULL, NULL);
		hotkey_parse(DEF_EXEC_ITEM_KEY, &exec_item_key,
				&exec_item_mask);
	}
	gdk_window_add_filter(GDK_ROOT_PARENT(), global_keys_filter, NULL);
	grab_key(menukey, menukey_mask);
	grab_key(prev_item_key, prev_item_mask);
	grab_key(exec_item_key, exec_item_mask);

	return_void();
}

/*
 * disable hotkeys
 */
void
hotkeys_done()
{
	begin_func("hotkeys_done");
	ungrab_key(menukey, menukey_mask);
	return_void();
}
