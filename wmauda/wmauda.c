/*  wmauda - Dockapp for controlling Audacious
 *  
 *  Copyright (C) 2006       Michael Stewart <michael@alteredeclipse.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <audacious/dbus.h>
#include <audacious/audctrl.h>

#include "dock-master.xpm"

#include <getopt.h>

#ifndef VERSION
# define VERSION 0.8
#endif

#ifndef PIXMAP_DIR
# define PIXMAP_DIR "/usr/local/share/pixmaps"
#endif

typedef struct
{
	int x, y, width, height, pressed_x, pressed_y, normal_x, normal_y;
	gboolean focus, pressed;
	void (*callback) (void);
}
Button;

void action_play(void);
void action_pause(void);
void action_eject(void);
void action_prev(void);
void action_next(void);
void action_stop(void);

Button buttons[] =
{
	{21, 32, 9, 11, 84, 0, 64, 0, FALSE, FALSE, action_play},       /* PLAY */
	{34, 32, 9, 11, 94, 0, 74, 0, FALSE, FALSE, action_pause},      /* PAUSE */
	{47, 32, 9, 11, 84, 11, 64, 11, FALSE, FALSE, action_eject},    /* EJECT */
	{21, 46, 9, 11, 84, 22, 64, 22, FALSE, FALSE, action_prev},     /* PREV */
	{34, 46, 9, 11, 94, 22, 74, 22, FALSE, FALSE, action_next},     /* NEXT */
	{47, 46, 9, 11, 94, 11, 74, 11, FALSE, FALSE, action_stop},     /* STOP */
};

#define NUM_BUTTONS 6

GList *button_list;

typedef struct
{
    unsigned char ascii; gint x, y;
} Charentry;
 
Charentry chartable[] =
{
  { '-', 60, 73},  /* put here coordinates of characters */
  { '.', 72, 73},  /* in xmms-dock-master.xpm */
  { ',', 78, 73}, 
  { '\\', 84, 73},
  { '/', 90, 73}, 
  { '(', 96, 73}, 
  { ')', 102, 73},
  { '%', 108, 73},
  { 'Ä', 114, 73},
  { 'ä', 114, 73},  /* toupper doesn't convert umlauts */
  { 'Ö', 120, 73},
  { 'ö', 120, 73},
  { 'Ü', 126, 73},
  { 'ü', 126, 73},
  { '?', 132, 73},
  { '!', 138, 73},
  { '&', 144, 73},
  { ':', 150, 73},
  { ' ', 66, 73},
};
#define NUM_CHARS 19
                  
#define VOLSLIDER_X		8 
#define VOLSLIDER_Y		17
#define VOLSLIDER_WIDTH		7 


#define	VOLSLIDER_HEIGHT	40

#define SEEKSLIDER_X		21
#define SEEKSLIDER_Y		20
#define SEEKSLIDER_WIDTH	30
#define SEEKSLIDER_HEIGHT	7
#define SEEKSLIDER_KNOB_WIDTH	3
#define SEEKSLIDER_MAX		(SEEKSLIDER_WIDTH - SEEKSLIDER_KNOB_WIDTH)

#define SCROLLTEXT_X		5
#define SCROLLTEXT_Y		6
#define SCROLLTEXT_WIDTH	40
#define SCROLLTEXT_HEIGHT	9 
#define SCROLLTEXT_CHARS	9

gboolean volslider_dragging = FALSE;
int volslider_pos = 0;
gboolean seekslider_visible = FALSE, seekslider_dragging = FALSE;
int seekslider_pos = -1, seekslider_drag_offset = 0;
gint scrollpos = 0;
int timeout_tag = 0;

void init(void);

GtkWidget *icon_win;
GdkPixmap *pixmap, *launch_pixmap;
GdkBitmap *mask, *launch_mask;
GdkGC *dock_gc;
GtkTooltips *tooltips = NULL;

char *xmms_cmd = "audacious";
gboolean xmms_running = FALSE;

gboolean has_geometry = FALSE, single_click = FALSE, song_title = FALSE;
char *icon_name = NULL;
int win_x, win_y;

DBusGProxy *dbus_proxy = NULL;   
static DBusGConnection *connection = NULL;

GtkTargetEntry drop_types[] =
{
	{"text/plain", 0, 1}
};

void action_play(void)
{
	audacious_remote_play(dbus_proxy);
}

void action_pause(void)
{
	audacious_remote_pause(dbus_proxy);
}

void action_eject(void)
{
	audacious_remote_playlist_clear(dbus_proxy);
	audacious_remote_stop(dbus_proxy);
}

void action_prev(void)
{
	audacious_remote_playlist_prev(dbus_proxy);
}

void action_next(void)
{
	audacious_remote_playlist_next(dbus_proxy);
}

void action_stop(void)
{
	audacious_remote_stop(dbus_proxy);
}

gboolean inside_region(int mx, int my, int x, int y, int w, int h)
{
	if ((mx >= x && mx < x + w) && (my >= y && my < y + h))
		return TRUE;
	return FALSE;
}

void real_draw_button(GdkWindow *w, Button *button)
{

	if (button->pressed)
		gdk_draw_pixmap(w, dock_gc, pixmap,
				button->pressed_x, button->pressed_y,
				button->x, button->y,
				button->width, button->height);
	else
		gdk_draw_pixmap(w, dock_gc, pixmap,
				button->normal_x, button->normal_y,
				button->x, button->y,
				button->width, button->height);
}

void draw_button(Button *button)
{
	real_draw_button(icon_win->window, button);
}

void draw_buttons(GList *list)
{
	for (; list; list = g_list_next(list))
		draw_button(list->data);
}

void real_draw_volslider(GdkWindow *w)
{
	gdk_draw_pixmap(w, dock_gc, pixmap, 112, 1, VOLSLIDER_X, VOLSLIDER_Y,
			VOLSLIDER_WIDTH, VOLSLIDER_HEIGHT);
	gdk_draw_pixmap(w, dock_gc, pixmap, 106,
			1 + VOLSLIDER_HEIGHT - volslider_pos,
			VOLSLIDER_X,
			VOLSLIDER_Y + VOLSLIDER_HEIGHT - volslider_pos,
			VOLSLIDER_WIDTH, volslider_pos);
}

void draw_volslider(void)
{
	real_draw_volslider(icon_win->window);
}

void real_draw_seekslider(GdkWindow *w)
{
	int slider_x;

	if (seekslider_visible)
	{
		gdk_draw_pixmap(w, dock_gc, pixmap, 66, 54,
				SEEKSLIDER_X, SEEKSLIDER_Y, 35, 10);
		if (seekslider_pos < SEEKSLIDER_MAX / 3)
			slider_x = 108;
		else if (seekslider_pos < (SEEKSLIDER_MAX * 2) / 3)
			slider_x = 111;
		else
			slider_x = 114;
		gdk_draw_pixmap(w, dock_gc, pixmap, slider_x, 48,
				SEEKSLIDER_X + seekslider_pos,
				SEEKSLIDER_Y, 3, SEEKSLIDER_HEIGHT);
	}
	else
		gdk_draw_pixmap(w, dock_gc, pixmap, 66, 39,
				SEEKSLIDER_X, SEEKSLIDER_Y, 35, 10);
}

void draw_seekslider(void)
{
	real_draw_seekslider(icon_win->window);
}

void real_draw_scrolltext(GdkWindow * w)
{
        /* get titlestring */
	gint pl_pos = audacious_remote_get_playlist_pos(dbus_proxy);

	if (pl_pos != -1)
	{
	        char *title = audacious_remote_get_playlist_title(dbus_proxy, pl_pos);
		if (title)
		{
		        /* render text */	
		        gint i = 0, c = 0, pos = 0, dest = 0;
			
			for (i=0; i<SCROLLTEXT_CHARS; i++)
			{
			        gint x = 66, y = 73;
			        scrollpos %= (strlen(title)+16) * 6;
				pos = i + (scrollpos / 6) -8;
				if (pos < strlen(title) && pos >= 0)
				        c = toupper(title[pos]);
				else
				        c = ' ';

				dest = SCROLLTEXT_X + (i * 6 - (scrollpos % 6));

				if (c >= 'A' && c <= 'Z')
				{
				    x = (c-'A')*6;
				    y = 64;
				}
				else if (c >= '0' && c <= '9')
				    x = (c-'0')*6;
				else
				{
				    int i = 0;
				    for (i=0; i<NUM_CHARS; i++)
				        if (c == chartable[i].ascii)
					{
					    x = chartable[i].x;
					    y = chartable[i].y;
					    break;
					}
				}
				gdk_draw_pixmap(w, dock_gc, pixmap, x, y, 
						dest, SCROLLTEXT_Y, 7, 9);

			}
			scrollpos++;
			g_free(title);
		}
	}
}

void draw_scrolltext(void)
{
	real_draw_scrolltext(icon_win->window);
}

void redraw_window(void)
{
	if (xmms_running)
	{
		gdk_draw_pixmap(icon_win->window, dock_gc, pixmap,
				0, 0, 0, 0, 64, 64);
		draw_buttons(button_list);
		draw_volslider();
		draw_seekslider();
		draw_scrolltext();
	}
	else
	{
		gdk_draw_pixmap(icon_win->window, dock_gc, launch_pixmap,
				0, 0, 0, 0, 64, 64);
	}
}

void expose_cb(GtkWidget *w, GdkEventExpose *event, gpointer data)
{
	redraw_window();
}

void wheel_scroll_cb(GtkWidget *w, GdkEventScroll *event)
{
	if (xmms_running)
	{
		if (event->direction == GDK_SCROLL_UP || event->direction == GDK_SCROLL_DOWN)
		{
			if (event->direction == GDK_SCROLL_UP)
				volslider_pos += 3;
			else
				volslider_pos -= 3;
			if (volslider_pos < 0)
				volslider_pos = 0;
			if (volslider_pos > VOLSLIDER_HEIGHT)
				volslider_pos = VOLSLIDER_HEIGHT;
			audacious_remote_set_main_volume(dbus_proxy, (volslider_pos * 100) / VOLSLIDER_HEIGHT);
			draw_volslider();
		}
	}
}
void button_press_cb(GtkWidget *w, GdkEventButton *event, gpointer data)
{
	GList *node;
	Button *btn;
	int pos;
	char *cmd;

	if (xmms_running)
	{
		if ((event->button == 2) || (event->button == 3))
		{
			if(audacious_remote_is_main_win(dbus_proxy))
				audacious_remote_main_win_toggle(dbus_proxy, FALSE);
			else
				audacious_remote_main_win_toggle(dbus_proxy, TRUE);
		}
	}

	if (event->button != 1)
		return;
	if (xmms_running)
	{
		for (node = button_list; node; node = g_list_next(node))
		{
			btn = node->data;
			if (inside_region(event->x, event->y, btn->x, btn->y, btn->width, btn->height))
			{
				btn->focus = TRUE;
				btn->pressed = TRUE;
				draw_button(btn);
			}
		}
		if (inside_region(event->x, event->y, VOLSLIDER_X, VOLSLIDER_Y, VOLSLIDER_WIDTH, VOLSLIDER_HEIGHT))
		{
			volslider_pos = VOLSLIDER_HEIGHT - (event->y - VOLSLIDER_Y);
			audacious_remote_set_main_volume(dbus_proxy, (volslider_pos * 100) / VOLSLIDER_HEIGHT);
			draw_volslider();
			volslider_dragging = TRUE;
		}
		if (inside_region(event->x, event->y, SEEKSLIDER_X, SEEKSLIDER_Y, SEEKSLIDER_WIDTH, SEEKSLIDER_HEIGHT) && seekslider_visible)
		{
			pos = event->x - SEEKSLIDER_X;

			if (pos >= seekslider_pos &&
			    pos < seekslider_pos + SEEKSLIDER_KNOB_WIDTH)
				seekslider_drag_offset = pos - seekslider_pos;
			else
			{
				seekslider_drag_offset = 1;
				seekslider_pos = pos - seekslider_drag_offset;
				if (seekslider_pos < 0)
					seekslider_pos = 0;
				if (seekslider_pos > SEEKSLIDER_MAX)
					seekslider_pos = SEEKSLIDER_MAX;
			}
			draw_seekslider();
			seekslider_dragging = TRUE;
		}
	}
	else if ((!single_click && event->type == GDK_2BUTTON_PRESS) ||
		 (single_click && event->type == GDK_BUTTON_PRESS))
	{
		cmd = g_strconcat(xmms_cmd, " &", NULL);
		system(cmd);
		g_free(cmd);
	}
}

void button_release_cb(GtkWidget *w, GdkEventButton *event, gpointer data)
{
	GList *node;
	Button *btn;
	int len;

	if (event->button != 1)
		return;

	for (node = button_list; node; node = g_list_next(node))
	{
		btn = node->data;
		if (btn->pressed)
		{
			btn->focus = FALSE;
			btn->pressed = FALSE;
			draw_button(btn);
			if (btn->callback)
				btn->callback();
		}
	}
	volslider_dragging = FALSE;
	if (seekslider_dragging)
	{
		len = audacious_remote_get_playlist_time(dbus_proxy, audacious_remote_get_playlist_pos(dbus_proxy));
		audacious_remote_jump_to_time(dbus_proxy, (seekslider_pos * len) / SEEKSLIDER_MAX);
		seekslider_dragging = FALSE;
	}

}

void motion_notify_cb(GtkWidget *w, GdkEventMotion *event, gpointer data)
{
	GList *node;
	Button *btn;
	gboolean inside;

	for (node = button_list; node; node = g_list_next(node))
	{
		btn = node->data;
		if (btn->focus)
		{
			inside = inside_region(event->x, event->y,
					       btn->x, btn->y,
					       btn->width, btn->height);
			if ((inside && !btn->pressed) ||
			    (!inside && btn->pressed))
			{
				btn->pressed = inside;
				draw_button(btn);
			}
		}
	}
	if (volslider_dragging)
	{
		volslider_pos = VOLSLIDER_HEIGHT - (event->y - VOLSLIDER_Y);
		if (volslider_pos < 0)
			volslider_pos = 0;
		if (volslider_pos > VOLSLIDER_HEIGHT)
			volslider_pos = VOLSLIDER_HEIGHT;
		audacious_remote_set_main_volume(dbus_proxy, (volslider_pos * 100) / VOLSLIDER_HEIGHT);
		draw_volslider();
	}
	if (seekslider_dragging)
	{
		seekslider_pos =
			event->x - SEEKSLIDER_X - seekslider_drag_offset;
		if (seekslider_pos < 0)
			seekslider_pos = 0;
		if (seekslider_pos > SEEKSLIDER_MAX)
			seekslider_pos = SEEKSLIDER_MAX;
		draw_seekslider();
	}

}

void destroy_cb(GtkWidget *w, gpointer data)
{
	gtk_exit(0);
}

static void update_tooltip(void)
{
	static int pl_pos = -1;
	static char *filename;
	int new_pos;
	
	if (!tooltips)
		return;
	
	new_pos = audacious_remote_get_playlist_pos(dbus_proxy);

	if (new_pos == 0)
	{
		/*
		 * Need to do some extra checking, as we get 0 also on
		 * a empty playlist
		 */
		char *current = audacious_remote_get_playlist_file(dbus_proxy, 0);
		if (!filename && current)
		{
			filename = current;
			new_pos = -1;
		}
		else if (filename && !current)
		{
			g_free(filename);
			filename = NULL;
			new_pos = -1;
		}
		else if (filename && current && strcmp(filename, current))
		{
			g_free(filename);
			filename = current;
			new_pos = -1;
		}
	}

	if (pl_pos != new_pos)
	{
		char *tip = NULL;
		char *title =
			audacious_remote_get_playlist_title(dbus_proxy, new_pos);
		if (title)
		{
			tip = g_strdup_printf("%d. %s", new_pos+1, title);
			g_free(title);
		}
		gtk_tooltips_set_tip(tooltips, icon_win, tip, NULL);
		g_free(tip);
		pl_pos = new_pos;
	}
}

int timeout_func(gpointer data)
{
	int new_pos, pos;
	gboolean playing, running;

	running = audacious_remote_is_running(dbus_proxy);

	if (running)
	{
		if (!xmms_running)
		{
			gtk_widget_shape_combine_mask(icon_win, mask, 0, 0);
			xmms_running = running;
			redraw_window();
		}
		if (!volslider_dragging)
		{
			new_pos = (audacious_remote_get_main_volume(dbus_proxy) * 40) / 100;

			if (new_pos < 0)
				new_pos = 0;
			if (new_pos > VOLSLIDER_HEIGHT)
				new_pos = VOLSLIDER_HEIGHT;

			if (volslider_pos != new_pos)
			{
				volslider_pos = new_pos;
				draw_volslider();
			}
		}

		update_tooltip();
		draw_scrolltext();

		playing = audacious_remote_is_playing(dbus_proxy);
		if (!playing && seekslider_visible)
		{
			seekslider_visible = FALSE;
			seekslider_dragging = FALSE;
			seekslider_pos = -1;
			draw_seekslider();
		}
		else if (playing)
		{
			int len, p = audacious_remote_get_playlist_pos(dbus_proxy);
			len = audacious_remote_get_playlist_time(dbus_proxy, p);
			if (len == -1)
			{
				seekslider_visible = FALSE;
				seekslider_dragging = FALSE;
				seekslider_pos = -1;
				draw_seekslider();
			}
			else if (!seekslider_dragging)
			{
				seekslider_visible = TRUE;
				pos = audacious_remote_get_output_time(dbus_proxy);
				if (len != 0)
					new_pos = (pos * SEEKSLIDER_MAX) / len;
				else
					new_pos = 0;
				if (new_pos < 0)
					new_pos = 0;
				if (new_pos > SEEKSLIDER_MAX)
					new_pos = SEEKSLIDER_MAX;
				if (seekslider_pos != new_pos)
				{
					seekslider_pos = new_pos;
					draw_seekslider();
				}
			}
		}
	}
	else
	{
		if (xmms_running)
		{
			if (tooltips != NULL)
				gtk_tooltips_set_tip(tooltips, icon_win, NULL, NULL);
			gtk_widget_shape_combine_mask(icon_win, launch_mask, 0, 0);
			xmms_running = FALSE;
			redraw_window();
		}
	}

	return TRUE;
}

void drag_data_received(GtkWidget *widget, GdkDragContext *context,
			int x, int y, GtkSelectionData *selection_data,
			guint info, guint time)
{
	if (selection_data->data)
	{
		char *url = selection_data->data;
		audacious_remote_playlist_clear(dbus_proxy);
		audacious_remote_playlist_add_url_string(dbus_proxy, url);
		audacious_remote_play(dbus_proxy);
	}
}

static gboolean dbus_init(void)
{
	GError *error = NULL;
	
	connection = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
	if (connection == NULL)
		return FALSE;
		
	dbus_proxy = dbus_g_proxy_new_for_name(connection, AUDACIOUS_DBUS_SERVICE,
                                                           AUDACIOUS_DBUS_PATH,
                                                           AUDACIOUS_DBUS_INTERFACE);
	if (dbus_proxy == NULL)
		return FALSE;
		
	return TRUE;
}

void init(void)
{
	GdkWindowAttr attr;
	GdkColor bg_color;
	GdkWindow *leader;
	XWMHints hints;
	int i, w, h;
	GdkGC *mask_gc;

	for (i = 0; i < NUM_BUTTONS; i++)
		button_list = g_list_append(button_list, &buttons[i]);

	if (song_title)
	{
		tooltips = gtk_tooltips_new();
		gtk_tooltips_set_delay(tooltips, 1000);
	}
		
	icon_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_app_paintable(icon_win, TRUE);
	gtk_widget_set_uposition(icon_win, 0, 0);
	gtk_widget_set_usize(icon_win, 64, 64);
	gtk_widget_set_events(icon_win,
			      GDK_BUTTON_MOTION_MASK | GDK_BUTTON_PRESS_MASK |
			      GDK_BUTTON_RELEASE_MASK | GDK_EXPOSURE_MASK);
	gtk_signal_connect(GTK_OBJECT(icon_win), "expose_event",
			   GTK_SIGNAL_FUNC(expose_cb), NULL);
	gtk_signal_connect(GTK_OBJECT(icon_win), "button_press_event",
			   GTK_SIGNAL_FUNC(button_press_cb), NULL);
	gtk_signal_connect(GTK_OBJECT(icon_win), "scroll_event",
			   GTK_SIGNAL_FUNC(wheel_scroll_cb), NULL);
	gtk_signal_connect(GTK_OBJECT(icon_win), "button_release_event",
			   GTK_SIGNAL_FUNC(button_release_cb), NULL);
	gtk_signal_connect(GTK_OBJECT(icon_win), "motion_notify_event",
			   GTK_SIGNAL_FUNC(motion_notify_cb), NULL);
	gtk_signal_connect(GTK_OBJECT(icon_win), "destroy",
			   GTK_SIGNAL_FUNC(destroy_cb), NULL);
	gtk_drag_dest_set(icon_win, GTK_DEST_DEFAULT_ALL, drop_types, 1,
			  GDK_ACTION_COPY);
	gtk_signal_connect(GTK_OBJECT(icon_win), "drag_data_received",
			   GTK_SIGNAL_FUNC(drag_data_received), NULL);
	gtk_widget_realize(icon_win);
	bg_color.red = 0;
	bg_color.green = 0;
	bg_color.blue = 0;
	gdk_colormap_alloc_color(gdk_colormap_get_system(),
				 &bg_color, FALSE, TRUE);
	gdk_window_set_background(icon_win->window, &bg_color);
	gdk_window_clear(icon_win->window);
	dock_gc = gdk_gc_new(icon_win->window);

	launch_pixmap = gdk_pixmap_new(icon_win->window, 64, 64, -1);

	launch_mask = gdk_pixmap_new(icon_win->window, 64, 64, 1);
	mask_gc = gdk_gc_new(launch_mask);
	bg_color.pixel = 0;
	gdk_gc_set_foreground(mask_gc, &bg_color);
	gdk_draw_rectangle(launch_mask, mask_gc, TRUE, 0, 0, -1, -1);

	if (!icon_name)
		icon_name = g_strdup_printf("%s/wmauda.xpm", PIXMAP_DIR);
	pixmap = gdk_pixmap_create_from_xpm(icon_win->window, &mask,
					    NULL, icon_name);
	if (!pixmap)
	{
		printf("ERROR: Couldn't find %s\n", icon_name);
		g_free(icon_name);
		gtk_exit(1);
	}
	g_free(icon_name);
	gdk_window_get_size(pixmap, &w, &h);
	if (w > 64)
		w = 64;
	if (h > 64)
		h = 64;
	gdk_draw_pixmap(launch_pixmap, dock_gc, pixmap,
			0, 0, 32 - (w / 2), 32 - (h / 2), w, h);
	gdk_draw_pixmap(launch_mask, mask_gc, mask,
			0, 0, 32 - (w / 2), 32 - (h / 2), w, h);
	gdk_gc_unref(mask_gc);
	gdk_pixmap_unref(pixmap);
	gdk_bitmap_unref(mask);

	gtk_widget_shape_combine_mask(icon_win, launch_mask, 0, 0);

	pixmap = gdk_pixmap_create_from_xpm_d(icon_win->window,
					      &mask, NULL, dock_master_xpm);

	attr.width = 64;
	attr.height = 64;
	attr.title = "wmauda";
	attr.event_mask = GDK_BUTTON_PRESS_MASK | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_POINTER_MOTION_HINT_MASK;
	attr.wclass = GDK_INPUT_OUTPUT;
	attr.visual = gdk_visual_get_system();
	attr.colormap = gdk_colormap_get_system();
	attr.wmclass_name = "wmauda";
	attr.wmclass_class = "wmauda";
	attr.window_type = GDK_WINDOW_TOPLEVEL;

	leader = gdk_window_new(NULL, &attr, GDK_WA_TITLE | GDK_WA_WMCLASS | GDK_WA_VISUAL | GDK_WA_COLORMAP);

	gdk_window_set_icon(leader, icon_win->window, NULL, NULL);
	gdk_window_reparent(icon_win->window, leader, 0, 0);
	gdk_window_show(leader);
	
	hints.initial_state = WithdrawnState;
	hints.flags = StateHint | IconWindowHint | IconPositionHint | WindowGroupHint;
	hints.icon_window = GDK_WINDOW_XWINDOW(icon_win->window);
	hints.icon_x = 0;
	hints.icon_y = 0;
	hints.window_group = GDK_WINDOW_XWINDOW(leader);
		
	XSetWMHints(GDK_DISPLAY(), GDK_WINDOW_XWINDOW(leader), &hints);
			
	gtk_widget_show(icon_win);
	timeout_tag = gtk_timeout_add(100, timeout_func, NULL);

}

void display_usage(char *cmd)
{
	printf(	"Usage: %s [options]\n\n"
		"Options:\n"
		"--------\n\n"
		"-h, --help		Display this text and exit.\n"
		"-g, --geometry		Set the geometry (for example +20+20)\n"
		"-c, --command		Command to launch Audacious (Default: audacious)\n"
		"-i, --icon		Set the icon to use when Audacious is not running\n"
		"-n, --single		Only a single click is needed to start Audacious\n"
		"-t, --title		Display song title when mouse is in window\n"
		"-v, --version		Display version information and exit\n\n",
	       cmd);
}

int main(int argc, char **argv)
{
	int c, dummy;

	static struct option lopt[] =
	{
		{"help", 		no_argument, 		0, 'h'},
		{"geometry",	required_argument,	0, 'g'},
		{"session", 	required_argument,	0, 's'},
		{"command", 	required_argument,	0, 'c'},
		{"icon",		required_argument,	0, 'i'},
		{"single",		no_argument,		0, 'n'},
		{"title", 		no_argument,		0, 't'},
		{"version", 	no_argument,		0, 'v'},
		{0, 0, 0, 0}
	};

	gtk_set_locale();

	gtk_init(&argc, &argv);

	while ((c = getopt_long(argc, argv, "hg:s:c:i:ntv", lopt, NULL)) != -1)
	{
		switch (c)
		{
			case 'h':
				display_usage(argv[0]);
				gtk_exit(0);
				break;
			case 'g':
				XParseGeometry(optarg, &win_x, &win_y,
					       &dummy, &dummy);
				has_geometry = TRUE;
				break;
			case 'c':
				xmms_cmd = g_strdup(optarg);
				break;
			case 'i':
				icon_name = g_strdup(optarg);
				break;
			case 'n':
				single_click = TRUE;
				break;
			case 't':
				song_title = TRUE;
				break;
			case 'v':
				printf("wmauda %s\n", VERSION);
				gtk_exit(0);
				break;
		}
	}
	if (!dbus_init())
		return 1;
	
	init();
	gtk_main();
	return 0;
}
