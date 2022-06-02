/*
 * GAI - General Applet Interface Library
 * Copyright (C) 2003-2004 Jonas Aaberg <cja@gmx.net>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * - Preference window generator 
 */

#include "../config.h"
#include "gai.h"
#include "gai-private.h"

//extern const char *colourbutton_xpm;
#include "colourbutton.xpm"

#include <stdio.h>
#include <string.h>

#define BUTTON_LABEL_PIC 0
#define BUTTON_PIC_LABEL 1
#define DEFAULT_MAX_WIDGET 1000

typedef struct {
    int type;
    void *default_val;
    void *result;
    void *result2;
    void *result3;
    void *result4;
    GtkWidget *widget;
} Var_GtkCheckButton;

typedef struct {
    GtkImage *image;
    GdkPixbuf *pixbuf;
    GtkWidget *csd;
    GtkWidget *button;
    char *name;
    unsigned char r;
    unsigned char g;
    unsigned char b;

    unsigned char old_r;
    unsigned char old_g;
    unsigned char old_b;

    unsigned char alpha;
    unsigned char old_alpha;
} Var_GtkColorSelectionDialog;

typedef struct
{
    GtkWidget *fsd;
    GtkWidget *entry;
    char *name;
} Var_GtkFileSelection;

static Var_GtkCheckButton *iw;
static Var_GtkColorSelectionDialog *cs;
static Var_GtkFileSelection *fs;
static GSList *radio_group[DEFAULT_MAX_WIDGET];


static int max_iw, max_cs;
static int iw_ptr, cs_ptr, fs_ptr;


static struct {
    GtkWidget *pref_window;
    GtkWidget *apply_button;
} var = { NULL, NULL };

static void gai_pref_get_answers(void);

/* ............................................................ */

static void 
apply_button_valid(void)
{
    if (var.apply_button != NULL)
	gtk_widget_set_sensitive(var.apply_button,1);
}


static void 
on_close_button_clicked(GtkButton *b, gpointer d)
{
    if(var.pref_window!=NULL){
	gtk_widget_destroy(var.pref_window);
	var.pref_window=NULL;
	var.apply_button=NULL;
    }	

    g_free(iw);
    g_free(cs);
    g_free(fs);
}

static void 
on_apply_button_clicked(GtkButton *b, gpointer d)
{
    GAI_NOTE(_("apply"));
    gai_pref_get_answers();
    if (GAI.on_preferences_callback)
    {
	GAI.restarting=1;

	if (GAI.on_preferences_callback)
	    GAI.on_preferences_callback(FALSE, GAI.on_preferences_userdata);

	GAI.restarting=0;
    }

    gtk_widget_set_sensitive(var.apply_button,0);
}

static void 
on_ok_button_clicked(GtkButton *b, gpointer d)
{
    on_apply_button_clicked (b,d);
    on_close_button_clicked (b,d);
}

void 
on_help_button_clicked(GtkButton *b, gpointer d)
{
    GtkWidget *w;

    if(GAI.help_text == NULL)
	return;
    w = gtk_message_dialog_new (NULL,0,GTK_MESSAGE_INFO,GTK_BUTTONS_OK,
				GAI.help_text);
    gtk_widget_show (w);
    g_signal_connect_swapped (G_OBJECT(w), "response",
			      G_CALLBACK(gtk_widget_destroy),
			      G_OBJECT(w));
}

static void 
gai_change_colour (GtkImage *image, GdkPixbuf *pixbuf, int r, int g, int b)
{

    char *buf;
    int x,y,w,h, rs,alpha,yshort;

    w =     gdk_pixbuf_get_width (pixbuf);
    h =     gdk_pixbuf_get_height (pixbuf);
    rs =    gdk_pixbuf_get_rowstride (pixbuf);
    alpha = gdk_pixbuf_get_has_alpha (pixbuf);
    
    buf = gdk_pixbuf_get_pixels(pixbuf);

    for (y=2; y < (h-2) ; y++)
    {
	yshort = rs*y;
	for(x=2; x < (w-2) ; x++)
	{
	    buf[yshort+x*(3+alpha)+0]=(char) r;
	    buf[yshort+x*(3+alpha)+1]=(char) g;
	    buf[yshort+x*(3+alpha)+2]=(char) b;
	}
    }

    gtk_image_set_from_pixbuf (image, pixbuf);
}

static void 
on_colour_sel_cancel_clicked(GtkButton *b, int *number)
{
    int i;
    i = number[0];
    gtk_widget_destroy (cs[i].csd);
    cs[i].csd = NULL;
}

/*
static void 
on_option_menu_changed(GtkOptionMenu *m, int *number)
{
    int i;
    i = number[0];
    om[i].result[0] = gtk_option_menu_get_history(GTK_OPTION_MENU(om[i].om));
}
*/

static void 
on_colour_sel_ok_clicked(GtkButton *b, int *number)
{
    int i, alpha;
    GdkColor colour;

    i=number[0];

    cs[i].old_r     = cs[i].r;
    cs[i].old_g     = cs[i].g;
    cs[i].old_b     = cs[i].b;
    cs[i].old_alpha = cs[i].alpha;

    gtk_color_selection_get_current_color(
	GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(cs[i].csd)->colorsel),
	&colour);

    alpha = gtk_color_selection_get_current_alpha(
	GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(cs[i].csd)->colorsel));

    cs[i].alpha = (unsigned char)(alpha>>8);

    cs[i].r=(unsigned char)(colour.red>>8);
    cs[i].g=(unsigned char)(colour.green>>8);
    cs[i].b=(unsigned char)(colour.blue>>8);

    gai_change_colour (cs[i].image, cs[i].pixbuf,cs[i].r,cs[i].g,cs[i].b);

    on_colour_sel_cancel_clicked (b, number);
    apply_button_valid ();
}

static void 
on_fileselector_cancel (GtkWidget *a, int *number)
{
    int i;
    i = number[0];

    if (fs[i].fsd != NULL)
    {
	gtk_widget_destroy (fs[i].fsd);
	fs[i].fsd = NULL;
    }
}

static void 
on_fileselector_ok (GtkWidget *a, int *number)
{
    int i;
    i = number[0];
    gtk_entry_set_text (GTK_ENTRY (fs[i].entry),
			gtk_file_selection_get_filename(
			    GTK_FILE_SELECTION(fs[i].fsd)));

    on_fileselector_cancel(a,number);
    apply_button_valid();

}


static void 
fileselector_button (GtkButton *button, int *number)
{
    int i;

    i = number[0];

    if (fs[i].fsd == NULL)
    {
	fs[i].fsd = gtk_file_selection_new (fs[i].name);

	gtk_file_selection_set_filename(
	    GTK_FILE_SELECTION(fs[i].fsd),
	    gtk_entry_get_text (GTK_ENTRY (fs[i].entry)));
	g_signal_connect (
	    (gpointer)GTK_FILE_SELECTION(fs[i].fsd)->ok_button,
	    "clicked",G_CALLBACK(on_fileselector_ok),number);
	g_signal_connect (
	    (gpointer)GTK_FILE_SELECTION(fs[i].fsd)->cancel_button,
	    "clicked",G_CALLBACK(on_fileselector_cancel),number);
	gtk_widget_show_all(fs[i].fsd);
    }
}

static void 
colourselector_button (GtkButton *button, int *number)
{
    int i;
    GdkColor colour;

    i = number[0];

    if (cs[i].csd == NULL)
    {
	cs[i].csd = gtk_color_selection_dialog_new (cs[i].name);
	gtk_window_set_resizable (GTK_WINDOW (cs[i].csd), FALSE);
	gtk_widget_realize(cs[i].csd);
	
	{
	    register GtkColorSelection* colorsel;

	    colour.red =   ((int)cs[i].r)<<8;
	    colour.green = ((int)cs[i].g)<<8;
	    colour.blue =  ((int)cs[i].b)<<8;

	    colorsel = GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(
					       cs[i].csd) ->colorsel);

	    gtk_color_selection_set_current_color (colorsel, &colour);
	    gtk_color_selection_set_current_alpha (colorsel,
						  (int)(cs[i].alpha <<8));

	    colour.red =   ((int)cs[i].old_r)<<8;
	    colour.green = ((int)cs[i].old_g)<<8;
	    colour.blue =  ((int)cs[i].old_b)<<8;
	
	    gtk_color_selection_set_previous_color (colorsel, &colour);
	    gtk_color_selection_set_previous_alpha (colorsel, 
						   (int)cs[i].old_alpha <<8);
	    gtk_color_selection_set_has_opacity_control (colorsel, TRUE);
	}
	    
	g_signal_connect(
	    (gpointer)GTK_COLOR_SELECTION_DIALOG (cs[i].csd)->ok_button,
	    "clicked", G_CALLBACK (on_colour_sel_ok_clicked), number);
	g_signal_connect(
	    (gpointer)GTK_COLOR_SELECTION_DIALOG(cs[i].csd)->cancel_button,
	    "clicked", G_CALLBACK(on_colour_sel_cancel_clicked), number);
	
	gtk_widget_show_all(cs[i].csd);
    } else {
	gtk_window_present(GTK_WINDOW(cs[i].csd));
    }
}

static GtkWidget *
make_pic_button(const char *text, const char *labeltext, int way)
{
    GtkWidget *label, *image;
    GtkWidget *align, *hbox;
    GtkWidget *button;

    button = gtk_button_new();

    align = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);

    image = gtk_image_new_from_stock (labeltext, GTK_ICON_SIZE_BUTTON);
    hbox = gtk_hbox_new (FALSE, 2);

    label = gtk_label_new(text);
    
    if (way==BUTTON_PIC_LABEL)
    {
	gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0); 
	gtk_box_pack_end (GTK_BOX (hbox), label, FALSE, FALSE, 0);
    } else {
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	gtk_box_pack_end (GTK_BOX (hbox), image, FALSE, FALSE, 0); 
    }

    gtk_container_add (GTK_CONTAINER (button), align);
    gtk_container_add (GTK_CONTAINER (align), hbox);
    
    return button;
}

static GtkWidget *
create_main_buttons(void)
{
    GtkWidget *buttonbar;
    GtkWidget *apply_button, *ok_button, *close_button, *help_button;

    buttonbar = gtk_hbox_new (FALSE, 0);

    ok_button =     make_pic_button(_(" Ok  "),GTK_STOCK_OK,    BUTTON_PIC_LABEL);
    apply_button =  make_pic_button(_("Apply"),GTK_STOCK_APPLY, BUTTON_PIC_LABEL);
    close_button =  make_pic_button(_("Close"),GTK_STOCK_CLOSE, BUTTON_PIC_LABEL);
    help_button =   make_pic_button(_("Help "),GTK_STOCK_HELP,  BUTTON_PIC_LABEL);
    
    gtk_box_pack_start (GTK_BOX (buttonbar), ok_button,    FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (buttonbar), apply_button, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (buttonbar), close_button, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (buttonbar), help_button,  FALSE, FALSE, 0);

    gtk_container_set_border_width (GTK_CONTAINER (ok_button), 5);
    gtk_container_set_border_width (GTK_CONTAINER (apply_button), 5);
    gtk_container_set_border_width (GTK_CONTAINER (close_button), 5);
    gtk_container_set_border_width (GTK_CONTAINER (help_button), 5);

    g_signal_connect(G_OBJECT(ok_button), "clicked",
		     G_CALLBACK(on_ok_button_clicked), NULL);
    g_signal_connect(G_OBJECT(apply_button), "clicked",
		     G_CALLBACK(on_apply_button_clicked), NULL);
    g_signal_connect(G_OBJECT(close_button), "clicked",
		     G_CALLBACK(on_close_button_clicked), NULL);
    g_signal_connect(G_OBJECT(help_button), "clicked",
		     G_CALLBACK(on_help_button_clicked), NULL);

    var.apply_button = apply_button;
    return buttonbar;
}

static GtkWidget *
create_internal_box(GaiBox *b)
{
    int i=0, num_items=0,j,k;
    GtkWidget *table, *label, *frame1, *frame2, *frame3, *hbox, *menu;
    GtkWidget *align, *button_box, *button, *tmp_widget;
    int max_length;
    char *temp_str;

    while (b[num_items].type != GAI_END)
	num_items++;

    table = gtk_table_new(2,num_items,FALSE);

    while(b[i].type != GAI_END)
    {
	if(b[i].type == GAI_CHECKBUTTON)
	{
	    iw[iw_ptr].widget = gtk_check_button_new_with_label(
		((GaiCheckButton *)b[i].ptr)->name);
	    gtk_toggle_button_set_active (
		GTK_TOGGLE_BUTTON(iw[iw_ptr].widget),
		((GaiCheckButton *)b[i].ptr)->default_val[0]);
	    iw[iw_ptr].type = GAI_CHECKBUTTON;
	    iw[iw_ptr].result = (void *)((GaiCheckButton *)b[i].ptr)->result;

	    gtk_table_attach (GTK_TABLE (table), 
			      iw[iw_ptr].widget, 0, 1, i, i+1,
			      (GtkAttachOptions) (GTK_FILL),
			      (GtkAttachOptions) (0), 2, 2);
	    g_signal_connect(G_OBJECT(iw[iw_ptr].widget),"toggled",
			     G_CALLBACK(apply_button_valid), NULL);
	    iw_ptr++;
	}
	else if(b[i].type == GAI_TEXTENTRY)
	{
	    label = gtk_label_new (((GaiTextEntry *)b[i].ptr)->name);
	    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

	    hbox = gtk_hbox_new (FALSE,2);
	    gtk_box_pack_start (GTK_BOX(hbox), label, FALSE,FALSE,0);

	    iw[iw_ptr].type = GAI_TEXTENTRY;
	    iw[iw_ptr].widget = gtk_entry_new();
	    gtk_box_pack_end (GTK_BOX(hbox), iw[iw_ptr].widget, FALSE,FALSE,0);
	    gtk_table_attach (GTK_TABLE (table), hbox,
			      0, 1, i, i+1,
			      (GtkAttachOptions) (GTK_FILL),
			      (GtkAttachOptions) (0), 2, 2);
	    gtk_entry_set_max_length (
		GTK_ENTRY (iw[iw_ptr].widget), 1024);

	    if ((char *)((GaiTextEntry *)b[i].ptr)->default_val[0] != NULL)
		gtk_entry_set_text (
		    GTK_ENTRY (iw[iw_ptr].widget), 
		    (char *)((GaiTextEntry *)b[i].ptr)->default_val[0]);



	    g_signal_connect(G_OBJECT(iw[iw_ptr].widget),"changed",
			     G_CALLBACK(apply_button_valid), NULL);

	    iw[iw_ptr].result = (void *)((GaiTextEntry *)b[i].ptr)->result;
	    iw[iw_ptr].default_val = (void *)((GaiTextEntry *)b[i].ptr)->default_val;
	    iw_ptr++;
	}
	else if(b[i].type == GAI_PASSWORDENTRY)
	{
	    label = gtk_label_new(((GaiTextEntry *)b[i].ptr)->name);
	    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

	    hbox = gtk_hbox_new(FALSE,2);

	    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);

	    /* This is actually a text entry with * display */
	    iw[iw_ptr].type = GAI_TEXTENTRY;
	    iw[iw_ptr].widget = gtk_entry_new();
	    gtk_box_pack_end (GTK_BOX(hbox),iw[iw_ptr].widget,FALSE,FALSE,0);
	    gtk_table_attach (GTK_TABLE (table), hbox,
			      0, 1, i, i+1,
			      (GtkAttachOptions) (GTK_FILL),
			      (GtkAttachOptions) (0), 2, 2);

	    /* Makes it a password entry */
	    gtk_entry_set_visibility (GTK_ENTRY(iw[iw_ptr].widget), FALSE);

	    gtk_entry_set_max_length(GTK_ENTRY(iw[iw_ptr].widget), 1024);

	    if((char *)((GaiTextEntry *)b[i].ptr)->default_val[0] != NULL)
    		gtk_entry_set_text (
		    GTK_ENTRY (iw[iw_ptr].widget), 
		    (char *)((GaiTextEntry *)b[i].ptr)->default_val[0]);

	    g_signal_connect (G_OBJECT(iw[iw_ptr].widget),"changed",
			      G_CALLBACK(apply_button_valid), NULL);

	    iw[iw_ptr].result = (void *)((GaiTextEntry *)b[i].ptr)->result;
	    iw[iw_ptr].default_val = (void *)((GaiTextEntry *)b[i].ptr)->default_val;
	    iw_ptr++;
	}
	else if(b[i].type == GAI_TEXT)
	{
	    iw[iw_ptr].widget = gtk_label_new 
		(((GaiTextEntry *)b[i].ptr)->name);
	    gtk_label_set_justify (
		GTK_LABEL (iw[iw_ptr].widget), GTK_JUSTIFY_LEFT);
	    gtk_table_attach (GTK_TABLE (table), iw[iw_ptr].widget,
			      0, 1, i, i+1,
			      (GtkAttachOptions) (GTK_FILL),
			      (GtkAttachOptions) (0), 2, 2);
	    iw[iw_ptr].type = GAI_TEXT;
	    iw_ptr++;
	}	
	else if(b[i].type == GAI_RADIOBUTTON)
	{
	    iw[iw_ptr].widget = gtk_radio_button_new_with_mnemonic (
		NULL, ((GaiRadioButton *)b[i].ptr)->name);

	    gtk_radio_button_set_group (
		GTK_RADIO_BUTTON(iw[iw_ptr].widget),
		radio_group[((GaiRadioButton *)b[i].ptr)->group_number]);

	    radio_group[((GaiRadioButton *)b[i].ptr)->group_number] = 
		gtk_radio_button_get_group(
		    GTK_RADIO_BUTTON(iw[iw_ptr].widget));

	    if (((GaiRadioButton *)b[i].ptr)->default_val[0])
		gtk_toggle_button_set_active(
		    GTK_TOGGLE_BUTTON(iw[iw_ptr].widget), TRUE);
	    
	    gtk_table_attach (GTK_TABLE(table), iw[iw_ptr].widget,
			      0, 1, i, i+1,
			      (GtkAttachOptions) (GTK_FILL),
			      (GtkAttachOptions) (0), 2, 2);

	    g_signal_connect (G_OBJECT(iw[iw_ptr].widget),"toggled",
			      G_CALLBACK(apply_button_valid), NULL);

	    iw[iw_ptr].type = GAI_RADIOBUTTON;
	    iw[iw_ptr].result = (void *)((GaiRadioButton *)b[i].ptr)->result;
	    iw_ptr++;
	}
	else if(b[i].type == GAI_FRAME)
	{
	    if ((((GaiFrame *)b[i].ptr)->ptr1) != NULL)
	    {
		frame1 = gtk_frame_new (NULL);
		gtk_frame_set_label_widget (
		    GTK_FRAME(frame1),
		    gtk_label_new((((GaiFrame *)b[i].ptr)->name1)));

		tmp_widget = create_internal_box(
		    (GaiBox *)(((GaiFrame *)b[i].ptr)->ptr1));
		iw[iw_ptr].widget = tmp_widget;
		gtk_container_add (GTK_CONTAINER (frame1), iw[iw_ptr].widget);

		gtk_table_attach(GTK_TABLE(table), frame1, 0, 1, i, i+1,
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 
				 5, 5);
		iw_ptr++;
	    }
	    if ((((GaiFrame *)b[i].ptr)->ptr2) != NULL)
	    {
		frame2 = gtk_frame_new (NULL);
		gtk_frame_set_label_widget (
		    GTK_FRAME(frame2),
		    gtk_label_new((((GaiFrame *)b[i].ptr)->name2)));

		tmp_widget = create_internal_box(
		    (GaiBox *)(((GaiFrame *)b[i].ptr)->ptr2));
		iw[iw_ptr].widget = tmp_widget;
		gtk_container_add (GTK_CONTAINER (frame2), iw[iw_ptr].widget);

		gtk_table_attach(GTK_TABLE(table), frame2, 1, 2, i, i+1,
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 
				 5, 5);
		iw_ptr++;
	    }
	}

	if (b[i].type == GAI_FRAME3)
	{
	    gtk_table_resize (GTK_TABLE(table),3,num_items);

	    if ((((GaiFrame3 *)b[i].ptr)->ptr1)!=NULL)
	    {
		frame1 = gtk_frame_new (NULL);
		gtk_frame_set_label_widget(
		    GTK_FRAME(frame1),
		    gtk_label_new((((GaiFrame3 *)b[i].ptr)->name1)));

		tmp_widget = create_internal_box(
		    (GaiBox *)(((GaiFrame3 *)b[i].ptr)->ptr1));
		iw[iw_ptr].widget = tmp_widget;
		gtk_container_add (GTK_CONTAINER (frame1), iw[iw_ptr].widget);
		gtk_table_attach(GTK_TABLE(table), frame1, 0, 1, i, i+1,
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 
				 5, 5);
		iw_ptr++;
	    }

	    if ((((GaiFrame3 *)b[i].ptr)->ptr2)!=NULL)
	    {
		frame2 = gtk_frame_new (NULL);
		gtk_frame_set_label_widget(
		    GTK_FRAME(frame2),
		    gtk_label_new((((GaiFrame3 *)b[i].ptr)->name2)));

		tmp_widget = create_internal_box(
		    (GaiBox *)(((GaiFrame3 *)b[i].ptr)->ptr2));
		iw[iw_ptr].widget = tmp_widget;
		gtk_container_add (GTK_CONTAINER (frame2), iw[iw_ptr].widget);
		gtk_table_attach(GTK_TABLE(table), frame2, 1, 2, i, i+1,
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 
				 5, 5);
		iw_ptr++;
	    }

	    if ((((GaiFrame3 *)b[i].ptr)->ptr3)!=NULL)
	    {
		frame3 = gtk_frame_new (NULL);
		gtk_frame_set_label_widget(
		    GTK_FRAME(frame3),
		    gtk_label_new ((((GaiFrame3 *)b[i].ptr)->name3)));

		tmp_widget = create_internal_box(
		    (GaiBox *)(((GaiFrame3 *)b[i].ptr)->ptr3));
		iw[iw_ptr].widget = tmp_widget;
		gtk_container_add (GTK_CONTAINER (frame3), iw[iw_ptr].widget);
		gtk_table_attach (GTK_TABLE(table), frame3, 2, 3, i, i+1,
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 
				  5, 5);
		iw_ptr++;
	    }
	}
	else if(b[i].type == GAI_SPINBUTTON)
	{
	    label = gtk_label_new (((GaiSpinButton *)b[i].ptr)->name);
	    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

	    hbox = gtk_hbox_new(FALSE,2);

	    gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,0);

	    iw[iw_ptr].type = GAI_SPINBUTTON;
	    iw[iw_ptr].widget = gtk_spin_button_new(
		GTK_ADJUSTMENT(gtk_adjustment_new(
				   ((GaiSpinButton *)b[i].ptr)->default_val[0],
				   ((GaiSpinButton *)b[i].ptr)->min[0],
				   ((GaiSpinButton *)b[i].ptr)->max[0],
				   ((GaiSpinButton *)b[i].ptr)->step[0],
				   10, 10)),1,0);
	    gtk_box_pack_end (GTK_BOX(hbox), iw[iw_ptr].widget,FALSE,FALSE,0);
	    gtk_table_attach (GTK_TABLE (table), hbox,
			      0, 1, i, i+1,
			      (GtkAttachOptions) (GTK_FILL),
			      (GtkAttachOptions) (0), 2, 2);

	    g_signal_connect (G_OBJECT(iw[iw_ptr].widget),"changed",
			      G_CALLBACK(apply_button_valid), NULL);


	    iw[iw_ptr].result = (void *)((GaiSpinButton *)b[i].ptr)->result;
	    iw_ptr++;
	}
	else if(b[i].type == GAI_SPINBUTTONFLOAT)
	{

	    label = gtk_label_new (((GaiSpinButtonFloat *)b[i].ptr)->name);
	    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

	    hbox = gtk_hbox_new(FALSE,2);

	    gtk_box_pack_start (GTK_BOX(hbox),label,FALSE,FALSE,0);

	    iw[iw_ptr].type = GAI_SPINBUTTONFLOAT;


	    iw[iw_ptr].widget = gtk_spin_button_new(
		GTK_ADJUSTMENT(gtk_adjustment_new(
				   ((GaiSpinButtonFloat *)b[i].ptr)->default_val[0],
				   ((GaiSpinButtonFloat *)b[i].ptr)->min[0],
				   ((GaiSpinButtonFloat *)b[i].ptr)->max[0],
				   ((GaiSpinButtonFloat *)b[i].ptr)->step[0],
				   10.0, 10.0)),
		1,((GaiSpinButtonFloat *)b[i].ptr)->decimals[0]);

	    gtk_box_pack_end (GTK_BOX(hbox), iw[iw_ptr].widget,FALSE,FALSE,0);
	    gtk_table_attach (GTK_TABLE (table), hbox,
			      0, 1, i, i+1,
			      (GtkAttachOptions) (GTK_FILL),
			      (GtkAttachOptions) (0), 2, 2);

	    g_signal_connect (G_OBJECT(iw[iw_ptr].widget),"changed",
			      G_CALLBACK(apply_button_valid), NULL);

	    /* This is a float */
	    iw[iw_ptr].result = (void *)((GaiSpinButtonFloat *)b[i].ptr)->result;
	    iw_ptr++;
	}

	else if(b[i].type == GAI_COLOURSELECTOR)
	{
	    cs_ptr = ((GaiColourSelector *)b[i].ptr)->number;
	    cs[cs_ptr].name = ((GaiColourSelector *)b[i].ptr)->name;
	    cs[cs_ptr].button = iw[iw_ptr].widget = gtk_button_new();
	    
	    label = gtk_label_new(cs[cs_ptr].name);
	    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

	    hbox = gtk_hbox_new(FALSE,2);

	    gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,2);

	    align = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
	    button_box = gtk_hbox_new (FALSE, 1);

	    cs[cs_ptr].pixbuf = gdk_pixbuf_new_from_xpm_data(
		(const char **)colourbutton_xpm);

	    cs[cs_ptr].csd = NULL;

	    /* Interesting - save it */
	    cs[cs_ptr].image = GTK_IMAGE(gtk_image_new_from_pixbuf(
					     cs[cs_ptr].pixbuf));
	    cs[cs_ptr].r = ((GaiColourSelector *)b[i].ptr)->r[0];
	    cs[cs_ptr].g = ((GaiColourSelector *)b[i].ptr)->g[0];
	    cs[cs_ptr].b = ((GaiColourSelector *)b[i].ptr)->b[0];
	    cs[cs_ptr].alpha = ((GaiColourSelector *)b[i].ptr)->alpha[0];
	    cs[cs_ptr].old_r = 0xff;
	    cs[cs_ptr].old_g = 0xff;
	    cs[cs_ptr].old_b = 0xff;
	    cs[cs_ptr].old_alpha = 0xff;

	    gai_change_colour(cs[cs_ptr].image, cs[cs_ptr].pixbuf,
			      cs[cs_ptr].r, cs[cs_ptr].g, cs[cs_ptr].b);

	    gtk_box_pack_start (GTK_BOX(button_box), 
				GTK_WIDGET(cs[cs_ptr].image), 
				FALSE, FALSE, 0); 
	    gtk_container_add (GTK_CONTAINER(align), button_box);
	    gtk_container_add (GTK_CONTAINER(iw[iw_ptr].widget), align);

	    gtk_box_pack_end (GTK_BOX(hbox),iw[iw_ptr].widget, FALSE,FALSE,2);

	    //	    gtk_container_add (GTK_CONTAINER (align2),hbox);

	    gtk_table_attach (GTK_TABLE(table), hbox,
			      0, 1, i, i+1,
			      (GtkAttachOptions) (GTK_FILL),
			      (GtkAttachOptions) (0), 2, 2);

	    g_signal_connect (G_OBJECT(iw[iw_ptr].widget),"clicked",
			      G_CALLBACK(colourselector_button),
			      &((GaiColourSelector *)b[i].ptr)->number);

	    iw[iw_ptr].type = GAI_COLOURSELECTOR;
	    iw[iw_ptr].result  = (void *)((GaiColourSelector *)b[i].ptr)->r;
	    iw[iw_ptr].result2 = (void *)((GaiColourSelector *)b[i].ptr)->g;
	    iw[iw_ptr].result3 = (void *)((GaiColourSelector *)b[i].ptr)->b;
	    iw[iw_ptr].result4 = (void *)((GaiColourSelector *)b[i].ptr)->alpha;
	    iw_ptr++;

	}
	else if(b[i].type == GAI_HLINE)
	{
	    gtk_table_attach (GTK_TABLE (table), 
			      GTK_WIDGET(gtk_hseparator_new()),
			      0, 2, i, i+1,
			      (GtkAttachOptions) (GTK_FILL),
			      (GtkAttachOptions) (0), 2, 2);
	}
	else if (b[i].type == GAI_FILESELECTOR)
	{
	    hbox = gtk_hbox_new(FALSE,2);

	    fs_ptr = ((GaiFileSelector *)b[i].ptr)->number;

	    iw[iw_ptr].type = GAI_FILESELECTOR;
	    fs[fs_ptr].entry = iw[iw_ptr].widget = gtk_entry_new();
	    fs[fs_ptr].name = ((GaiFileSelector *)b[i].ptr)->name;
	    gtk_entry_set_max_length (GTK_ENTRY (iw[iw_ptr].widget), 256);

	    if ((char *)((GaiFileSelector *)b[i].ptr)->default_val[0]!=NULL)
		gtk_entry_set_text (
		    GTK_ENTRY (iw[iw_ptr].widget), 
		    (char *)((GaiFileSelector *)b[i].ptr)->default_val[0]);



	    button_box = gtk_hbox_new (FALSE, 1);

	    label = gtk_label_new (((GaiTextEntry *)b[i].ptr)->name);
	    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	    
	    gtk_box_pack_start (GTK_BOX (button_box), label, FALSE, FALSE, 0); 

	    gtk_box_pack_start (GTK_BOX (button_box),
				iw[iw_ptr].widget, FALSE, FALSE, 0); 

	    button = gtk_button_new_with_label("Change");
	    gtk_box_pack_end (GTK_BOX (button_box),button, FALSE, FALSE, 0); 

	    gtk_table_attach (GTK_TABLE (table), button_box,
			      0, 1, i, i+1,
			      (GtkAttachOptions) (GTK_FILL),
			      (GtkAttachOptions) (0), 2, 2);

	    g_signal_connect(G_OBJECT(button),"clicked",
			     G_CALLBACK(fileselector_button),
			     &((GaiFileSelector *)b[i].ptr)->number);

	    iw[iw_ptr].default_val = (void *)((GaiTextEntry *)b[i].ptr)->default_val;
	    iw[iw_ptr].result = (void *)((GaiFileSelector *)b[i].ptr)->result;
	    iw_ptr++;

	}
	else if(b[i].type == GAI_OPTIONMENU)
	{
	    hbox = gtk_hbox_new(FALSE,2);
	    label = gtk_label_new(((GaiOptionMenu *)b[i].ptr)->name);
	    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	    
	    gtk_box_pack_start (GTK_BOX (hbox),label, FALSE, FALSE, 0); 

	    iw[iw_ptr].widget = gtk_option_menu_new();
	    /* om[((GaiOptionMenu *)b[i].ptr)->number].result =
	          ((GaiOptionMenu *)b[i].ptr)->result;
	    */
	
	    menu = gtk_menu_new();
	    max_length = 0;

	    for (j=0 ;; j++)
	    {
		if ((((GaiOptionMenu *)b[i].ptr)->option_list[j]) == NULL) 
		    break;
		if (max_length < strlen(((GaiOptionMenu *)b[i].ptr)->option_list[j]))
		    max_length = strlen(((GaiOptionMenu *)b[i].ptr)->option_list[j]);
	    }
	    /*	    printf("max length: %d\n",max_length); */
	    temp_str = g_malloc0(max_length+20);

	    for (j=0 ;; j++)
	    {
		if ((((GaiOptionMenu *)b[i].ptr)->option_list[j]) == NULL) 
		    break;

		g_snprintf(temp_str, max_length+1, "%s",
			   (((GaiOptionMenu *)b[i].ptr)->option_list[j]));

		if (strlen((((GaiOptionMenu *)b[i].ptr)->option_list[j])) < 
		    max_length)
		{
		    for(k=(strlen((((GaiOptionMenu *)b[i].ptr)->option_list[j])));
			k < max_length ; k++)
			temp_str[k]=' ';
		    temp_str[max_length+1] = '\0';
		}
		gtk_container_add (GTK_CONTAINER(menu),
				   gtk_menu_item_new_with_mnemonic(temp_str));
	    }

	    g_free(temp_str);

	    gtk_option_menu_set_menu (
		GTK_OPTION_MENU(iw[iw_ptr].widget), menu);
	    gtk_option_menu_set_history(
		GTK_OPTION_MENU(iw[iw_ptr].widget), 
		((GaiOptionMenu *)b[i].ptr)->default_val[0]);
	    gtk_box_pack_end(GTK_BOX(hbox),iw[iw_ptr].widget,FALSE,FALSE,0);
	    gtk_table_attach (GTK_TABLE (table), hbox,
			      0, 1, i, i+1,
			      (GtkAttachOptions) (GTK_FILL),
			      (GtkAttachOptions) (0), 2, 2);
	    /*
	    g_signal_connect (G_OBJECT(iw[iw_ptr].widget), "changed",
			      G_CALLBACK (on_option_menu_changed),
			      &((GaiOptionMenu *)b[i].ptr)->number);
	    */

	    g_signal_connect(G_OBJECT(iw[iw_ptr].widget),"changed",
			     G_CALLBACK(apply_button_valid), NULL);

	    iw[iw_ptr].type = GAI_OPTIONMENU;
	    iw[iw_ptr].result = (void *)((GaiOptionMenu *)b[i].ptr)->result;
	    iw_ptr++;
	}

	else if(b[i].type == GAI_COMBO) {
	    label = gtk_label_new (((GaiCombo *)b[i].ptr)->name);
	    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

	    hbox = gtk_hbox_new (FALSE,2);
	    gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE,0);

	    iw[iw_ptr].type = GAI_COMBO;
	    iw[iw_ptr].widget = gtk_combo_new();
	    gtk_box_pack_end (GTK_BOX(hbox), iw[iw_ptr].widget, FALSE, FALSE,0);
	    gtk_table_attach (GTK_TABLE (table), hbox,
			      0, 1, i, i+1,
			      (GtkAttachOptions) (GTK_FILL),
			      (GtkAttachOptions) (0), 2, 2);

	    gtk_entry_set_max_length (
		GTK_ENTRY (GTK_COMBO(iw[iw_ptr].widget)->entry), 1024);

	    gtk_combo_set_popdown_strings(GTK_COMBO(iw[iw_ptr].widget), 
					  ((GaiCombo *)b[i].ptr)->list[0]);


	    if(g_list_nth_data(((GaiCombo *)b[i].ptr)->list[0],
			       (int)((GaiCombo *)b[i].ptr)->val[0]) != NULL)
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(iw[iw_ptr].widget)->entry), 
				   g_list_nth_data(((GaiCombo *)b[i].ptr)->list[0],
						   (int)((GaiCombo *)b[i].ptr)->val[0]));

	    g_signal_connect(G_OBJECT(GTK_COMBO(iw[iw_ptr].widget)->entry),"changed",
			     G_CALLBACK(apply_button_valid), NULL);

	    iw[iw_ptr].result = (void *)((GaiCombo *)b[i].ptr)->list;
	    iw[iw_ptr].result2 = (void *)((GaiCombo *)b[i].ptr)->val;
	    iw_ptr++;
	}

	i++;
    }
    return table;
}

static void 
gai_pref_get_answers(void)
{
    int i, j;
    char *buff;

    GAI_NOTE("Getting answers");

    for(i=0; i < iw_ptr ; i++)
    {
	if(iw[i].type == GAI_CHECKBUTTON || iw[i].type == GAI_RADIOBUTTON)
	{
	    if(((int *)iw[i].result) != NULL)
		*((int *)iw[i].result) = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(iw[i].widget));
	}
	else if (iw[i].type == GAI_TEXTENTRY || iw[i].type == GAI_FILESELECTOR)
	{

	    /* Free if both are the same pointers */
	    if((unsigned char*)(((int *)iw[i].result)[0]) ==
	       (unsigned char*)(((int *)iw[i].default_val)[0])){
		g_free((unsigned char*)(((int *)iw[i].result)[0]));

	    }


	    /* Can surely be done prettier */
	    buff = (char *)gtk_entry_get_text(
		GTK_ENTRY(iw[i].widget));

	    if((unsigned char*)(((int *)iw[i].result)[0]) != NULL &&
	       (unsigned char*)(((int *)iw[i].default_val)[0]) != NULL)
		(((int *)iw[i].result)[0]) = 
		    (((int *)iw[i].default_val)[0]) = g_strdup(buff);


	    //printf("%s\n",buff);
	}
	else if (iw[i].type == GAI_COMBO){


	    buff = (char *)gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(iw[i].widget)->entry));
	    //	    printf("length: %d\n",g_list_length((GList *)((int *)iw[i].result)[0]));

	    for(j=0;j<g_list_length((GList *)((int *)iw[i].result)[0]);j++){

		/* Build the resulting list */
		if(!strcmp(buff,g_list_nth_data((GList *)((int *)iw[i].result)[0],j)))
		    break;
	    }

	    
	    if(j == g_list_length((GList *)((int *)iw[i].result)[0])){
		((int *)iw[i].result)[0] = g_list_append((GList *)((int *)iw[i].result)[0], 
								  g_strdup(buff));
		//		printf("%s is a new entry\n",buff);
	    } 
	    ((int *)iw[i].result2)[0] = j;

	}
	else if (iw[i].type == GAI_SPINBUTTON)
	{
	    if(((int *)iw[i].result) != NULL)
		*((int *)iw[i].result) = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(iw[i].widget));
	}
	else if (iw[i].type == GAI_SPINBUTTONFLOAT)
	{
	    if(((float *)iw[i].result) !=NULL)
		*((float *)iw[i].result) = (float)gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(iw[i].widget));
	}

	else if (iw[i].type == GAI_COLOURSELECTOR)
	{
	    for (j=0;j <= DEFAULT_MAX_WIDGET ; j++)
		if (iw[i].widget == cs[j].button) 
		    break;

	    if (j <= DEFAULT_MAX_WIDGET)
	    {
		if(((unsigned char*)iw[i].result) != NULL)
		    *((unsigned char*)iw[i].result) = cs[j].r;
		if(((unsigned char*)iw[i].result2) != NULL)
		    *((unsigned char*)iw[i].result2) = cs[j].g;
		if(((unsigned char*)iw[i].result3) != NULL)
		    *((unsigned char*)iw[i].result3) = cs[j].b;
		if(((unsigned char*)iw[i].result4) != NULL)
		    *((unsigned char*)iw[i].result4) = cs[j].alpha;

		//printf("j=%d -  %d %d %d\n",j,cs[j].r,cs[j].g,cs[j].b);
	    } else {
		printf(_("GAI: Didn't find colour selection!\n"));
	    }
	}
	else if(iw[i].type == GAI_OPTIONMENU)
	{
	    if(((int *)iw[i].result) != NULL)
		((int *)iw[i].result)[0] = gtk_option_menu_get_history(GTK_OPTION_MENU(iw[i].widget));
	}
    }
    GAI_NOTE("Done getting answers");
}


void 
gai_make_preference_window (const char *window_name, GaiNoteBook *g)
{
    GtkWidget *main_buttons;
    GtkWidget *notebook = NULL, *mainbox;
    GtkWidget *line, *box;
    int i = 0, num_pages = 0;
    
    if (var.pref_window !=NULL)
    {
	gtk_window_present(GTK_WINDOW(var.pref_window));
	return;
    }

    /* gai_data = gai_get_struct_ptr(); */

    iw = g_malloc0 (sizeof(Var_GtkCheckButton) * DEFAULT_MAX_WIDGET);
    max_iw = DEFAULT_MAX_WIDGET;
    iw_ptr = 0;
    
    cs = g_malloc0 (sizeof(Var_GtkColorSelectionDialog) * DEFAULT_MAX_WIDGET);
    max_cs = DEFAULT_MAX_WIDGET;
    cs_ptr = 0;

    fs = g_malloc0 (sizeof(Var_GtkFileSelection) * DEFAULT_MAX_WIDGET);
    fs_ptr = 0;


    //  radio_group = g_malloc0(sizeof(GSList)*DEFAULT_MAX_WIDGET);

    memset(radio_group,0,sizeof(GSList *)*DEFAULT_MAX_WIDGET);

    while(g[num_pages].name !=NULL)
	num_pages++;


    var.pref_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW (var.pref_window), window_name);
    
    mainbox = gtk_vbox_new(FALSE, 0);

    gtk_container_add (GTK_CONTAINER (var.pref_window), mainbox);

    if (num_pages>1)
    {
	notebook = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX(mainbox), notebook, FALSE, FALSE, 5);
	gtk_container_set_border_width(GTK_CONTAINER (notebook), 5);
    }
    
    while (g[i].name != NULL)
    {
	box = create_internal_box((GaiBox *)g[i].ptr);
	if(num_pages>1)
	{
	    gtk_container_add(GTK_CONTAINER(notebook), box);
	    gtk_notebook_set_tab_label(
		GTK_NOTEBOOK(notebook), 
		gtk_notebook_get_nth_page(GTK_NOTEBOOK (notebook), i), 
		gtk_label_new(g[i].name));
	} else
	    gtk_box_pack_start(GTK_BOX(mainbox), box, TRUE, TRUE, 5);
	i++;
    }

    line = gtk_hseparator_new();
    
    gtk_box_pack_start(GTK_BOX(mainbox), line, FALSE, FALSE, 5);
    
    main_buttons = create_main_buttons();
    gtk_box_pack_start(GTK_BOX(mainbox), main_buttons, FALSE, FALSE, 5);
    
    gtk_widget_show_all (var.pref_window);

    //  gtk_widget_realize(var.apply_button);
    gtk_widget_set_sensitive( var.apply_button,0);
}
