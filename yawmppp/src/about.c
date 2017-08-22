/*

   YAWMPPP - PPP dock app/helper for WindowMaker
   Copyright (C) 2000, 2001:

   Author: Felipe Bergo (bergo@seul.org)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   $Id: about.c,v 1.1.1.1 2001/02/22 06:15:21 bergo Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "about.h"

#include "stepphone.xpm"

GtkWidget *about;

/* data must be a pointer to parent widget (GtkWindow) */
void applet_about (GtkWidget * widget, gpointer data)
{
  GtkStyle *style;
  GdkPixmap *phone;
  GtkWidget *pi,*v,*h,*text,*h2,*b,*p,*v2;
  char tabout[1024];
  GdkBitmap *mask;
  int i;

  about=gtk_window_new(GTK_WINDOW_DIALOG);
  gtk_window_set_transient_for(GTK_WINDOW(about),GTK_WINDOW(data));
  gtk_window_set_wmclass(GTK_WINDOW(about),"yawmppp","about");
  gtk_widget_realize(about);
  gtk_window_set_policy(GTK_WINDOW(about),TRUE,TRUE,TRUE);
  gtk_window_set_title (GTK_WINDOW (about), "About YAWMPPP");

  style=gtk_widget_get_style(about);
  phone=gdk_pixmap_create_from_xpm_d(about->window,&mask,
				    &style->bg[GTK_STATE_NORMAL],
				    (gchar **)stepphone_xpm);
  pi=gtk_pixmap_new(phone,mask);
  gdk_pixmap_unref(phone);

  v=gtk_vbox_new(FALSE,2);
  gtk_container_add(GTK_CONTAINER(about),v);

  h=gtk_hbox_new(FALSE,2);
  gtk_box_pack_start(GTK_BOX(v),h,FALSE,TRUE,2);

  v2=gtk_vbox_new(FALSE,4);
  gtk_box_pack_start(GTK_BOX(h),v2,FALSE,FALSE,2);
  gtk_box_pack_start(GTK_BOX(v2),pi,FALSE,FALSE,6);

  strcpy(tabout,"YAWMPPP\nYet Another PPP Dock Applet for Window Maker\n");
  strcat(tabout,"Version ");
  strcat(tabout,VERSION);
  strcat(tabout,"\nCopyright (C) 2000, 2001 Felipe Bergo\n");
  strcat(tabout,"email: bergo@seul.org\n\n");

  strcat(tabout,"YAWMPPP is distributed under the terms of the GNU\n");
  strcat(tabout,"General Public License, version 2 or later.\n");
  strcat(tabout,"YAWMPPP comes with ABSOLUTELY NO WARRANTY;\n");
  strcat(tabout,"This is free software, and you are welcome to\n");
  strcat(tabout,"redistribute it under certain conditions.\n");
  strcat(tabout,"Read the file COPYING for details.\n\n");
  strcat(tabout,"To learn more about free software visit\n");
  strcat(tabout,"http://www.gnu.org\n");

  text=gtk_label_new(tabout);
  gtk_box_pack_start(GTK_BOX(h),text,FALSE,TRUE,2);
  gtk_label_set_justify(GTK_LABEL(text),GTK_JUSTIFY_LEFT);

  h2=gtk_hbox_new(TRUE,2);
  gtk_box_pack_start(GTK_BOX(v),h2,FALSE,TRUE,2);

  for(i=0;i<3;i++) {
    p=gtk_label_new(" ");
    gtk_box_pack_start(GTK_BOX(h2),p,FALSE,FALSE,2);
    gtk_widget_show(p);
  }

  b=gtk_button_new_with_label("Dismiss");
  gtk_box_pack_start(GTK_BOX(h2),b,FALSE,TRUE,4);

  gtk_container_set_border_width(GTK_CONTAINER(about),6);

  gtk_widget_show(b);
  gtk_widget_show(text);
  gtk_widget_show(pi);
  gtk_widget_show(h2);
  gtk_widget_show(h);
  gtk_widget_show(v2);
  gtk_widget_show(v);
  gtk_widget_show(about);
  gtk_signal_connect(GTK_OBJECT(b),"clicked",
		     GTK_SIGNAL_FUNC(about_dismiss),NULL);
  gtk_grab_add(about);
}

void
about_dismiss(GtkWidget *wid,gpointer data)
{
  gtk_grab_remove(about);
  gtk_widget_destroy(about);
}
