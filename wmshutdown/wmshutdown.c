/* wmshutdown - dockapp to shutdown or reboot your machine
 *
 * Copyright 2001, 2002 Rafael V. Aroca <rafael@linuxqos.cjb.net>
 * Copyright 2014 Doug Torrance <dtorrance@monmouthcollege.edu>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gio/gio.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

enum {
  GTK_RESPONSE_HALT,
  GTK_RESPONSE_REBOOT,
  GTK_RESPONSE_SUSPEND,
  GTK_RESPONSE_HIBERNATE
};

#define HALT_METHOD      "PowerOff"
#define REBOOT_METHOD    "Reboot"
#define SUSPEND_METHOD   "Suspend"
#define HIBERNATE_METHOD "Hibernate"
#define DBUS_PATH        "/org/freedesktop/login1"
#define DBUS_INTERFACE   "org.freedesktop.login1.Manager"
#define DBUS_DESTINATION "org.freedesktop.login1"

static int showVersion;
static gboolean show_suspend = FALSE;
static gboolean show_hibernate = FALSE;

GtkWidget *dialog = NULL;

static GOptionEntry entries[] = {
	{ "version", 'v', 0, G_OPTION_ARG_NONE, &showVersion,
	  "Display version information", NULL },
	{ "suspend", 's', 0, G_OPTION_ARG_NONE, &show_suspend,
	  "Show 'suspend' button", NULL },
	{ "hibernate", 'b', 0, G_OPTION_ARG_NONE, &show_hibernate,
	  "Show 'hibernate' button", NULL },
	{ NULL }
};

/* gtk3 dockapp code based on wmpasman by Brad Jorsch
 * <anomie@users.sourceforge.net>
 * http://sourceforge.net/projects/wmpasman */

GtkWidget *cria_dock(GtkWidget *mw, unsigned int s)
{
	GdkDisplay *display;
	GtkWidget *foobox;
	Display *d;
	Window mainwin, iw, w, p, dummy1, *dummy2;
	unsigned int dummy3;
	XWMHints *wmHints;

	(void) s;
	display = gdk_display_get_default();
	foobox = gtk_window_new(GTK_WINDOW_POPUP);

	gtk_window_set_wmclass(GTK_WINDOW(mw), g_get_prgname(), "DockApp");
	gtk_widget_set_size_request(foobox, 47, 47);

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

void fecha(void)
{
	gtk_widget_destroy(dialog);
	dialog = NULL;
}

void handle_click(GtkWidget *widget, gpointer data)
{
	GDBusConnection *connection;
	GDBusMessage *message, *reply;
	GError *error = NULL;

	gchar *method = (gchar *)data;

	connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
	message = g_dbus_message_new_method_call(
			NULL,
			DBUS_PATH,
			DBUS_INTERFACE,
			method);
	g_dbus_message_set_body(message, g_variant_new("(b)", TRUE));
	g_dbus_message_set_destination(message, DBUS_DESTINATION);
	reply = g_dbus_connection_send_message_with_reply_sync(
		connection, message, 0, -1, NULL, NULL, &error);

	if (g_dbus_message_get_message_type(reply) ==
	    G_DBUS_MESSAGE_TYPE_ERROR) {
		GtkWidget *dialog;

		dialog = gtk_message_dialog_new(
			GTK_WINDOW(gtk_widget_get_toplevel(widget)),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_CLOSE,
			"%s",
			g_strcompress(g_variant_print(
					      g_dbus_message_get_body(reply),
					      TRUE)));
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}

	g_object_unref(message);
	g_object_unref(reply);
	g_object_unref(connection);
}

void button_press(GtkWidget *widget, GdkEvent *event)
{
	GtkWidget *label;
	GtkWidget *halt_button;
	GtkWidget *reboot_button;
	GtkWidget *suspend_button;
	GtkWidget *hibernate_button;
	GtkWidget *cancel_button;
	GdkEventButton  *bevent;

	(void) widget;

	bevent = (GdkEventButton *)event;
	switch (bevent->button) {
	case 1:
		if (dialog != NULL)
			return;
		dialog = gtk_dialog_new();
		label = gtk_label_new("Shutdown confirmation");
		gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(
							GTK_DIALOG(dialog))),
				  label);

		gtk_dialog_add_buttons(GTK_DIALOG(dialog),
				       "Halt", GTK_RESPONSE_HALT,
				       "Reboot", GTK_RESPONSE_REBOOT, NULL);
		halt_button = gtk_dialog_get_widget_for_response(
			GTK_DIALOG(dialog),
			GTK_RESPONSE_HALT);
		reboot_button = gtk_dialog_get_widget_for_response(
			GTK_DIALOG(dialog),
			GTK_RESPONSE_REBOOT);
		g_signal_connect(halt_button,
				 "clicked",
				 G_CALLBACK(handle_click),
				 HALT_METHOD);
		g_signal_connect(reboot_button,
				 "clicked",
				 G_CALLBACK(handle_click),
				 REBOOT_METHOD);

		if (show_suspend) {
		  gtk_dialog_add_buttons(GTK_DIALOG(dialog), "Suspend",
					 GTK_RESPONSE_SUSPEND, NULL);
		  suspend_button =
		    gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),
						       GTK_RESPONSE_SUSPEND);
		  g_signal_connect(suspend_button,
				   "clicked",
				   G_CALLBACK(handle_click),
				   SUSPEND_METHOD);
		}
		if (show_hibernate) {
		  gtk_dialog_add_buttons(GTK_DIALOG(dialog), "Hibernate",
					 GTK_RESPONSE_HIBERNATE, NULL);
		  hibernate_button =
		    gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),
						       GTK_RESPONSE_HIBERNATE);
		  g_signal_connect(hibernate_button,
				   "clicked",
				   G_CALLBACK(handle_click),
				   HIBERNATE_METHOD);
		}

		gtk_dialog_add_buttons(GTK_DIALOG(dialog), "Cancel",
				       GTK_RESPONSE_CANCEL, NULL);
		cancel_button = gtk_dialog_get_widget_for_response(
			GTK_DIALOG(dialog),
			GTK_RESPONSE_CANCEL);
		g_signal_connect(cancel_button,
				 "clicked",
				 G_CALLBACK(fecha),
				 (gpointer) dialog);

		g_signal_connect(dialog, "destroy", G_CALLBACK(fecha), NULL);
		gtk_widget_show_all(dialog);
		break;
	default:
		break;
	}
}

int main(int argc, char *argv[])
{
	GError *error = NULL;
	GOptionContext *context;
	GtkWidget *gtkiw;
	GtkWidget *dockArea;
	GtkWidget *pixmap;

	gtk_init(&argc, &argv);

	context = g_option_context_new(
		"- dockapp to shutdown or reboot your machine");
	g_option_context_add_main_entries(context, entries, NULL);
	g_option_context_add_group(context, gtk_get_option_group(TRUE));
	g_option_context_parse(context, &argc, &argv, &error);

	if (showVersion) {
		printf(PACKAGE_STRING"\n");
		return 0;
	}

	gtkiw = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	dockArea = cria_dock(gtkiw, 47);

	pixmap = gtk_image_new_from_file(DATADIR"/wmshutdown.xpm");
	gtk_widget_show(pixmap);
	gtk_container_add(GTK_CONTAINER(dockArea), pixmap);

	gtk_widget_add_events(dockArea, GDK_BUTTON_PRESS_MASK);
	g_signal_connect(dockArea, "button-press-event",
			 G_CALLBACK(button_press), NULL);

	gtk_main();

	return 0;
}
