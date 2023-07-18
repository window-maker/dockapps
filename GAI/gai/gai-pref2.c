
/*
 * GAI - General Applet Interface Library
 * Copyright (C) 2003-2005 Jonas Aaberg <cja@gmx.net>
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
 * - Preference window generator v2.0
 */


#include <stdio.h>
#include <string.h>
#include "../config.h"
#include "gai.h"
#include "gai-private.h"

#include "colourbutton.xpm"


#define GAI_PREF_MASK 0x3f

#define IS_NONE 0
#define IS_DATA 1
#define IS_LIST 2

typedef struct {
    GtkWidget *widget;
    GaiPrefTypes type;
    void *result, *result2, *default_val;
    int id, group;
    GList *list;
    gboolean changed;
} GaiIW;

/* Selector structure */
typedef struct {
    int type;
    GdkPixbuf *pixbuf;
    GtkImage *image;
    GaiColor color, old_color;
    GtkWidget *sd, *entry;
    char *name;
} GaiSelS;


static GaiSelS *ss;
static GaiIW *iw;
static GtkWidget *pref_window = NULL;
static GtkAdjustment *rox_adj = NULL;
static int num_items = 0, num_notebooks = 0, ptr, iw_ptr = 0, radio_group_number = 0, ss_ptr = 0;
static float align = 0.0;


static GtkWidget *gai_create_page(GaiPI *);

static void gai_pref_update_hash(gpointer new, gpointer old, gint id)
{
    gint val;
    if(old != NULL){
	val = GPOINTER_TO_INT(g_hash_table_lookup(GAI.pref_mem_usage, old));
	if(val != IS_NONE){
	    g_hash_table_remove(GAI.pref_mem_usage, old);
	    if(val == IS_DATA) /* Do not free changed lists - glib takes care of that */
		g_free(old);
	}
    }
    if(new != NULL)
	g_hash_table_insert(GAI.pref_mem_usage, new, (gpointer) id);
}


static void 
gai_change_colour(GtkImage *image, GdkPixbuf *pixbuf, GaiColor c)
{

    unsigned char *buf;
    int x, y, w, h, rs, alpha, yshort;

    w =     gdk_pixbuf_get_width(pixbuf);
    h =     gdk_pixbuf_get_height(pixbuf);
    rs =    gdk_pixbuf_get_rowstride(pixbuf);
    alpha = gdk_pixbuf_get_has_alpha(pixbuf);
    
    buf = gdk_pixbuf_get_pixels(pixbuf);

    for (y=2; y < (h-2) ; y++)
    {
	yshort = rs*y;
	for(x=2; x < (w-2) ; x++)
	{
	    buf[yshort+x*(3+alpha)+0] = c.r;
	    buf[yshort+x*(3+alpha)+1] = c.g;
	    buf[yshort+x*(3+alpha)+2] = c.b;
	}
    }

    gtk_image_set_from_pixbuf(image, pixbuf);
}


#ifdef GTK24

/* FIXME: this order is completly wrong according to all standards!!! */
/* gpointer number and dialog is switched in atleast gtk+ 2.6.4 atleast */
static void on_sel_dialog(gpointer number, GtkResponseType r, GtkDialog *dialog)
{

    void *swap;
    if((int)number > 1000){
	gai_display_error_continue("\t * Your version of GTK seems to be handeling GtkDialogs correctly\n"
				   "\t   Please mail me, <cja@gmx.net> the version of your GTK+ installation. Run:\n"
				   "pkg-config gtk+-2.0 --modversion\n"
				   "\t to get your version of GTK+\n");
	/* Ugly fix that hopefully helps */
	swap = (void *)dialog;
	dialog = (GtkDialog *) number;
	number = (gpointer) swap;
    }

    if(r == GTK_RESPONSE_CANCEL){
	if(ss[(int)number].sd != NULL)
	    gtk_widget_destroy(ss[(int)number].sd);
	ss[(int)number].sd = NULL;
	return;
    }

    if(r == GTK_RESPONSE_ACCEPT){
	gtk_entry_set_text(GTK_ENTRY(ss[(int)number].entry),
			   gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(ss[(int)number].sd)));

	on_sel_dialog(number, GTK_RESPONSE_CANCEL, dialog);
	return;
    }
}

#endif


static void on_sel_cancel_clicked(GtkButton *b, gpointer number)
{

    if(ss[(int)number].sd != NULL)
	gtk_widget_destroy(ss[(int)number].sd);
    ss[(int)number].sd = NULL;
}


static void on_sel_ok_clicked(GtkButton *b, gpointer number)
{
    int alpha;
    GdkColor color;

    if(ss[(int)number].type == GAI_COLORSELECTOR){
	ss[(int)number].old_color = ss[(int)number].color;

	gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(ss[(int)number].sd)->colorsel),
					      &color);

	alpha = gtk_color_selection_get_current_alpha(GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(ss[(int)number].sd)->colorsel));
	ss[(int)number].color.alpha = (unsigned char)(alpha>>8);
	ss[(int)number].color.r = (unsigned char)(color.red>>8);
	ss[(int)number].color.g = (unsigned char)(color.green>>8);
	ss[(int)number].color.b = (unsigned char)(color.blue>>8);

	gai_change_colour(ss[(int)number].image, ss[(int)number].pixbuf, ss[(int)number].color);
    }

    if(ss[(int)number].type == GAI_FILESELECTOR){
	gtk_entry_set_text(GTK_ENTRY(ss[(int)number].entry),
			   gtk_file_selection_get_filename(GTK_FILE_SELECTION(ss[(int)number].sd)));
    }


    on_sel_cancel_clicked(b, number);

}


static void selector_button(GtkButton *b, gpointer number)
{

    GdkColor colour;
    GtkColorSelection *colorsel;


    if (ss[(int)number].sd == NULL)
    {	
	if(ss[(int)number].type == GAI_COLORSELECTOR){
	    ss[(int)number].sd = gtk_color_selection_dialog_new(T(ss[(int)number].name));
	    gtk_window_set_resizable (GTK_WINDOW (ss[(int)number].sd), FALSE);
	    gtk_widget_realize(ss[(int)number].sd);
	
	    colour.red =   ((int)ss[(int)number].color.r)<<8;
	    colour.green = ((int)ss[(int)number].color.g)<<8;
	    colour.blue =  ((int)ss[(int)number].color.b)<<8;

	    colorsel = GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(ss[(int)number].sd) ->colorsel);

	    gtk_color_selection_set_current_color(colorsel, &colour);
	    gtk_color_selection_set_current_alpha(colorsel,(int)(ss[(int)number].color.alpha <<8));
	
	    colour.red =   ((int)ss[(int)number].old_color.r)<<8;
	    colour.green = ((int)ss[(int)number].old_color.g)<<8;
	    colour.blue =  ((int)ss[(int)number].old_color.b)<<8;
	
	    gtk_color_selection_set_previous_color(colorsel, &colour);
	    gtk_color_selection_set_previous_alpha(colorsel, (int)ss[(int)number].old_color.alpha <<8);
	    gtk_color_selection_set_has_opacity_control (colorsel, TRUE);

	    g_signal_connect((gpointer)GTK_COLOR_SELECTION_DIALOG (ss[(int)number].sd)->ok_button,
			     "clicked", G_CALLBACK (on_sel_ok_clicked), number);

	    g_signal_connect((gpointer)GTK_COLOR_SELECTION_DIALOG(ss[(int)number].sd)->cancel_button,
			     "clicked", G_CALLBACK(on_sel_cancel_clicked), number);

	}
	
	if(ss[(int)number].type == GAI_FILESELECTOR){

#ifndef GTK24
	    ss[(int)number].sd = gtk_file_selection_new(T(ss[(int)number].name));

	    gtk_file_selection_set_filename(GTK_FILE_SELECTION(ss[(int)number].sd),
					gtk_entry_get_text(GTK_ENTRY(ss[(int)number].entry)));
	    g_signal_connect((gpointer)GTK_FILE_SELECTION(ss[(int)number].sd)->ok_button,
			     "clicked",G_CALLBACK(on_sel_ok_clicked), number);
	    g_signal_connect((gpointer)GTK_FILE_SELECTION(ss[(int)number].sd)->cancel_button,
			     "clicked",G_CALLBACK(on_sel_cancel_clicked),number);
#else
	    ss[(int)number].sd = gtk_file_chooser_dialog_new(T(ss[(int)number].name),
							     GTK_WINDOW(pref_window),
							     GTK_FILE_CHOOSER_ACTION_OPEN,
							     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
							     GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
							     NULL);
	    g_signal_connect_swapped(ss[(int)number].sd,
				     "response", 
				     G_CALLBACK(on_sel_dialog),
				     number);

#endif
	}


	g_signal_connect(G_OBJECT(ss[(int)number].sd), "destroy", G_CALLBACK(on_sel_cancel_clicked), number);
	gtk_widget_show_all(ss[(int)number].sd);
    } else 
	gtk_window_present(GTK_WINDOW(ss[(int)number].sd));


}


static void on_list_toggle(GtkCellRendererToggle *cell, const gchar *path_str, gpointer d)
{
    GtkTreeModel *model;
    GtkTreeIter  iter;
    GtkTreePath *path;
    gboolean fixed;
    gint column, row, lptr;
    GList *row_list, *entry;


    lptr = GPOINTER_TO_INT(d);

    iw[lptr].changed = TRUE;

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(iw[lptr].widget));
    path = gtk_tree_path_new_from_string(path_str);
    row = gtk_tree_path_get_indices(path)[0];
    column = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(cell), "column"));

    row_list = g_list_nth_data(iw[lptr].list, row);
    entry = g_list_nth(row_list, column);

    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_model_get(model, &iter, column, &fixed, -1);

    gtk_list_store_set(GTK_LIST_STORE(model), &iter, column, !fixed, -1);

    if(entry->data != NULL)
	g_free(entry->data);

    entry->data = g_strdup_printf("%d",!fixed);

    gtk_tree_path_free(path);
}


static void on_list_cell_edited(GtkCellRendererText *cell, const gchar *path_string, const gchar *new_text, gpointer d)
{
    GtkTreeModel *model;
    GtkTreePath *path; 
    GtkTreeIter iter;
    gint column, row, lptr;
    GList *row_list, *entry;

    lptr = GPOINTER_TO_INT(d);
    model =  gtk_tree_view_get_model(GTK_TREE_VIEW(iw[lptr].widget));
    path = gtk_tree_path_new_from_string(path_string);
    row = gtk_tree_path_get_indices(path)[0];
    column = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(cell), "column"));

    row_list = g_list_nth_data(iw[lptr].list, row);
    entry = g_list_nth(row_list, column);


    gtk_tree_model_get_iter(model, &iter, path);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, column, g_strdup(new_text), -1);

    if(entry->data != NULL)
	g_free(entry->data);

    entry->data = g_strdup(new_text);

    iw[lptr].changed = TRUE;

    gtk_tree_path_free (path);
}

static void on_list_add_item(GtkWidget *w, gpointer d)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint local_ptr, columns, j;
    GList *local_list = NULL, *old_list = NULL;

    local_ptr = GPOINTER_TO_INT(d);
    model =  gtk_tree_view_get_model(GTK_TREE_VIEW(iw[local_ptr].widget));
    gtk_list_store_append(GTK_LIST_STORE(model), &iter);


    for(columns = 0; ((GType *)iw[local_ptr].result2)[columns] != -1 ; columns++);
    
    columns--;

    for(j=0;j<columns;j++){

	if(((GType *)iw[local_ptr].result2)[j] == G_TYPE_BOOLEAN)
	    gtk_list_store_set(GTK_LIST_STORE(model), &iter, j, FALSE, -1);
	else
	    gtk_list_store_set(GTK_LIST_STORE(model), &iter, j, "", -1);
	local_list = g_list_append(local_list, NULL);
    }

    old_list = iw[local_ptr].list;
    iw[local_ptr].list = g_list_append(iw[local_ptr].list, local_list);
    gai_pref_update_hash((gpointer)iw[local_ptr].list, (gpointer)old_list, IS_LIST);
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, columns, TRUE, -1);

    iw[local_ptr].changed = TRUE;
}

static void on_list_remove_item(GtkWidget *w, gpointer d)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    GtkTreePath *path; 
    gint row, lptr;	
    GList *old_list;

    lptr = GPOINTER_TO_INT(d);
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(iw[lptr].widget));

    if(gtk_tree_selection_get_selected(selection, NULL, &iter)){

	model =  gtk_tree_view_get_model(GTK_TREE_VIEW(iw[lptr].widget));

	path = gtk_tree_model_get_path (model, &iter);
	row = gtk_tree_path_get_indices(path)[0];

	gtk_list_store_remove(GTK_LIST_STORE(model), &iter);

	old_list = iw[lptr].list;
	iw[lptr].list = g_list_remove(iw[lptr].list, g_list_nth_data(iw[lptr].list, row));
	gai_pref_update_hash((gpointer)iw[lptr].list, (gpointer)old_list, IS_LIST);
    }

    iw[lptr].changed = TRUE;
}



/* ======================================================================== */

static GtkWidget *gai_gen_label(GaiPI *g)
{
    GtkWidget *label;

    if(g->name != NULL)
	label = gtk_label_new(T(g->name));
    else
	label = gtk_label_new(" ");

    if((g->type&GAI_NO_TEXT_MARKUP) != GAI_NO_TEXT_MARKUP)
	gtk_label_set_use_markup(GTK_LABEL(label), TRUE);

    if((g->type&GAI_LEFT) == GAI_LEFT)
	gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
    else
    if((g->type&GAI_RIGHT) == GAI_RIGHT)
	gtk_misc_set_alignment(GTK_MISC(label), 1.0, 0.5);
    else
    if((g->type&GAI_CENTER) == GAI_CENTER)
	gtk_misc_set_alignment(GTK_MISC(label), 0.5, 0.5);
    else
        gtk_misc_set_alignment(GTK_MISC(label), align, 0.5);

    return label;
}


static GtkWidget *gai_gen_checkbutton(GaiPI *g)
{
    GtkWidget *item;

    if(g->name != NULL)
	item = iw[iw_ptr].widget = gtk_check_button_new_with_label(T(g->name));
    else
	item = iw[iw_ptr].widget = gtk_check_button_new_with_label(" ");
    
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(iw[iw_ptr].widget),
				 ((int *)g->default_val)[0]);

    iw[iw_ptr].type = GAI_CHECKBUTTON;
    iw[iw_ptr].result = g->result_val;
    iw[iw_ptr].default_val = g->default_val;
    iw_ptr++;
    ptr++;
    return item;
}


static GtkWidget *gai_gen_textentry(GaiPI *g, int passwd)
{
    GtkWidget *label, *item;
    label = gai_gen_label(g);

    item = gtk_hbox_new (FALSE, 2);
    gtk_box_pack_start(GTK_BOX(item), label, FALSE, TRUE, 0);

    iw[iw_ptr].type = GAI_TEXTENTRY;
    iw[iw_ptr].widget = gtk_entry_new();

    gtk_entry_set_max_length(GTK_ENTRY(iw[iw_ptr].widget), 1024);
    
    if (((char **)(g->default_val))[0] != NULL)
	gtk_entry_set_text(GTK_ENTRY(iw[iw_ptr].widget), 
			   ((char **)(g->default_val))[0]);

    /* Makes it a password entry */
    if(passwd)
	gtk_entry_set_visibility(GTK_ENTRY(iw[iw_ptr].widget), FALSE);


    gtk_box_pack_end(GTK_BOX(item), iw[iw_ptr].widget, TRUE, TRUE,0);


    gtk_label_set_mnemonic_widget(GTK_LABEL(label), iw[iw_ptr].widget);
    iw[iw_ptr].result = g->result_val;
    iw[iw_ptr].default_val = g->default_val;
    iw_ptr++;
    ptr++;
    return item;
}


static void gai_gen_radiobutton(GaiPI *g, int *i, GtkWidget *box)
{
    int j;
    GSList *radio_group = NULL;
	    
    for(j = 0; ; j++){
	if(((char **)g->name)[j] == NULL)
	    break;

	iw[iw_ptr].widget = gtk_radio_button_new_with_mnemonic(NULL, T(((char **)g->name)[j]));

	gtk_radio_button_set_group (GTK_RADIO_BUTTON(iw[iw_ptr].widget), radio_group);

	radio_group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(iw[iw_ptr].widget));

	if(j == ((int *)g->default_val)[0])
	    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(iw[iw_ptr].widget), TRUE);


	gtk_table_attach (GTK_TABLE(box), iw[iw_ptr].widget,
			  0, 1, i[0], i[0]+1,
			  (GtkAttachOptions) (GTK_FILL),
			  (GtkAttachOptions) (0), 2, 2);


	iw[iw_ptr].type = GAI_RADIOBUTTON;
	iw[iw_ptr].group = radio_group_number;
	iw[iw_ptr].result = g->result_val;
	iw[iw_ptr].default_val = g->default_val;
	iw[iw_ptr].id = j;
	i[0]++;
	iw_ptr++;
    }


    /* Just to make the main function uniform */
    i--;

    radio_group_number++;
    /* We can't dealloc the list now. It is needed by the layout.
       Wonder if it deallocates it on widget destroy...*/
    /* g_slist_free(radio_group);*/
    ptr++;
}



static GtkWidget *gai_gen_spinbutton(GaiPI *g, int use_int)
{
    GtkWidget *item, *label;
    label = gai_gen_label(g);

    item = gtk_hbox_new(FALSE,2);

    gtk_box_pack_start (GTK_BOX(item), label, FALSE, TRUE, 0);

    if(use_int){
	iw[iw_ptr].type = GAI_SPINBUTTON;
	iw[iw_ptr].widget = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(((int *)g->default_val)[0],
										  ((GaiSS *)g->extra)->min,
										  ((GaiSS *)g->extra)->max,
										  ((GaiSS *)g->extra)->step, 10, 10)), 1, 0);
    } else {
	    iw[iw_ptr].type = GAI_SPINBUTTON_FLOAT;
	    iw[iw_ptr].widget = gtk_spin_button_new(GTK_ADJUSTMENT(gtk_adjustment_new(((float *)g->default_val)[0],
								   ((GaiSSF *)g->extra)->min,
								   ((GaiSSF *)g->extra)->max,
								   ((GaiSSF *)g->extra)->step, 10.0, 10.0)), 
						    1, ((GaiSSF *)g->extra)->decimals);
    }

    gtk_box_pack_end(GTK_BOX(item), iw[iw_ptr].widget, FALSE, TRUE, 0);
    gtk_label_set_mnemonic_widget(GTK_LABEL(label), iw[iw_ptr].widget);
    iw[iw_ptr].result = g->result_val;
    iw[iw_ptr].default_val = g->default_val;
    iw_ptr++;
    ptr++;
    return item;
}



static GtkWidget *gai_gen_selector(GaiPI *g, int sel_type) 
{
    GtkWidget *item, *label, *button_align, *button_box, *button = NULL;

    label = gai_gen_label(g);

    button_box = gtk_hbox_new(FALSE, 1);
    item = gtk_hbox_new(FALSE, 2);
    button_align = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_box_pack_start(GTK_BOX(item), label, FALSE, FALSE, 2);

    ss[ss_ptr].sd = NULL;
    ss[ss_ptr].name = g_strdup(g->name);
    ss[ss_ptr].type = sel_type;


    if(sel_type == GAI_COLORSELECTOR){

	button = iw[iw_ptr].widget = gtk_button_new();
	ss[ss_ptr].pixbuf = gdk_pixbuf_new_from_xpm_data((const char **)colourbutton_xpm);
	ss[ss_ptr].image = GTK_IMAGE(gtk_image_new_from_pixbuf(ss[ss_ptr].pixbuf));
	ss[ss_ptr].color = ((GaiColor *)g->default_val)[0];
	ss[ss_ptr].old_color.r = ss[ss_ptr].old_color.g = ss[ss_ptr].old_color.b = ss[ss_ptr].old_color.alpha = 0xff; 

	gai_change_colour(ss[ss_ptr].image, ss[ss_ptr].pixbuf, ss[ss_ptr].color);

	gtk_box_pack_start(GTK_BOX(button_box), GTK_WIDGET(ss[ss_ptr].image), FALSE, FALSE, 0); 
	iw[iw_ptr].type = GAI_COLORSELECTOR;

	gtk_container_add(GTK_CONTAINER(button_align), button_box);
	gtk_container_add(GTK_CONTAINER(iw[iw_ptr].widget), button_align);
	gtk_box_pack_end(GTK_BOX(item), iw[iw_ptr].widget, FALSE, FALSE, 2);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), iw[iw_ptr].widget);


    }
    if(sel_type == GAI_FILESELECTOR){

	ss[ss_ptr].entry = iw[iw_ptr].widget = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(iw[iw_ptr].widget), 1024);


	if(((char **)(g->default_val))[0] != NULL)
	    gtk_entry_set_text(GTK_ENTRY(iw[iw_ptr].widget), 
			   ((char **)(g->default_val))[0]);


	gtk_box_pack_start(GTK_BOX(item),
			   iw[iw_ptr].widget, FALSE, FALSE, 0); 

	button = gtk_button_new();

	gtk_container_add(GTK_CONTAINER(button), button_align);
	gtk_container_add(GTK_CONTAINER(button_align), button_box);

	gtk_box_pack_start(GTK_BOX(button_box), 
			   gtk_image_new_from_stock ("gtk-find", GTK_ICON_SIZE_BUTTON),
			   FALSE, FALSE, 0);
	label = gtk_label_new_with_mnemonic(_("Ch_ange"));

	gtk_label_set_mnemonic_widget(GTK_LABEL(label), button);
		
	gtk_box_pack_start(GTK_BOX(button_box), 
			       label,
			       FALSE, FALSE, 0);


	gtk_box_pack_end(GTK_BOX(item), button, FALSE, FALSE, 0); 
	iw[iw_ptr].type = GAI_FILESELECTOR;

    }


    g_signal_connect(G_OBJECT(button),"clicked",
		     G_CALLBACK(selector_button),
		     (void *)ss_ptr);	/* This is abit ugly. Convert an int to a pointer */

    iw[iw_ptr].default_val = g->default_val;
    iw[iw_ptr].result  = g->result_val;
    iw[iw_ptr].result2 = (void *)ss_ptr;
    ss_ptr++;
    ptr++;
    iw_ptr++;
    return item;
}


#ifdef GTK24
static GtkWidget *gai_gen_combo_box(GaiPI *g)
{
    int j;
    GtkWidget *label, *item;

    item = gtk_hbox_new(FALSE, 2);

    label = gai_gen_label(g);

    gtk_box_pack_start(GTK_BOX(item), label, FALSE, TRUE, 0);

    iw[iw_ptr].widget = gtk_combo_box_new_text();

    for(j=0 ;; j++){
	if((((char **)g->extra)[j]) == NULL) 
	    break;
	gtk_combo_box_append_text(GTK_COMBO_BOX (iw[iw_ptr].widget), T(((char **)g->extra)[j]));
    }

    gtk_combo_box_set_active(GTK_COMBO_BOX(iw[iw_ptr].widget), 
					   ((int *)g->default_val)[0]);

    gtk_box_pack_end(GTK_BOX(item), iw[iw_ptr].widget, FALSE, TRUE, 0);

    iw[iw_ptr].type = GAI_COMBOBOX;
    iw[iw_ptr].result = g->result_val;
    iw[iw_ptr].default_val = g->default_val;
    iw_ptr++;
    ptr++;
    return item;
}

#else
static GtkWidget *gai_gen_option_menu(GaiPI *g)
{
    int j;
    GtkWidget *label, *item, *menu;

    item = gtk_hbox_new(FALSE, 2);

    label = gai_gen_label(g);

    gtk_box_pack_start(GTK_BOX(item), label, FALSE, TRUE, 0);

    iw[iw_ptr].widget = gtk_option_menu_new();
	
    menu = gtk_menu_new();

    for (j=0 ;; j++){
	if((((char **)g->extra)[j]) == NULL) 
	    break;
	gtk_container_add(GTK_CONTAINER(menu),
			  gtk_menu_item_new_with_mnemonic(T(((char **)g->extra)[j])));
    }

    gtk_option_menu_set_menu(GTK_OPTION_MENU(iw[iw_ptr].widget), menu);

    gtk_option_menu_set_history(GTK_OPTION_MENU(iw[iw_ptr].widget), 
				((int *)g->default_val)[0]);

    gtk_box_pack_end(GTK_BOX(item), iw[iw_ptr].widget, FALSE, TRUE, 0);

    iw[iw_ptr].type = GAI_OPTIONMENU;
    iw[iw_ptr].result = g->result_val;
    iw[iw_ptr].default_val = g->default_val;
    iw_ptr++;
    ptr++;
    return item;
}
#endif


static GtkWidget *gai_gen_frame(GaiPI *g)
{
    GtkWidget *label, *item;
    item = gtk_frame_new(NULL);

    label = gtk_label_new(T(g[ptr].name));

    gtk_frame_set_label_widget(GTK_FRAME(item), label);

    ptr++;
    gtk_container_add(GTK_CONTAINER(item), gai_create_page(g));

    return item;
}

static GtkWidget *gai_gen_scroll_box(GaiPI *g, GtkPolicyType hpol, GtkPolicyType vpol)
{
    GtkWidget *item, *viewport;
    item = gtk_scrolled_window_new(NULL, NULL); 

    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(item), hpol, vpol);

    viewport = gtk_viewport_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(item), viewport); 

    ptr++;
    gtk_container_add(GTK_CONTAINER(viewport), gai_create_page(g));

    return item;
}



static GtkWidget *gai_gen_button(GaiPI *g, int button_type)
{
    GtkWidget *item, *label, *button_box, *button_align;
    GdkPixbuf *pixbuf;

    item = gtk_hbox_new(FALSE, 2);

    iw[iw_ptr].widget = gtk_button_new();

    label = gai_gen_label(g);


    button_align = gtk_alignment_new(0.5, 0.5, 0.0, 0.0);
    button_box = gtk_hbox_new(FALSE, 2);

    if(button_type == GAI_BUTTON_IMAGE){

	pixbuf = gdk_pixbuf_new_from_file((char *)g->default_val, NULL);

	if(pixbuf != NULL){
	    
	    gtk_box_pack_start(GTK_BOX(button_box), 
			       gtk_image_new_from_pixbuf(pixbuf),
			       FALSE, FALSE, 0); 
	    g_object_unref(pixbuf);
	} 
	iw[iw_ptr].type = GAI_BUTTON_IMAGE;

	g_signal_connect (G_OBJECT(iw[iw_ptr].widget),"clicked",
			  G_CALLBACK(g->result_val), NULL);
    }
    if(button_type == GAI_BUTTON_TEXT){
	iw[iw_ptr].type = GAI_BUTTON_TEXT;
	g_signal_connect (G_OBJECT(iw[iw_ptr].widget),"clicked",
			  G_CALLBACK(g->default_val), NULL);

    }
    if(button_type == GAI_BUTTON_STOCK){
	gtk_box_pack_start(GTK_BOX(button_box), 
			   gtk_image_new_from_stock ((char *)g->default_val, GTK_ICON_SIZE_BUTTON),
			   FALSE, FALSE, 0);
	iw[iw_ptr].type = GAI_BUTTON_STOCK;
	g_signal_connect (G_OBJECT(iw[iw_ptr].widget),"clicked",
			  G_CALLBACK(g->result_val), NULL);
    }



    gtk_box_pack_start(GTK_BOX(button_box), label, FALSE, FALSE, 2);
    gtk_container_add(GTK_CONTAINER(button_align), button_box);
    gtk_container_add(GTK_CONTAINER(iw[iw_ptr].widget), button_align);
    gtk_box_pack_end(GTK_BOX(item),iw[iw_ptr].widget, FALSE, FALSE, 2);

    gtk_label_set_mnemonic_widget(GTK_LABEL(label), iw[iw_ptr].widget);
    ptr++;
    iw_ptr++;
    return item;
}


#ifndef GTK24
static GtkWidget *gai_gen_combo(GaiPI *g)
{
    GtkWidget *label, *item;

    label = gai_gen_label(g);

    item = gtk_hbox_new(FALSE,2);
    gtk_box_pack_start(GTK_BOX(item), label, FALSE, FALSE,0);

    iw[iw_ptr].type = GAI_COMBO;
    iw[iw_ptr].widget = gtk_combo_new();
    gtk_box_pack_end(GTK_BOX(item), iw[iw_ptr].widget, FALSE, FALSE,0);


    gtk_entry_set_max_length(GTK_ENTRY(GTK_COMBO(iw[iw_ptr].widget)->entry), 
			     1024);

    gtk_combo_set_popdown_strings(GTK_COMBO(iw[iw_ptr].widget), 
				  (GList *)((int *)g->extra)[0]);

    gtk_label_set_mnemonic_widget(GTK_LABEL(label), iw[iw_ptr].widget);

    if(g_list_nth_data((GList *)((int *)g->extra)[0],
		       ((int *)g->default_val)[0]) != NULL)
	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(iw[iw_ptr].widget)->entry), 
			   g_list_nth_data((GList *)((int *)g->extra)[0],
					   ((int *)g->default_val)[0]));

    iw[iw_ptr].default_val = g->default_val;
    iw[iw_ptr].result = g->extra;
    iw[iw_ptr].result2 = g->result_val;
    iw_ptr++;
    ptr++;
    return item;
}
#else
static GtkWidget *gai_gen_combo_entry(GaiPI *g)
{
    gint i;
    GtkWidget *label, *item;
    GList *list;

    label = gai_gen_label(g);

    item = gtk_hbox_new(FALSE,2);
    gtk_box_pack_start(GTK_BOX(item), label, FALSE, FALSE,0);

    iw[iw_ptr].type = GAI_COMBOENTRY;


    iw[iw_ptr].widget = gtk_combo_box_entry_new_text();
    gtk_box_pack_end(GTK_BOX(item), iw[iw_ptr].widget, FALSE, FALSE,0);


    gtk_entry_set_max_length(GTK_ENTRY(GTK_BIN(iw[iw_ptr].widget)->child), 
			     1024);
    list = (GList *)((int *)g->extra)[0];

    /* The text is user configure able, so no translation here */
    for(i=0;i<g_list_length(list);i++){
	gtk_combo_box_append_text(GTK_COMBO_BOX(iw[iw_ptr].widget), g_list_nth_data(list, i));
	if(i == ((int *)g->default_val)[0])
	    	gtk_entry_set_text(GTK_ENTRY(GTK_BIN(iw[iw_ptr].widget)->child), 
				   g_list_nth_data(list, i));

    }

    gtk_label_set_mnemonic_widget(GTK_LABEL(label), iw[iw_ptr].widget);

    iw[iw_ptr].default_val = g->default_val;
    iw[iw_ptr].result = g->extra;
    iw[iw_ptr].result2 = g->result_val;
    iw_ptr++;
    ptr++;
    return item;
}


#endif


/* g->name = Names[], g->defaul_val = Defaults[], g->result_val = Results[], g->extra = Types[] */

// GtkTreeView
// model = gtk_tree_view_get_model(GTK_TREE_VIEW(liststore));

static GtkWidget *gai_gen_liststore(GaiPI *g, gboolean editable)
{
    int columns, rows, i, j;
    GtkListStore *store;
    GtkTreeIter iter;
    GtkWidget *liststore, *sw; 
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *list_column = NULL;
    GtkWidget *all, *button_bar, *add_button, *remove_button;
    GType *type_list;
    GList *list = NULL, *list2 = NULL;
    GtkRequisition widget_req;

    g_assert(g->name != NULL);
    g_assert(g->default_val != NULL);
    if(editable)
	g_assert(g->result_val != NULL);
    g_assert(g->extra != NULL);

    for(columns=0;((char **)g->name)[columns] != NULL;columns++);
    for(rows=0;((char **)g->default_val)[rows] != NULL; rows++);


    type_list = g_malloc0((columns+2)*sizeof(GType));
    /* Manual copy */
    for(i=0;i<columns;i++){
	type_list[i] = ((GType *)g->extra)[i];
    }	

    //memcpy(type_list, g->extra, sizeof(GType)*columns);
    type_list[columns] = G_TYPE_BOOLEAN;
    type_list[columns+1] = -1;

    store = gtk_list_store_newv(columns+1, type_list);
    /* Do not free type_list - use it later */

    for(i=0;i<rows;i++)
    {
	gtk_list_store_append(store, &iter);
	list2 = NULL;
	for(j=0;j<columns;j++){
	    if(((GType *)g->extra)[j] == G_TYPE_BOOLEAN)
		gtk_list_store_set(store, &iter, j, atoi(((char ***)g->default_val)[i][j]), -1);
	    if(((GType *)g->extra)[j] == G_TYPE_STRING)
		gtk_list_store_set(store, &iter,
				   j, ((char ***)g->default_val)[i][j], -1);
	    if(editable)
		list2 = g_list_append(list2, g_strdup(((char ***)g->default_val)[i][j]));
	}
	gtk_list_store_set(store, &iter, columns, editable, -1);
	if(editable)
	    list = g_list_append(list, list2);
    }

    liststore = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

    if(!editable)
	gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(liststore), TRUE);

    gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(liststore)),
				GTK_SELECTION_SINGLE);


    sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
					GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
				   GTK_POLICY_NEVER,
				   GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(sw), liststore);

    /* Main widgets done */

    iw[iw_ptr].widget = liststore; /* liststore */
    iw[iw_ptr].result = g->result_val; /* result val*/
    iw[iw_ptr].default_val = g->default_val;
    if(editable) {
	iw[iw_ptr].result2 = type_list; /* types */
	iw[iw_ptr].type = GAI_EDITLISTSTORE;
	iw[iw_ptr].list = list;
	g_hash_table_insert(GAI.pref_mem_usage, list, (gpointer)IS_LIST);
    }
    else {
	iw[iw_ptr].result2 = NULL;
	g_free(type_list);
	iw[iw_ptr].type = GAI_LISTSTORE;
    }



    for(i=0;i<columns;i++){
	if( ((char **)g->name)[i] == NULL)
	    break;


	if(((GType *)g->extra)[i] == G_TYPE_BOOLEAN){
	    renderer = gtk_cell_renderer_toggle_new();
	    list_column = gtk_tree_view_column_new_with_attributes(T(((char **)g->name)[i]),
								   renderer,
								   "active", i,
								   NULL);
	    if(editable){
		g_object_set_data(G_OBJECT(renderer), "column", (gpointer)i);
		g_signal_connect(G_OBJECT(renderer), "toggled",
				 G_CALLBACK (on_list_toggle), (gpointer) iw_ptr);
	    }

	}
	if(((GType *)g->extra)[i] == G_TYPE_STRING) {
	    renderer = gtk_cell_renderer_text_new();
	    list_column = gtk_tree_view_column_new_with_attributes(T(((char **)g->name)[i]),
								   renderer,
								   "text", i,
								   "editable", columns,
								   NULL);
	    if(editable){
		g_object_set_data(G_OBJECT(renderer), "column", (gpointer)i);
		g_signal_connect (renderer, "edited",  G_CALLBACK(on_list_cell_edited), (gpointer) iw_ptr);
	    }
	}


	
	gtk_tree_view_column_set_sort_column_id(list_column, i);
	gtk_tree_view_append_column(GTK_TREE_VIEW(liststore), list_column);
	fflush(NULL);
    }
    

    if(editable){

	all = gtk_vbox_new(FALSE, 1);

	button_bar = gtk_hbox_new(FALSE, 1);

	add_button = gtk_button_new_from_stock("gtk-add");
	remove_button = gtk_button_new_from_stock("gtk-remove");

	gtk_container_set_border_width(GTK_CONTAINER(add_button), 5);
	gtk_container_set_border_width(GTK_CONTAINER(remove_button), 5);

 
	gtk_widget_size_request(add_button, &widget_req);
	gtk_widget_set_size_request(add_button, widget_req.width+5, widget_req.height);

	gtk_widget_size_request(remove_button, &widget_req);
	gtk_widget_set_size_request(remove_button, widget_req.width+5, widget_req.height);

	gtk_box_pack_start(GTK_BOX(button_bar), add_button, FALSE, FALSE, 5);
	gtk_box_pack_end(GTK_BOX(button_bar), remove_button, FALSE, FALSE, 5);


	gtk_box_pack_start(GTK_BOX(all), sw, TRUE, TRUE, 2);
	gtk_box_pack_end(GTK_BOX(all), button_bar, FALSE, FALSE, 2);


	g_signal_connect(G_OBJECT(add_button),"clicked", G_CALLBACK(on_list_add_item), (gpointer)iw_ptr);
	g_signal_connect(G_OBJECT(remove_button),"clicked", G_CALLBACK(on_list_remove_item), (gpointer)iw_ptr);


	/* return all instead */
	sw = all;
    }
    ptr++;
    iw_ptr++;
    return sw;
}


static int gai_get_entries(GaiPI *g, int start)
{
    int i, entries=0, depth = 0;
 
    for(i = start; g[i].type != GAI_END; i++){

	if(g[i].type == GAI_FRAME || 
	   g[i].type == GAI_FRAME_R || 
	   g[i].type == GAI_SCROLL_BOX || 
	   g[i].type == GAI_VSCROLL_BOX || 
	   g[i].type == GAI_HSCROLL_BOX
	   ){
	    if(depth==0)
		if((g[i].type&GAI_PREF_MASK) != GAI_ALL_LEFT ||
		   (g[i].type&GAI_PREF_MASK) != GAI_ALL_RIGHT ||
		   (g[i].type&GAI_PREF_MASK) != GAI_ALL_CENTER ||
		   g[i].type != GAI_NOTEBOOK_E ||
		   g[i].type != GAI_NOTEBOOK)
		    entries++;
	    depth++;
	    continue;
	}

	if((g[i].type == GAI_FRAME_E ||
	    g[i].type == GAI_SCROLL_BOX_E ||
	    g[i].type == GAI_HSCROLL_BOX_E ||
	    g[i].type == GAI_VSCROLL_BOX_E
	    ) && depth>0){
	    depth--;
	    continue;
	}

	if(depth == 0)
	    if((g[i].type&GAI_PREF_MASK) != GAI_ALL_LEFT ||
	       (g[i].type&GAI_PREF_MASK) != GAI_ALL_RIGHT ||
	       (g[i].type&GAI_PREF_MASK) != GAI_ALL_CENTER ||
	       g[i].type != GAI_NOTEBOOK_E ||
	       g[i].type != GAI_NOTEBOOK)

		entries++;

	if(g[i].type == GAI_NOTEBOOK_E ||
	   g[i].type == GAI_FRAME_E ||
	   g[i].type == GAI_SCROLL_BOX_E ||
	   g[i].type == GAI_HSCROLL_BOX_E ||
	   g[i].type == GAI_VSCROLL_BOX_E
	   )
	    break;
    }
    return entries;
}



static GtkWidget *gai_create_page(GaiPI *g)
{
    int i, quit=0,  entries;
    GtkWidget *item = NULL, *box;


    entries = gai_get_entries(g, ptr);

    box = gtk_table_new(2, entries, FALSE);

    i = 0;
    while(g[ptr].type != GAI_END && !quit){

	switch(g[ptr].type&GAI_PREF_MASK){

	case GAI_END:
	    quit=1;
	    break;

	case GAI_CHECKBUTTON:
	    item = gai_gen_checkbutton(&g[ptr]);

	    gtk_table_attach (GTK_TABLE (box), 
			      item, 0, 1, i, i+1,
			      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
			      (GtkAttachOptions) (0), 2, 2);
	    i++;
	    break;
	case GAI_TEXTENTRY:

	    item = gai_gen_textentry(&g[ptr], FALSE);

	    gtk_table_attach (GTK_TABLE (box), item,
			      0, 1, i, i+1,
			      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
			      (GtkAttachOptions) (0), 2, 2);
	    i++;
	    break;
	case GAI_TEXT:
	    item = gai_gen_label(&g[ptr]);

	    gtk_table_attach (GTK_TABLE (box), item,
			      0, 1, i, i+1,
			      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
			      (GtkAttachOptions) (0), 2, 2);
	    ptr++;
	    i++;
	    break;
	case GAI_NOTEBOOK:
	    ptr++;
	    break;
	case GAI_RADIOBUTTON:
	    gai_gen_radiobutton(&g[ptr], &i, box);
	    i++;
	    break;
	case GAI_SPINBUTTON:
	    item = gai_gen_spinbutton(&g[ptr], TRUE);
	    gtk_table_attach(GTK_TABLE(box), item,
			      0, 1, i, i+1,
			      (GtkAttachOptions) (GTK_FILL),
			      (GtkAttachOptions) (0), 2, 2);
	    i++;

	    break;
	case GAI_COLORSELECTOR:
	    item = gai_gen_selector(&g[ptr], GAI_COLORSELECTOR);
	    gtk_table_attach (GTK_TABLE(box), item,
			      0, 1, i, i+1,
			      (GtkAttachOptions) (GTK_FILL),
			      (GtkAttachOptions) (0), 2, 2);
	    i++;
	    break;
	case GAI_HLINE:
	    item = gtk_hseparator_new();
	    gtk_table_attach (GTK_TABLE (box), item,
			      0, 1, i, i+1,
			      (GtkAttachOptions) (GTK_FILL),
			      (GtkAttachOptions) (0), 2, 2);
	    i++;
	    ptr++;
	    break;
	case GAI_FILESELECTOR:
	    item = gai_gen_selector(&g[ptr], GAI_FILESELECTOR);

	    gtk_table_attach(GTK_TABLE(box), item,
			     0, 1, i, i+1,
			     (GtkAttachOptions) (GTK_FILL),
			     (GtkAttachOptions) (0), 2, 2);
	    i++;
	    break;
	case GAI_FRAME:
	    item = gai_gen_frame(g);

	    gtk_table_attach(GTK_TABLE(box), item, 0, 1, i, i+1,
			     (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			     (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 
			     5, 5);
	    i++;
	    break;
	case GAI_FRAME_R:
	    item = gai_gen_frame(g);

	    gtk_table_attach(GTK_TABLE(box), item, 1, 2, i-1, i,
			     (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			     (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 
			     5, 5);

	    i++;
	    break;

	case GAI_SCROLL_BOX:
	    item = gai_gen_scroll_box(g, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	    gtk_table_attach(GTK_TABLE(box), item, 0, 1, i, i+1,
			     (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			     (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 
			     5, 5);

	    i++;
	    break;

	case GAI_HSCROLL_BOX:
	    item = gai_gen_scroll_box(g, GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

	    gtk_table_attach(GTK_TABLE(box), item, 0, 1, i, i+1,
			     (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			     (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 
			     5, 5);

	    i++;
	    break;

	case GAI_VSCROLL_BOX:
	    item = gai_gen_scroll_box(g, GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);

	    gtk_table_attach(GTK_TABLE(box), item, 0, 1, i, i+1,
			     (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			     (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 
			     5, 5);

	    i++;
	    break;



#ifdef GTK24
	case GAI_COMBOBOX:
	    item = gai_gen_combo_box(&g[ptr]);
	    gtk_table_attach(GTK_TABLE (box), item,
			     0, 1, i, i+1,
			     (GtkAttachOptions) (GTK_FILL),
			     (GtkAttachOptions) (0), 2, 2);

	    i++;
	    break;

#else
	case GAI_OPTIONMENU:
	    item = gai_gen_option_menu(&g[ptr]);
	    gtk_table_attach(GTK_TABLE (box), item,
			     0, 1, i, i+1,
			     (GtkAttachOptions) (GTK_FILL),
			     (GtkAttachOptions) (0), 2, 2);

	    i++;
	    break;
#endif
	case GAI_BUTTON_TEXT:

	    item = gai_gen_button(&g[ptr], GAI_BUTTON_TEXT);

	    gtk_table_attach (GTK_TABLE(box), item,
			      0, 1, i, i+1,
			      (GtkAttachOptions) (GTK_FILL),
			      (GtkAttachOptions) (0), 2, 2);
	    i++;
	    break;
	case GAI_BUTTON_IMAGE:
	    item = gai_gen_button(&g[ptr], GAI_BUTTON_IMAGE);
	    gtk_table_attach (GTK_TABLE(box), item,
			      0, 1, i, i+1,
			      (GtkAttachOptions) (GTK_FILL),
			      (GtkAttachOptions) (0), 2, 2);
	    i++;
	    break;
	case GAI_BUTTON_STOCK:
	    item = gai_gen_button(&g[ptr], GAI_BUTTON_STOCK);
	    gtk_table_attach (GTK_TABLE(box), item, 
			      0, 1, i, i+1,
			      (GtkAttachOptions) (GTK_FILL),
			      (GtkAttachOptions) (0), 2, 2);
	    i++;
	    break;
	case GAI_PASSWORDENTRY:
	    item = gai_gen_textentry(&g[ptr], TRUE);
	    gtk_table_attach (GTK_TABLE (box), item,
			      0, 1, i, i+1,
			      (GtkAttachOptions) (GTK_FILL),
			      (GtkAttachOptions) (0), 2, 2);
	    i++;
	    break;
	case GAI_SPINBUTTON_FLOAT:
	    item = gai_gen_spinbutton(&g[ptr], FALSE);
	    gtk_table_attach(GTK_TABLE(box), item,
			      0, 1, i, i+1,
			      (GtkAttachOptions) (GTK_FILL),
			      (GtkAttachOptions) (0), 2, 2);
	    i++;

	    break;
#ifdef GTK24
	case GAI_COMBOENTRY:
	    item = gai_gen_combo_entry(&g[ptr]);
	    gtk_table_attach(GTK_TABLE(box), item,
			     0, 1, i, i+1,
			     (GtkAttachOptions)(GTK_FILL),
			     (GtkAttachOptions)(0), 2, 2);
	    i++;
	    break;
#else
	case GAI_COMBO:
	    item = gai_gen_combo(&g[ptr]);
	    gtk_table_attach(GTK_TABLE(box), item,
			     0, 1, i, i+1,
			     (GtkAttachOptions)(GTK_FILL),
			     (GtkAttachOptions)(0), 2, 2);
	    i++;
	    break;
#endif
	case GAI_LISTSTORE:
	    item = gai_gen_liststore(&g[ptr], FALSE);
	    gtk_table_attach(GTK_TABLE(box), item,
			     0, 1, i, i+1,
			     (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			     (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 
			     5, 5);
	    i++;
	    break;
	case GAI_EDITLISTSTORE:
	    item = gai_gen_liststore(&g[ptr], TRUE);
	    gtk_table_attach(GTK_TABLE(box), item,
			     0, 1, i, i+1,
			     (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			     (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), 
			     5, 5);
	    i++;
	    break;
	case GAI_ALL_LEFT:
	    align = 0.0;
	    ptr++;
	    break;
	case GAI_ALL_CENTER:
	    align = 0.5;
	    ptr++;
	    break;
	case GAI_ALL_RIGHT:
	    align = 1.0;
	    ptr++;
	    break;
	case GAI_FRAME_E:
	    ptr++;
	    quit = 1;
	    break;
	case GAI_SCROLL_BOX_E:
	    ptr++;
	    quit = 1;
	    break;
	case GAI_VSCROLL_BOX_E:
	    ptr++;
	    quit = 1;
	    break;
	case GAI_HSCROLL_BOX_E:
	    ptr++;
	    quit = 1;
	    break;
	case GAI_NOTEBOOK_E:
	    ptr++;
	    quit = 1;
	    break;
	default:
	    printf("Unknown command (%d)!\n",g[i].type&GAI_PREF_MASK);
	    break;
	}

    }


    return box;
}


static gboolean close_down(GtkWidget *w, gpointer d)
{
    char *buff;
    int val = GPOINTER_TO_INT(d);

    if(GAI.applet_type == GAI_ROX){
	gai_size_change(gtk_adjustment_get_value(rox_adj), 0, 0, TRUE, 0);
	buff = g_strdup_printf("%s/rox_panel_size", GAI.applet.name);
	gai_save_int(buff, gtk_adjustment_get_value(rox_adj));
	g_free(buff);
    }


    gtk_widget_destroy(pref_window);
    pref_window = NULL;

    if(GAI.on_preferences_callback){
	GAI.restarting = TRUE;
	if (GAI.on_preferences_callback && w == NULL)
	    GAI.on_preferences_callback(val, GAI.on_preferences_userdata);
	else
	    GAI.on_preferences_callback(FALSE, GAI.on_preferences_userdata);

	GAI.restarting = FALSE;
    }

    if(iw != NULL) {
	g_free(iw);
	iw = NULL;
    }

    if(ss != NULL) {
	g_free(ss);
	ss = NULL;
    }

    return TRUE;
}


static char *gai_pref_free_used(char *mem, char *new)
{
    char *buff;
    buff = g_strdup(new);

    gai_pref_update_hash((gpointer)buff, (gpointer)mem, IS_DATA);
    
    return buff;
}
static void gai_pref_free_previous_list(GList *list)
{
    int i,j;
    GList *list2;
    char *buff;

    if(list != NULL){
	if(g_hash_table_lookup(GAI.pref_mem_usage, list) != NULL){
	    g_hash_table_remove(GAI.pref_mem_usage, list);

	    for(i=0;i<g_list_length(list);i++){
		list2 = g_list_nth_data(list, i);
		for(j=0;j<g_list_length(list2);j++){
		    buff = g_list_nth_data(list2, j);
		    if(buff != NULL)
			g_free(buff);
		}
		g_list_free(list2);

	    }
	    g_list_free(list);

	}	
    }

}

/* 
   And the world is like a shiny diamond,
   the way it glitters if you polish it right,
   if the light should burn and leave you blinded
   there'll be treasure on the other side

   - Masterplan
*/

static gboolean on_close_button_clicked(GtkWidget *w, gpointer d)
{
    int i,j;
    char *buff;
    gboolean changed = FALSE;

    for(i=0;i<iw_ptr;i++){


	switch(iw[i].type){
	    
	case GAI_CHECKBUTTON: /* return: int */
	    if(iw[i].result != NULL)
		((int *)iw[i].result)[0] = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(iw[i].widget));
	    if(((int *)iw[i].result)[0] != ((int *)iw[i].default_val)[0])
		changed = TRUE;
	    break;
	case GAI_RADIOBUTTON: /* return: int */
	    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(iw[i].widget))){
		if(iw[i].result != NULL)
		    ((int *)iw[i].result)[0] = iw[i].id;
		if(((int *)iw[i].result)[0] != ((int *)iw[i].default_val)[0])
		    changed = TRUE;
	    }
	    break;

#ifndef GTK24
	case GAI_COMBO: /* return: int and builds on current list */

	    buff = (char *)gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(iw[i].widget)->entry));
#else
	case GAI_COMBOENTRY: /* return: int and builds on current list */

	    buff = (char *)gtk_entry_get_text(GTK_ENTRY(GTK_BIN(iw[i].widget)->child));

#endif
	    for(j=0;j<g_list_length((GList *)((int *)iw[i].result)[0]);j++){

		/* Build the resulting list */
		if(!strcmp(buff,g_list_nth_data((GList *)((int *)iw[i].result)[0], j)))
		    break;
	    }

	    if(j == g_list_length((GList *)((int *)iw[i].result)[0])){
		((GList **)iw[i].result)[0] = g_list_append((GList *)((int *)iw[i].result)[0],  
								    (gpointer)g_strdup(buff));
		changed = TRUE;
	    }
	    ((int *)iw[i].result2)[0] = j;

	    if(((int *)iw[i].result2)[0] != ((int *)iw[i].default_val)[0])
		changed = TRUE;

	    break;

	case GAI_FILESELECTOR: /* return: char* */
	    if(((float *)iw[i].result) != NULL)
		((char **)iw[i].result)[0] = gai_pref_free_used(((char **)iw[i].result)[0], 
								(char *)gtk_entry_get_text(GTK_ENTRY(iw[i].widget)));
	    if(!strcmp(((char **)iw[i].result)[0], ((char **)iw[i].default_val)[0]))
		changed = TRUE;

	    break;

	case GAI_TEXTENTRY: /* return: char*  */
	    if(((float *)iw[i].result) != NULL)
		((char **)iw[i].result)[0] = gai_pref_free_used(((char **)iw[i].result)[0],
								(char *)gtk_entry_get_text(GTK_ENTRY(iw[i].widget)));
	    if(!strcmp(((char **)iw[i].result)[0], ((char **)iw[i].default_val)[0]))
		changed = TRUE;
	    break;
	case GAI_SPINBUTTON: /* return: int */
	    if(((int *)iw[i].result) != NULL)
		((int *)iw[i].result)[0] = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(iw[i].widget));

	    if(((int *)iw[i].result)[0] != ((int *)iw[i].default_val)[0])
		changed = TRUE;

	    break;
	case GAI_SPINBUTTON_FLOAT: /* return: float */
	    if(((float *)iw[i].result) != NULL)
		((float *)iw[i].result)[0] = gtk_spin_button_get_value_as_float(GTK_SPIN_BUTTON(iw[i].widget));
	    if(((float *)iw[i].result)[0] != ((float *)iw[i].default_val)[0])
		changed = TRUE;

	    break;
	case GAI_COLORSELECTOR: /* return: GaiColor */
	    if(((float *)iw[i].result) != NULL)
		((GaiColor *)iw[i].result)[0] = ss[(int)iw[i].result2].color;

	    if(((GaiColor *)iw[i].result)[0].r != ((GaiColor *)iw[i].default_val)[0].r ||
	       ((GaiColor *)iw[i].result)[0].g != ((GaiColor *)iw[i].default_val)[0].g ||
	       ((GaiColor *)iw[i].result)[0].b != ((GaiColor *)iw[i].default_val)[0].b ||
	       ((GaiColor *)iw[i].result)[0].alpha != ((GaiColor *)iw[i].default_val)[0].alpha)
		changed = TRUE;

	    break;

#ifdef GTK24
	case GAI_COMBOBOX: /* return int */
	    if(((int *)iw[i].result) != NULL)
		((int *)iw[i].result)[0] = gtk_combo_box_get_active(GTK_COMBO_BOX(iw[i].widget));

	    if(((int *)iw[i].result)[0] != ((int *)iw[i].default_val)[0])
		changed = TRUE;
	    
	    break;

#else
	case GAI_OPTIONMENU: /* return int */
	    if(((int *)iw[i].result) != NULL)
		((int *)iw[i].result)[0] = gtk_option_menu_get_history(GTK_OPTION_MENU(iw[i].widget));

	    if(((int *)iw[i].result)[0] != ((int *)iw[i].default_val)[0])
		changed = TRUE;
	    
	    break;
#endif
	case GAI_EDITLISTSTORE: /* return 2d list */

	    if(((GList *)iw[i].result) != NULL)
		gai_pref_free_previous_list(((GList **)iw[i].result)[0]);

	    ((GList **)iw[i].result)[0] = iw[i].list;
	    if(iw[i].result2 != NULL)
		g_free(iw[i].result2);
	    changed = iw[i].changed;
	    break;


	default:
	    break;
	}

    }

    close_down(NULL, (gpointer)changed);

    return TRUE;
}

static gboolean on_help_button_clicked(GtkWidget *widget, gpointer d)
{
    GtkWidget *w;

    if(GAI.help_text == NULL)
	return FALSE;

    w = gtk_message_dialog_new (NULL,0,GTK_MESSAGE_INFO,GTK_BUTTONS_OK,
				GAI.help_text);
    gtk_widget_show (w);
    g_signal_connect_swapped(G_OBJECT(w), "response",
			     G_CALLBACK(gtk_widget_destroy),
			     G_OBJECT(w));

    return TRUE;
}

static GtkWidget *
gai_create_main_buttons(void)
{
    GtkWidget *buttonbar, *close_button, *help_button;
    GtkRequisition widget_req;

    buttonbar = gtk_hbox_new(FALSE, 0);

    help_button = gtk_button_new_from_stock("gtk-help");
    close_button = gtk_button_new_from_stock("gtk-close");

    gtk_container_set_border_width(GTK_CONTAINER(help_button), 5);
    gtk_container_set_border_width(GTK_CONTAINER(close_button), 5);

    gtk_widget_size_request(help_button, &widget_req);
    gtk_widget_set_size_request(help_button, widget_req.width+5, widget_req.height);

    gtk_widget_size_request(close_button, &widget_req);
    gtk_widget_set_size_request(close_button, widget_req.width+5, widget_req.height);


    gtk_box_pack_start(GTK_BOX(buttonbar), help_button, FALSE, FALSE, 5);
    gtk_box_pack_end(GTK_BOX(buttonbar), close_button, FALSE, FALSE, 5);

    g_signal_connect(G_OBJECT(help_button), "clicked",
		     G_CALLBACK(on_help_button_clicked), NULL);

    g_signal_connect(G_OBJECT(close_button), "clicked",
		     G_CALLBACK(on_close_button_clicked), NULL);


    return buttonbar;
}

void 
gai_make_preference_window2(const char *window_name, GaiPI *g)
{
    GtkWidget *page, *mainbox, *notebook, *label;
    GtkWidget *rox_box, *rox_frame, *rox_scale;
    int i, old_ptr, j, num_ss = 0, num_iw = 0;

    GAI_ENTER;

    if(pref_window != NULL){
	gtk_window_present(GTK_WINDOW(pref_window));
	GAI_LEAVE;
	return;
    }

    iw_ptr = 0;
    ss_ptr = 0;
    num_items = 0;
    num_notebooks=0;
    radio_group_number = 0;

    while(g[num_items].type != GAI_END){
	if(g[num_items].type == GAI_NOTEBOOK) 
	    num_notebooks++;
	num_items++;
    }


    for(i=0;i<num_items;i++){
	if((g[i].type&GAI_PREF_MASK) == GAI_CHECKBUTTON ||
	   (g[i].type&GAI_PREF_MASK) == GAI_TEXTENTRY ||
	   (g[i].type&GAI_PREF_MASK) == GAI_RADIOBUTTON ||
	   (g[i].type&GAI_PREF_MASK) == GAI_SPINBUTTON ||
	   (g[i].type&GAI_PREF_MASK) == GAI_COLORSELECTOR ||
	   (g[i].type&GAI_PREF_MASK) == GAI_FILESELECTOR ||
	   (g[i].type&GAI_PREF_MASK) == GAI_OPTIONMENU ||
	   (g[i].type&GAI_PREF_MASK) == GAI_COMBOBOX ||
	   (g[i].type&GAI_PREF_MASK) == GAI_BUTTON_TEXT ||
	   (g[i].type&GAI_PREF_MASK) == GAI_BUTTON_STOCK ||
	   (g[i].type&GAI_PREF_MASK) == GAI_BUTTON_IMAGE ||
	   (g[i].type&GAI_PREF_MASK) == GAI_PASSWORDENTRY ||
	   (g[i].type&GAI_PREF_MASK) == GAI_SPINBUTTON_FLOAT ||
	   (g[i].type&GAI_PREF_MASK) == GAI_LISTSTORE ||
	   (g[i].type&GAI_PREF_MASK) == GAI_EDITLISTSTORE ||
	   (g[i].type&GAI_PREF_MASK) == GAI_COMBO ||
	   (g[i].type&GAI_PREF_MASK) == GAI_COMBOENTRY)
	num_iw++;

	if((g[i].type&GAI_PREF_MASK) == GAI_COLORSELECTOR || 
	   (g[i].type&GAI_PREF_MASK) == GAI_FILESELECTOR)
	    num_ss++;


	if((g[i].type&GAI_PREF_MASK) == GAI_RADIOBUTTON){
	    for(j=0;;j++){
		if(((char **)g[i].name)[j] == NULL) break;
		num_iw++;
	    }
	    num_iw--;
	} 
	    

    }

    iw = g_malloc0(sizeof(GaiIW)*num_iw);
    ss = g_malloc0(sizeof(GaiSelS)*num_ss);

    ss_ptr = 0;

    pref_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(pref_window), T(window_name));

    mainbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(pref_window), mainbox);


    if(num_notebooks){
	if(g[0].type != GAI_NOTEBOOK){
	    printf(_("If you want to use Notebooks, the first entry must be a GAI_NOTEBOOK\n"));
	    GAI_NOTE(_("If you want to use Notebooks, the first entry must be a GAI_NOTEBOOK\n"));
	    GAI_LEAVE;
	    return;
	}
	notebook = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX(mainbox), notebook, TRUE, TRUE, 5);
	gtk_container_set_border_width(GTK_CONTAINER (notebook), 5);

	ptr = 1;
	old_ptr = 0;
	i = 0;
	while(g[ptr].type !=GAI_END){
	    page = gai_create_page(g);

	    gtk_container_add(GTK_CONTAINER(notebook), page);

	    label = gtk_label_new_with_mnemonic(T(g[old_ptr].name));

	    gtk_notebook_set_tab_label(GTK_NOTEBOOK(notebook), 
				       gtk_notebook_get_nth_page(GTK_NOTEBOOK (notebook), i), 
				       label);

	    old_ptr = ptr;
	    i++;

	    if(g[ptr].type == GAI_END)
		break;

	    if(g[ptr].type != GAI_NOTEBOOK){
		printf(_("A GAI_NOTEBOOK_E must be followed by a new GAI_NOTEBOOK!\n"));
		GAI_NOTE(_("A GAI_NOTEBOOK_E must be followed by a new GAI_NOTEBOOK!\n"));
		break;
	    }
	}
    } else {
	ptr = 0;
	page = gai_create_page(g);
	gtk_box_pack_start(GTK_BOX(mainbox), page, TRUE, TRUE, 5);
    }

    
    if(GAI.applet_type == GAI_ROX){

	rox_frame = gtk_frame_new(NULL);
	gtk_container_set_border_width(GTK_CONTAINER (rox_frame), 5);
	gtk_box_pack_start(GTK_BOX(mainbox), rox_frame, TRUE, TRUE, 5);
	gtk_frame_set_label_widget(GTK_FRAME(rox_frame), gtk_label_new(_("ROX settings")));

	rox_box = gtk_hbox_new(FALSE,2);
	gtk_box_pack_start(GTK_BOX(rox_box), gtk_label_new(_("Applet size:")), TRUE, TRUE, 5);

	if(GAI.orient == GAI_VERTICAL)
	    rox_adj = GTK_ADJUSTMENT(gtk_adjustment_new(GAI.width, 16 , 128, 0, 0, 0));
	else
	    rox_adj = GTK_ADJUSTMENT(gtk_adjustment_new(GAI.height, 16 , 128, 0, 0, 0));

	rox_scale = gtk_hscale_new(rox_adj);
	gtk_scale_set_digits(GTK_SCALE(rox_scale), 0);
	gtk_box_pack_start(GTK_BOX(rox_box), rox_scale, TRUE, TRUE, 5);

	gtk_container_add(GTK_CONTAINER(rox_frame), rox_box);

    }
    
    gtk_box_pack_end(GTK_BOX(mainbox), gai_create_main_buttons(), FALSE, FALSE, 5);

    g_signal_connect(G_OBJECT(pref_window), "destroy", G_CALLBACK(close_down), NULL);
		     
    gtk_widget_show_all(pref_window);

    GAI_LEAVE;
}
