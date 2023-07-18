
/* This is a simple program that demonstrates how you can easily dock a GAI applet */
/* Written by Jonas Aaberg <cja@gmx.net> */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>

static GtkWidget *fileselection, *dock_entry, *spinbutton, *drawingarea;
static pid_t applet_pid;

void create_fileselection (void);

void exec_gai_applet(char *gai_applet, int size)
{
    putenv(g_strdup_printf("GAI_APPLET_SIZE=%d\n",size));
    putenv(g_strdup_printf("GAI_APPLET_XWINDOW=%d\n",(int)GDK_WINDOW_XWINDOW(drawingarea->window)));

    applet_pid = fork();

    if(applet_pid == 0){
	execlp(gai_applet,gai_applet,NULL);
	exit(0);
    }
    
}

void on_browse_button_clicked(GtkButton *button, gpointer user_data)
{
    create_fileselection();
}




void on_start_applet_button_pressed(GtkButton *button, gpointer user_data)
{

    exec_gai_applet((char *)gtk_entry_get_text (GTK_ENTRY (dock_entry)),
	   gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinbutton)));
}


void on_stop_applet_button_pressed(GtkButton *button, gpointer user_data)
{
    kill(applet_pid,SIGTERM);
}

void on_spinbutton_value_changed(GtkSpinButton *spinbutton, gpointer user_data)
{
    on_stop_applet_button_pressed(NULL, NULL);
    on_start_applet_button_pressed(NULL, NULL);
}


void on_quit_button_pressed(GtkButton *button, gpointer user_data)
{
    on_stop_applet_button_pressed(NULL, NULL);
    gtk_exit(0);
}


void on_ok_button1_pressed(GtkButton *button, gpointer user_data)
{

    gtk_entry_set_text (GTK_ENTRY (dock_entry), 
			gtk_file_selection_get_filename(GTK_FILE_SELECTION(fileselection)));
    gtk_widget_destroy(fileselection);
}


void on_cancel_button1_pressed(GtkButton *button, gpointer user_data)
{
    gtk_widget_destroy(fileselection);
}


void create_dock_window(void)
{
  GtkWidget *window;
  GtkWidget *hbox, *hbox_main;
  GtkWidget *frame;

  GtkWidget *table;
  GtkWidget *browse_button;
  GtkWidget *alignment;
  GtkWidget *image;
  GtkWidget *label;

  GtkObject *spinbutton_adj;
  GtkWidget *start_applet_button;
  GtkWidget *stop_applet_button;
  GtkWidget *quit_button;


  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width(GTK_CONTAINER(window), 5);
  gtk_window_set_title(GTK_WINDOW(window), "Docking a GAI applet example");

  hbox_main = gtk_hbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(window), hbox_main);

  frame = gtk_frame_new(NULL);
  gtk_box_pack_start(GTK_BOX(hbox_main), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(frame), 5);

  table = gtk_table_new(4, 2, FALSE);
  gtk_container_add(GTK_CONTAINER(frame), table);

  browse_button = gtk_button_new();
  gtk_table_attach(GTK_TABLE(table), browse_button, 0, 1, 0, 1,
                   (GtkAttachOptions)(GTK_FILL),
                   (GtkAttachOptions)(0), 0, 0);

  alignment = gtk_alignment_new(0.5, 0.5, 0, 0);
  gtk_container_add(GTK_CONTAINER(browse_button), alignment);

  hbox = gtk_hbox_new(FALSE, 2);
  gtk_container_add(GTK_CONTAINER(alignment), hbox);

  image = gtk_image_new_from_stock("gtk-find", GTK_ICON_SIZE_BUTTON);
  gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic("Browse..");
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);

  dock_entry = gtk_entry_new();
  gtk_table_attach(GTK_TABLE(table), dock_entry, 1, 2, 0, 1,
                   (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                   (GtkAttachOptions)(0), 0, 0);

  label = gtk_label_new("Applet size:");
  gtk_table_attach(GTK_TABLE(table), label, 0, 1, 1, 2,
                   (GtkAttachOptions)(GTK_FILL),
                   (GtkAttachOptions)(0), 0, 0);
  gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);

  spinbutton_adj = gtk_adjustment_new(64, 0, 300, 1, 10, 10);
  spinbutton = gtk_spin_button_new(GTK_ADJUSTMENT(spinbutton_adj), 1, 0);
  gtk_table_attach(GTK_TABLE(table), spinbutton, 1, 2, 1, 2,
                   (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                   (GtkAttachOptions)(0), 0, 0);
  gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(spinbutton), TRUE);

  start_applet_button = gtk_button_new();
  gtk_table_attach(GTK_TABLE(table), start_applet_button, 0, 1, 2, 3,
                   (GtkAttachOptions)(GTK_FILL),
                   (GtkAttachOptions)(0), 0, 0);
  gtk_container_set_border_width(GTK_CONTAINER(start_applet_button), 5);

  alignment = gtk_alignment_new(0.5, 0.5, 0, 0);
  gtk_container_add(GTK_CONTAINER(start_applet_button), alignment);

  hbox = gtk_hbox_new(FALSE, 2);
  gtk_container_add(GTK_CONTAINER(alignment), hbox);

  image = gtk_image_new_from_stock("gtk-execute", GTK_ICON_SIZE_BUTTON);
  gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic("Start applet");
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);

  stop_applet_button = gtk_button_new();
  gtk_table_attach(GTK_TABLE(table), stop_applet_button, 1, 2, 2, 3,
                   (GtkAttachOptions)(GTK_FILL),
                   (GtkAttachOptions)(0), 0, 0);
  gtk_container_set_border_width(GTK_CONTAINER(stop_applet_button), 5);

  alignment = gtk_alignment_new(0.5, 0.5, 0, 0);
  gtk_container_add(GTK_CONTAINER(stop_applet_button), alignment);

  hbox = gtk_hbox_new(FALSE, 2);
  gtk_container_add(GTK_CONTAINER(alignment), hbox);

  image = gtk_image_new_from_stock("gtk-cancel", GTK_ICON_SIZE_BUTTON);
  gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic("Stop applet");
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);

  quit_button = gtk_button_new();
  gtk_table_attach(GTK_TABLE(table), quit_button, 1, 2, 3, 4,
                   (GtkAttachOptions)(GTK_FILL),
                   (GtkAttachOptions)(0), 0, 0);
  gtk_container_set_border_width(GTK_CONTAINER(quit_button), 5);

  alignment = gtk_alignment_new(0.5, 0.5, 0, 0);
  gtk_container_add(GTK_CONTAINER(quit_button), alignment);

  hbox = gtk_hbox_new(FALSE, 2);
  gtk_container_add(GTK_CONTAINER(alignment), hbox);

  image = gtk_image_new_from_stock("gtk-quit", GTK_ICON_SIZE_BUTTON);
  gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);

  label = gtk_label_new_with_mnemonic("Quit");
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
  gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);

  label = gtk_label_new("Settings");
  gtk_frame_set_label_widget(GTK_FRAME(frame), label);
  gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);

  frame = gtk_frame_new(NULL);
  gtk_box_pack_start(GTK_BOX(hbox_main), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(frame), 5);

  drawingarea = gtk_drawing_area_new();
  gtk_container_add(GTK_CONTAINER(frame), drawingarea);

  label = gtk_label_new("Docked GAI applet");
  gtk_frame_set_label_widget(GTK_FRAME(frame), label);
  gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);

  g_signal_connect((gpointer) browse_button, "clicked",
                    G_CALLBACK(on_browse_button_clicked),
                    NULL);
  g_signal_connect((gpointer) spinbutton, "value_changed",
                    G_CALLBACK(on_spinbutton_value_changed),
                    NULL);
  g_signal_connect((gpointer) start_applet_button, "pressed",
                    G_CALLBACK(on_start_applet_button_pressed),
                    NULL);
  g_signal_connect((gpointer) stop_applet_button, "pressed",
                    G_CALLBACK(on_stop_applet_button_pressed),
                    NULL);
  g_signal_connect((gpointer) quit_button, "pressed",
                    G_CALLBACK(on_quit_button_pressed),
                    NULL);

  g_signal_connect((gpointer) window, "destroy",
                    G_CALLBACK(gtk_exit),
                    NULL);

  gtk_widget_show_all(window);
}


void create_fileselection(void)
{
  GtkWidget *ok_button1;
  GtkWidget *cancel_button1;

  fileselection = gtk_file_selection_new("Select a GAI applet to run");
  gtk_container_set_border_width(GTK_CONTAINER(fileselection), 10);

  ok_button1 = GTK_FILE_SELECTION(fileselection)->ok_button;
  gtk_widget_show(ok_button1);
  GTK_WIDGET_SET_FLAGS(ok_button1, GTK_CAN_DEFAULT);

  cancel_button1 = GTK_FILE_SELECTION(fileselection)->cancel_button;
  gtk_widget_show(cancel_button1);
  GTK_WIDGET_SET_FLAGS(cancel_button1, GTK_CAN_DEFAULT);

  g_signal_connect((gpointer) ok_button1, "pressed",
                    G_CALLBACK(on_ok_button1_pressed),
                    NULL);
  g_signal_connect((gpointer) cancel_button1, "pressed",
                    G_CALLBACK(on_cancel_button1_pressed),
                    NULL);

  gtk_widget_show_all(fileselection);
}

int main(int argc, char **argv)
{
    gtk_init(&argc, &argv);
    create_dock_window();
    gtk_main();
    return 0;
}
