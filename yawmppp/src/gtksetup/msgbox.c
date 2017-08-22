/*

    Message Box Composite Widget
    (currently included in various software packages)
    Copyright (C) 1999-2000 Felipe Paulo Guazzi Bergo
    bergo@seul.org

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

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "msgbox.h"

#include "bong.xpm"
#include "exclam.xpm"
#include "info.xpm"
#include "question.xpm"

/* PRIVATE */

void message_box_click_yes(GtkWidget *widget,gpointer data);
void message_box_click_no(GtkWidget *widget,gpointer data);
void message_box_click_cancel(GtkWidget *widget,gpointer data);
void message_box_click_ok(GtkWidget *widget,gpointer data);
gint immortal(GtkWidget *widget,GdkEvent *event,gpointer data);

struct msgbox_data {
  gint         answer_available;
  MsgBoxResult answer;
} cmdata;

gchar ** mb_icons[5]={NULL,bong_xpm,info_xpm,exclam_xpm,question_xpm};

MsgBoxResult
message_box(GtkWindow *parent,char *txt,char *title,MsgBoxType mbt,MsgBoxIcon mbi)
{
  GtkWidget *msgbox,*b[3],*lb,*tbl,*hs;
  GdkBitmap *mask;
  GdkPixmap *dqmark;
  GtkWidget *tqmark;
  GtkStyle *style;
  int i,nb;

  cmdata.answer_available=FALSE;

  msgbox=gtk_window_new(GTK_WINDOW_DIALOG);
  tbl=gtk_table_new(3,4,FALSE);
  gtk_container_add(GTK_CONTAINER(msgbox),tbl);
  gtk_window_set_title(GTK_WINDOW(msgbox),title);
  if (parent!=NULL)
    gtk_window_set_transient_for(GTK_WINDOW(msgbox),parent);
  gtk_container_set_border_width(GTK_CONTAINER(msgbox),4);
  gtk_window_set_position(GTK_WINDOW(msgbox),GTK_WIN_POS_CENTER);
  gtk_window_set_policy(GTK_WINDOW(msgbox),TRUE,TRUE,TRUE);
  gtk_widget_realize(msgbox); 

  style=gtk_widget_get_style(msgbox);
  if (mbi!=MSGBOX_ICON_NONE) {
    dqmark=gdk_pixmap_create_from_xpm_d(msgbox->window,&mask,
					&style->bg[GTK_STATE_NORMAL],
					mb_icons[mbi]);
    tqmark=gtk_pixmap_new(dqmark,mask);
    gtk_table_attach(GTK_TABLE(tbl),tqmark,0,1,0,1,GTK_FILL,GTK_FILL,5,5);
    gtk_widget_show(tqmark);
  }

  lb=gtk_label_new(txt);
  gtk_label_set_justify(GTK_LABEL(lb),GTK_JUSTIFY_LEFT);
  gtk_table_attach(GTK_TABLE(tbl),lb,1,4,0,1,GTK_FILL,GTK_FILL,10,10);
  gtk_widget_show(lb);

  hs=gtk_hseparator_new();
  gtk_table_attach(GTK_TABLE(tbl),hs,0,4,1,2,GTK_FILL,GTK_FILL,0,10);
  gtk_widget_show(hs);

  /* buttons */

  switch(mbt) {
  case MSGBOX_OK:
    nb=1;
    b[0]=gtk_button_new_with_label("OK");
    gtk_signal_connect(GTK_OBJECT(b[0]),"clicked",
		       GTK_SIGNAL_FUNC(message_box_click_ok),NULL);
    break;
  case MSGBOX_OKCANCEL:
    nb=2;
    b[0]=gtk_button_new_with_label("OK");
    gtk_signal_connect(GTK_OBJECT(b[0]),"clicked",
		       GTK_SIGNAL_FUNC(message_box_click_ok),NULL);
    b[1]=gtk_button_new_with_label("Cancel");
    gtk_signal_connect(GTK_OBJECT(b[1]),"clicked",
		       GTK_SIGNAL_FUNC(message_box_click_cancel),NULL);
    break;
  case MSGBOX_YESNO:
    nb=2;
    b[0]=gtk_button_new_with_label("Yes");
    gtk_signal_connect(GTK_OBJECT(b[0]),"clicked",
		       GTK_SIGNAL_FUNC(message_box_click_yes),NULL);
    b[1]=gtk_button_new_with_label("No");
    gtk_signal_connect(GTK_OBJECT(b[1]),"clicked",
		       GTK_SIGNAL_FUNC(message_box_click_no),NULL);
    break;
  case MSGBOX_YESNOCANCEL:
    nb=3;
    b[0]=gtk_button_new_with_label("Yes");
    gtk_signal_connect(GTK_OBJECT(b[0]),"clicked",
		       GTK_SIGNAL_FUNC(message_box_click_yes),NULL);
    b[1]=gtk_button_new_with_label("No");
    gtk_signal_connect(GTK_OBJECT(b[1]),"clicked",
		       GTK_SIGNAL_FUNC(message_box_click_no),NULL);
    b[2]=gtk_button_new_with_label("Cancel");
    gtk_signal_connect(GTK_OBJECT(b[2]),"clicked",
		       GTK_SIGNAL_FUNC(message_box_click_cancel),NULL);
    break;
  default:
    nb=0;
  }

  for(i=0;i<nb;i++)
    gtk_table_attach(GTK_TABLE(tbl),b[i],
		     4-nb+i,4-nb+i+1,
		     2,3,
		     GTK_FILL,
		     (i?GTK_SHRINK:GTK_FILL),2,2);

  GTK_WIDGET_SET_FLAGS(b[0],GTK_CAN_DEFAULT);
  gtk_widget_grab_default(b[0]);

  for(i=0;i<nb;i++)
    gtk_widget_show(b[i]);

  gtk_signal_connect(GTK_OBJECT(msgbox),"delete_event",
		     GTK_SIGNAL_FUNC(immortal),NULL);

  gtk_widget_show(tbl);
  gtk_widget_show(msgbox);
  gtk_grab_add(msgbox);

  /* gtk_main MUST be running before message_box is called!!! */
  while(!cmdata.answer_available)
    gtk_main_iteration();

  gtk_grab_remove(msgbox);
  gtk_widget_destroy(msgbox);

  return(cmdata.answer);
}

void message_box_click_yes(GtkWidget *widget,gpointer data) {
  cmdata.answer=MSGBOX_R_YES;
  cmdata.answer_available=TRUE;
}

void message_box_click_no(GtkWidget *widget,gpointer data) {
  cmdata.answer=MSGBOX_R_NO;
  cmdata.answer_available=TRUE;
}

void message_box_click_cancel(GtkWidget *widget,gpointer data) {
  cmdata.answer=MSGBOX_R_CANCEL;
  cmdata.answer_available=TRUE;
}

void message_box_click_ok(GtkWidget *widget,gpointer data) {
  cmdata.answer=MSGBOX_R_OK;
  cmdata.answer_available=TRUE;
}

gint immortal(GtkWidget *widget,GdkEvent *event,gpointer data) {
  return TRUE;
}

