/*
 *
 * gkrellm_hdplop.c
 * dae (jolly.frederic@fnac.net)
 *
 * gkrellm plugin for wmhdplop:
 * http://hules.free.fr/wmhdplop/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *     02111-1307, USA.
 *
 */

#include <libgen.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>

#include "config.h"
#include "global.h"
#include "wmhdplop.h"
#define PLUGIN_VERSION	VERSION

#define PLUGIN_NAME	"gkhdplop"
#define PLUGIN_DESC	"wmhdplop gkrellm port"
#define PLUGIN_URL	"http://hules.free.fr/wmhdplop/"
#define PLUGIN_STYLE	PLUGIN_NAME
#define PLUGIN_KEYWORD	PLUGIN_NAME


static GkrellmMonitor *mon = NULL;
static GkrellmChart *chart = NULL;
static GkrellmChartconfig *chart_config = NULL;

#define TIMER1 50            /* main updates every 40 ms */
#define TIMER2 50 * TIMER1   /* small updates every 1600 ms */

#define PLUGIN_HEIGHT 64

static gint timeout_id;
static int option_timer = 0;

/* Options stuffs */
GtkWidget *swap_check = NULL;
GtkWidget *io_check = NULL;
GtkWidget *leds_check = NULL;
GtkWidget *colormap = NULL;
GtkWidget *hdlist_check = NULL;
GtkWidget *throughput_threshold = NULL;
GtkWidget *hddtemp_check = NULL;
GtkWidget *entry_smallfont = NULL, *entry_bigfont = NULL;

static gboolean update_plugin(void)
{
  GdkEventExpose event;
  gint ret_val;
  gtk_signal_emit_by_name(GTK_OBJECT(chart->drawing_area), "expose_event", &event, &ret_val);
  return TRUE;	/* restart timer */
}

static gint chart_expose_event(GtkWidget *widget UNUSED, GdkEventExpose *ev UNUSED)
{
  int update_options = 0;
  option_timer++;
  if (option_timer == TIMER2 / TIMER1) {
    option_timer = 0;
    update_options = 1;
  }
  gkrellm_hdplop_update(update_options);
  gkrellm_draw_chart_to_screen(chart);
  return TRUE;
}

static gint wheel_event(GtkWidget *widget UNUSED, GdkEventScroll *ev)
{
  if (ev->direction == GDK_SCROLL_UP)
    next_displayed_hd(); //change_displayed_hd(+1);
  else if (ev->direction == GDK_SCROLL_DOWN)
    prev_displayed_hd(); //change_displayed_hd(-1);
  return TRUE;
}

static gint button_release_event(GtkWidget *widget UNUSED, GdkEventButton *ev, gpointer data UNUSED)
{
  if (ev->button == 3) {
    gkrellm_open_config_window(mon);
  } else if (ev->button == 1) {
    change_displayed_hd(+1);
  }
  return TRUE;
}

static void disable_plugin(void)
{
  if (timeout_id)
    gtk_timeout_remove(timeout_id);
  timeout_id = 0;
}

static void create_plugin(GtkWidget *vbox, gint first_create)
{
  if(first_create) {
    chart = gkrellm_chart_new0();
  }

  gkrellm_set_chart_height_default(chart, PLUGIN_HEIGHT);
  gkrellm_chart_create(vbox, mon, chart, &chart_config);

  if (first_create) {
    hdplop_main(chart->w, chart->h, chart->drawing_area->window);
  } else {
    dockimlib2_gkrellm_xinit(app->dock, chart->drawing_area->window);
    reshape(chart->w, chart->h);
  }
  if (first_create) {
    //printf("chart : w=%d, h=%d\n", chart->w, chart->h);
    gtk_signal_connect(GTK_OBJECT(chart->drawing_area),
		       "expose_event", (GtkSignalFunc) chart_expose_event, NULL);
    gtk_signal_connect(GTK_OBJECT(chart->drawing_area),
		       "button_release_event", GTK_SIGNAL_FUNC(button_release_event), NULL);
    g_signal_connect(G_OBJECT(chart->drawing_area),
		       "scroll_event", G_CALLBACK(wheel_event), NULL);
  }
  /* Update plugin every TIMER1 ms */
  if (!timeout_id)
    timeout_id = g_timeout_add(TIMER1, (GtkFunction) update_plugin, NULL);
  gkrellm_disable_plugin_connect(mon, disable_plugin);
}

static void option_toggled_cb(GtkToggleButton *button, gpointer user_data UNUSED)
{
  gboolean active = gtk_toggle_button_get_active(button);
  GtkWidget *togglebutton = GTK_WIDGET(button);

  if (togglebutton == swap_check) {
     Prefs.disable_swap_matrix = !active;
  }
  else if (togglebutton == io_check) {
    Prefs.disable_io_matrix = !active;
  }
  else if (togglebutton == hdlist_check) {
    if (active) {
      Prefs.hdlist_pos = AL_BOTTOM + AL_LEFT;
      gtk_widget_set_sensitive(leds_check, TRUE);
      gtk_widget_set_sensitive(hddtemp_check, TRUE);
    }
    else {
      Prefs.hdlist_pos = AL_NONE;
      gtk_widget_set_sensitive(leds_check, FALSE);
      gtk_widget_set_sensitive(hddtemp_check, FALSE);
    }
  }
  else if (togglebutton == leds_check) {
    Prefs.disable_hd_leds = !active;
  }
  else if (togglebutton == hddtemp_check) {
    Prefs.enable_hddtemp = active;
    app->displayed_hd_changed = 1;
  }
}

static void cb_colormap_modified(GtkWidget *widget UNUSED, GtkSpinButton *spin)
{
  Prefs.iomatrix_colormap = gtk_spin_button_get_value_as_int(spin);
}

static void cb_spinbutton_modified(GtkWidget *widget UNUSED, GtkSpinButton *spin)
{
  if (GTK_WIDGET(spin) == throughput_threshold) {
    Prefs.popup_throughput_threshold = gtk_spin_button_get_value_as_float(spin);
  }
}

static void cb_reload_fonts(GtkWidget *widget) {
  (void) widget;
  if (strcmp(gtk_entry_get_text(GTK_ENTRY(entry_smallfont)), app->current_smallfont_name) ||
      strcmp(gtk_entry_get_text(GTK_ENTRY(entry_bigfont)), app->current_bigfont_name)) {
    ASSIGN_STRING(Prefs.smallfontname, gtk_entry_get_text(GTK_ENTRY(entry_smallfont)));
    ASSIGN_STRING(Prefs.bigfontname, gtk_entry_get_text(GTK_ENTRY(entry_bigfont)));
    init_fonts(app);
    if (app->smallfont == NULL) {
      gkrellm_config_message_dialog("font problem..", "could not load the small font");
    } else gtk_entry_set_text(GTK_ENTRY(entry_smallfont), app->current_smallfont_name);
    if (app->bigfont == NULL) {
      gkrellm_config_message_dialog("font problem..", "could not load the big font");
    } else gtk_entry_set_text(GTK_ENTRY(entry_bigfont), app->current_bigfont_name);
    /* force recomputations of some dimensions */
    app->displayed_hd_changed = 1; app->reshape_cnt++;
  }
}

static void create_plugin_tab(GtkWidget *tab_vbox)
{
  GtkWidget *tabs = NULL;
  GtkWidget *options_tab = NULL;
  GtkWidget *info_tab = NULL;
  GtkWidget *info = NULL;
  GtkWidget *about_tab = NULL;
  GtkWidget *about = NULL;
  GtkWidget *main_box;

  static gchar *info_text[] =
    {
      "<b>" PLUGIN_NAME "\n\n",
      PLUGIN_DESC "\n\n",
      "improve your productivity with " PLUGIN_NAME ":\n",
      "It monitors your hard-drives by sending visual stimuli to your cortex\n",
      "each time your /dev/hdx writes or reads anything.\n",
      "Try to launch openoffice and enjoy the gkhdplop show!\n\n",
      "<i>Usage:\n\n",
      "- Change the hard drive with the mouse wheel\n",
      "- Animation reflecting swap activity...\n",
      "- Background animation reflecting disk activity...\n",
      "- Small led indicating disk activity...\n",
      "- Several glittering color schemes...\n",
      "- Popup display with the io throughput...\n",
    };

  static gchar *about_text =
    _(
      PLUGIN_NAME " " PLUGIN_VERSION "\n"
      "a " PLUGIN_DESC "\n\n"
      "Copyright (c) 2004 dae\n"
      "jolly.frederic@fnac.net\n"
      "http://quadaemon.free.fr\n\n"
      "Based on wmhdplop\nCopyright (c) 2003,2004 pouaite\n"
      PLUGIN_URL "\n\n"
      "Released under the GNU Public Licence"
      );

  tabs = gtk_notebook_new();
  gtk_notebook_set_tab_pos(GTK_NOTEBOOK(tabs), GTK_POS_TOP);
  gtk_box_pack_start(GTK_BOX(tab_vbox), tabs, TRUE, TRUE, 0);

  /* Options tab */
  options_tab = gkrellm_gtk_notebook_page(tabs, _("Options"));

  main_box = gtk_vbox_new(FALSE, 0);
  gtk_widget_set_name(main_box, "main_box");
  gtk_widget_ref(main_box);
  gtk_object_set_data_full(GTK_OBJECT (options_tab), "main_box", main_box,
			   (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show(main_box);
  gtk_container_add(GTK_CONTAINER (options_tab), main_box);

  /* io button */
  gkrellm_gtk_check_button_connected(main_box, &io_check, !Prefs.disable_io_matrix, 1, 1, 0,
				     option_toggled_cb, NULL, _("Show disk activity"));
  /* swap button */
  gkrellm_gtk_check_button_connected(main_box, &swap_check, !Prefs.disable_swap_matrix, 1, 1, 0,
				     option_toggled_cb, NULL, _("Show swap activity"));
  /* hdlist button */
  gkrellm_gtk_check_button_connected(main_box, &hdlist_check,
				     (Prefs.hdlist_pos == AL_NONE)?FALSE:TRUE, 1, 1, 0,
				     option_toggled_cb, NULL, _("Show disk name"));
  /* leds button */
  gkrellm_gtk_check_button_connected(main_box, &leds_check, !Prefs.disable_hd_leds, 1, 1, 0,
				     option_toggled_cb, NULL, _("Led indicating disk activity"));
  gtk_widget_set_sensitive(leds_check, (Prefs.hdlist_pos == AL_NONE)?FALSE:TRUE);
  /* hddtemp button */
  gkrellm_gtk_check_button_connected(main_box, &hddtemp_check, Prefs.enable_hddtemp, 1, 1, 0,
				     option_toggled_cb, NULL,
				     _("Display hd temperature (requires hddtemp daemon running on port 7634)"));
  gtk_widget_set_sensitive(hddtemp_check, (Prefs.hdlist_pos == AL_NONE)?FALSE:TRUE);
  /* colormap spin button */
  gkrellm_gtk_spin_button(main_box, NULL, Prefs.iomatrix_colormap, 0, 4, 1, 1, 0, 0,
			  cb_colormap_modified, NULL, FALSE, _("Colormap"));

  /* threshold spin button */
  gkrellm_gtk_spin_button(main_box, &throughput_threshold, Prefs.popup_throughput_threshold, 0., 500, 0.1, 1, 1, 0,
                          cb_spinbutton_modified, NULL, FALSE, _("minimum io throughput (MB/s)"));

  {
    GtkWidget *frame = gtk_frame_new("Fonts");
    gtk_box_pack_start(GTK_BOX(main_box), frame, TRUE, FALSE, 0);

    GtkWidget *hbox0 = gtk_hbox_new(FALSE, 4);
    gtk_container_add(GTK_CONTAINER(frame), hbox0);

    GtkWidget *vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox0), vbox, TRUE, FALSE, 0);

    GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, FALSE, 0);
    entry_smallfont = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox), entry_smallfont, TRUE, FALSE, 0);
    GtkWidget *pLabel = gtk_label_new("Small font (without .ttf extension):");
    gtk_entry_set_text(GTK_ENTRY(entry_smallfont), app->current_smallfont_name);
    gtk_box_pack_start(GTK_BOX(hbox), pLabel, TRUE, FALSE, 0);

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, FALSE, 0);
    entry_bigfont = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox), entry_bigfont, TRUE, FALSE, 0);
    gtk_entry_set_text(GTK_ENTRY(entry_bigfont), app->current_bigfont_name);
    pLabel = gtk_label_new("Big font (without .ttf extension)");
    gtk_box_pack_start(GTK_BOX(hbox), pLabel, TRUE, FALSE, 0);

    gkrellm_gtk_button_connected(hbox0, NULL, TRUE/* expand*/, TRUE /* fill*/, 0 /*pad*/,
                                 G_CALLBACK(cb_reload_fonts), NULL, "reload fonts");
  }

  /* Info tab */
  info_tab = gkrellm_gtk_framed_notebook_page(tabs, _("Info"));
  info = gkrellm_gtk_scrolled_text_view(info_tab, NULL, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  gkrellm_gtk_text_view_append_strings(info, info_text, sizeof(info_text) / sizeof(gchar *));

  /* About tab */
  about_tab = gkrellm_gtk_notebook_page(tabs, _("About"));
  about = gtk_label_new(about_text);
  gtk_box_pack_start(GTK_BOX(about_tab), about, TRUE, TRUE, 0);
}

static void save_plugin_config(FILE *f)
{
  fprintf(f, "%s options %d %d %d %d %d %d %f smallfont=%s bigfont=%s\n", PLUGIN_KEYWORD,
	  Prefs.disable_swap_matrix,
	  Prefs.disable_io_matrix,
	  Prefs.disable_hd_leds,
	  Prefs.hdlist_pos,
	  Prefs.enable_hddtemp,
	  Prefs.iomatrix_colormap,
	  Prefs.popup_throughput_threshold,
          app->current_smallfont_name, app->current_bigfont_name);
}

static void load_plugin_config(gchar *config_line)
{
  char sf[1000], bf[1000]; sf[0] = bf[0] = 0;
  int n =sscanf(config_line, "options %d %d %d %d %d %d %f smallfont=%1000s bigfont=%1000s",
                &Prefs.disable_swap_matrix,
                &Prefs.disable_io_matrix,
                &Prefs.disable_hd_leds,
                &Prefs.hdlist_pos,
                &Prefs.enable_hddtemp,
                &Prefs.iomatrix_colormap,
                &Prefs.popup_throughput_threshold,
                sf, bf);
  if (n>= 8) Prefs.smallfontname = strdup(sf);
  if (n>= 9) Prefs.bigfontname = strdup(bf);
}

static GkrellmMonitor hdplop_mon =
{
  PLUGIN_NAME,         /* Name, for config tab.                    */
  0,                   /* Id,  0 if a plugin                       */
  create_plugin,       /* The create_plugin() function             */
  NULL,                /* The update_plugin() function             */
  create_plugin_tab,   /* The create_plugin_tab() config function  */
  NULL,                /* The apply_plugin_config() function       */

  save_plugin_config,  /* The save_plugin_config() function        */
  load_plugin_config,  /* The load_plugin_config() function        */
  PLUGIN_KEYWORD,      /* config keyword                           */

  NULL,                /* Undefined 2                              */
  NULL,                /* Undefined 1                              */
  NULL,                /* private                                  */

  MON_CPU,             /* Insert plugin before this monitor.       */
  NULL,                /* Handle if a plugin, filled in by GKrellM */
  NULL                 /* path if a plugin, filled in by GKrellM   */
};


GkrellmMonitor *gkrellm_init_plugin(void)
{
  gkrellm_add_meter_style(&hdplop_mon, PLUGIN_STYLE);
  return (mon = &hdplop_mon);
}



