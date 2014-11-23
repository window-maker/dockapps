/*
 * File: foodock.h
 *
 * Created: Fri Jan 14 01:14:25 2000
 * 
 * (c) 2000, Alexey Vyskubov <alexey@pepper.spb.ru>
 *
 * LGPL, see file LICENSE 
*/

/*
 * Function foo_create_main_icon_window returns pointer to gtk event
 * box. This event box is created into main window and can be used as
 * a dockable Windowmaker applet. Main window should be realized
 * before calling foo_create_main_icon_window. Returned event box
 * will be realized by foo_create_main_icon_window. You should to show
 * icon window as well as main window before gtk_main().
 *
 * Call foo_set_wmhints() after both windows are shown (gtk_widget_show()).
 * 
 * Input:
 *	mw		Pointer to main window
 *	s		icon window size (56 is recommended)
 *	margc, margv	argc and argv of main program for XSetCommand
 */
GtkWidget *foo_create_main_icon_window(GtkWidget *main_window,
				       unsigned int size,
				       int main_argc,
				       char *main_argv[]);

/*
 * Set WMHints on the dockapp (icon) window. Needs to be called after
 * the main window is shown, due to changes in GTK+ 2.4.
 * 
 * Input:
 *	mw		Pointer to main window
 *	dw		Pointer to icon (dockapp) window
 *	margc, margv	argc and argv of main program for XSetCommand
 */
void foo_set_wmhints(GtkWidget *mw,
		     GtkWidget *dw,
		     int margc,
		     char *margv[]);
