/*

   YAWMPPP - PPP dock app/helper for WindowMaker
   Copyright (C) 2000,2001:

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

   $Id: applet.c,v 1.2 2001/04/21 18:29:33 bergo Exp $

 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <gtk/gtk.h>
#include "applet.h"
#include "isprc.h"
#include "about.h"
#include "msgbox.h"

#include "stepphone.xpm"
#include "pppdoc.xpm"
#include "padlock.xpm"

/* straight from the Gimp... */
#include "cmd_new.xpm"
#include "cmd_dup.xpm"
#include "cmd_del.xpm"
#include "cmd_up.xpm"
#include "cmd_down.xpm"

GtkWidget *applet,*nbook;
GtkWidget *isp_list,*rem_button,*dup_button,
          *up_button,*dn_button;
GtkWidget *right_pane[8];

GtkWidget *ppp_pw[31];

struct YAWMPPP_ISP_INFO rc_entries[MAX_ISPS];
struct PREF_ISP_INFO  pref_entries[MAX_ISPS];
struct PREF_PPP_OPT   pppo;
int isp_count;

int selected_entry=-1;

char chat_path[512];
char pppd_path[512];

int   man_flag[6]={0,0,0,0,0,0};
char *man_help[6];

GdkFont *fixedfont=NULL;

int
main(int argc, char **argv)
{
  int i;

  gtk_init(&argc,&argv);
  find_out_paths();

  create_preferences_panel();
  load_rc_entries();
  set_ppp_pane();

  if (gdk_screen_width()>800)
    fixedfont=gdk_font_load("-*-fixed-medium-r-normal--15-*-*-*-*-*-*");
  else
    fixedfont=gdk_font_load("-*-fixed-medium-r-normal--10-*-*-*-*-*-*");

  gtk_main();

  gdk_font_unref(fixedfont);

  for(i=0;i<6;i++)
    if (man_flag[i])
      g_free(man_help[i]);

  return 0;
}

void
create_preferences_panel(void)
{
  GtkWidget *mvb;
  GtkWidget *hs,*bhb,*cb[5],*lt,*up,*down,*nl[2];
  GtkWidget *mhb,*svb,*sw,*aw[30],*rp,*vs,*ghb,*abo,*pppp,*hel;

  GtkStyle  *style;
  GdkBitmap *mask;
  GdkPixmap *myicon,*tp;

  GtkTooltips *tips;

  int i;

  applet=gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_usize (applet, 550, 430);
  gtk_window_set_title (GTK_WINDOW (applet), "YAWMPPP Configuration");
  gtk_window_set_wmclass(GTK_WINDOW(applet),"yawmppp","pref");
  gtk_container_set_border_width(GTK_CONTAINER(applet),4);
  gtk_widget_show (applet);

  style=gtk_widget_get_style(applet);
  myicon = gdk_pixmap_create_from_xpm_d (applet->window, &mask,
				       &style->bg[GTK_STATE_NORMAL],
				       (gchar **) stepphone_xpm);
  gdk_window_set_icon (applet->window, NULL, myicon, mask);
  gdk_window_set_icon_name(applet->window,"yawmppp");

  mvb=gtk_vbox_new(FALSE,2);
  gtk_container_add(GTK_CONTAINER(applet),mvb);

  nbook=gtk_notebook_new();
  gtk_box_pack_start(GTK_BOX(mvb),nbook,TRUE,TRUE,0);

  mhb=gtk_hbox_new(FALSE,2);

  nl[0]=gtk_label_new("Dialing Entries");
  gtk_notebook_append_page(GTK_NOTEBOOK(nbook),mhb,nl[0]);

  nl[1]=gtk_label_new("PPP Options");
  pppp=make_ppp_pane();
  gtk_notebook_append_page(GTK_NOTEBOOK(nbook),pppp,nl[1]);

  /* LEFT PANE */

  svb=gtk_vbox_new(FALSE,2);
  gtk_box_pack_start(GTK_BOX(mhb),svb,TRUE,TRUE,2);

  aw[0]=hlabel_new("YAWMPPP Dialing Entries:");
  gtk_box_pack_start(GTK_BOX(svb),aw[0],FALSE,TRUE,2);

  sw=gtk_scrolled_window_new(NULL,NULL);
  gtk_box_pack_start(GTK_BOX(svb),sw,TRUE,TRUE,4);
  gtk_widget_show(sw);
  gtk_container_set_border_width(GTK_CONTAINER(sw),0);

  isp_list=gtk_clist_new(2);
  gtk_clist_set_shadow_type(GTK_CLIST(isp_list),GTK_SHADOW_IN);
  gtk_clist_set_selection_mode(GTK_CLIST(isp_list),GTK_SELECTION_SINGLE);
  gtk_clist_column_titles_passive(GTK_CLIST(isp_list));
  gtk_clist_column_titles_hide(GTK_CLIST(isp_list));
  gtk_clist_set_column_width(GTK_CLIST(isp_list),0,34);
  gtk_clist_set_column_width(GTK_CLIST(isp_list),1,110);
  gtk_clist_set_row_height(GTK_CLIST(isp_list),36);
  gtk_clist_set_column_auto_resize(GTK_CLIST(isp_list),1,TRUE);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
				 GTK_POLICY_AUTOMATIC,
				 GTK_POLICY_ALWAYS);
  gtk_container_add(GTK_CONTAINER(sw),isp_list);
  gtk_widget_show(isp_list);

  lt=gtk_hbox_new(TRUE,0);
  gtk_box_pack_start(GTK_BOX(svb),lt,FALSE,FALSE,2);

  /* NEW */
  tp=gdk_pixmap_create_from_xpm_d (applet->window, &mask,
				   &style->bg[GTK_STATE_NORMAL],
				   (gchar **) cmd_new_xpm);
  up=gtk_pixmap_new(tp,mask); gtk_widget_show(up); gdk_pixmap_unref(tp);
  aw[1]=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(aw[1]),up);

  /* DEL */
  tp=gdk_pixmap_create_from_xpm_d (applet->window, &mask,
				   &style->bg[GTK_STATE_NORMAL],
				   (gchar **) cmd_del_xpm);
  up=gtk_pixmap_new(tp,mask); gtk_widget_show(up); gdk_pixmap_unref(tp);
  rem_button=aw[2]=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(aw[2]),up);

  /* DUP */
  tp=gdk_pixmap_create_from_xpm_d (applet->window, &mask,
				   &style->bg[GTK_STATE_NORMAL],
				   (gchar **) cmd_dup_xpm);
  up=gtk_pixmap_new(tp,mask); gtk_widget_show(up); gdk_pixmap_unref(tp);
  dup_button=aw[19]=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(aw[19]),up);

  /* UP */
  tp=gdk_pixmap_create_from_xpm_d (applet->window, &mask,
				   &style->bg[GTK_STATE_NORMAL],
				   (gchar **) cmd_up_xpm);
  up=gtk_pixmap_new(tp,mask); gtk_widget_show(up); gdk_pixmap_unref(tp);
  up_button=aw[20]=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(aw[20]),up);

  /* DOWN */
  tp=gdk_pixmap_create_from_xpm_d (applet->window, &mask,
				   &style->bg[GTK_STATE_NORMAL],
				   (gchar **) cmd_down_xpm);
  down=gtk_pixmap_new(tp,mask); gtk_widget_show(down); gdk_pixmap_unref(tp);
  dn_button=aw[21]=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(aw[21]),down);

  gtk_box_pack_start(GTK_BOX(lt),aw[1],FALSE,TRUE,0);
  gtk_box_pack_start(GTK_BOX(lt),aw[19],FALSE,TRUE,0);
  gtk_box_pack_start(GTK_BOX(lt),aw[2],FALSE,TRUE,0);
  gtk_box_pack_start(GTK_BOX(lt),aw[21],FALSE,TRUE,0);
  gtk_box_pack_start(GTK_BOX(lt),aw[20],FALSE,TRUE,0);

  /* TIPS */
  tips=gtk_tooltips_new();
  gtk_tooltips_set_tip(tips,aw[1],"New entry",NULL);
  gtk_tooltips_set_tip(tips,aw[19],"Duplicate entry",NULL);
  gtk_tooltips_set_tip(tips,aw[2],"Delete entry",NULL);
  gtk_tooltips_set_tip(tips,aw[21],"Move down",NULL);
  gtk_tooltips_set_tip(tips,aw[20],"Move up",NULL);

  vs=gtk_vseparator_new();
  gtk_box_pack_start(GTK_BOX(mhb),vs,FALSE,TRUE,2);

  /* RIGHT PANE */

  aw[3]=hlabel_new("Selected Entry:");

  rp=gtk_vbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(mhb),rp,TRUE,TRUE,2);

  aw[4]=hlabel_new("Description");
  aw[6]=hlabel_new("Display Handle");
  aw[8]=hlabel_new("Phone number");
  aw[10]=hlabel_new("Modem device");
  aw[12]=hlabel_new("Username");
  aw[14]=hlabel_new("Password");

  right_pane[0]=aw[5]=gtk_entry_new_with_max_length(128);
  right_pane[1]=aw[7]=gtk_entry_new_with_max_length(16);
  right_pane[2]=aw[9]=gtk_entry_new_with_max_length(32);
  right_pane[3]=aw[11]=gtk_entry_new_with_max_length(64);
  right_pane[4]=aw[13]=gtk_entry_new_with_max_length(32);
  right_pane[5]=aw[15]=gtk_entry_new_with_max_length(32);

  gtk_entry_set_visibility(GTK_ENTRY(aw[15]),FALSE);

  right_pane[6]=aw[16]=gtk_check_button_new_with_label("Pulse dialing");

  aw[17]=gtk_hbox_new(FALSE,2);

  for(i=3;i<=17;i++)
    gtk_box_pack_start(GTK_BOX(rp),aw[i],FALSE,TRUE,(i<4)?2:1);

  right_pane[7]=aw[18]=gtk_button_new_with_label(" More settings... ");
  gtk_box_pack_end(GTK_BOX(aw[17]),aw[18],FALSE,FALSE,2);

  /* BOTTOM */

  hs=gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(mvb),hs,FALSE,FALSE,2);

  ghb=gtk_hbox_new(FALSE,2);
  gtk_box_pack_start(GTK_BOX(mvb),ghb,FALSE,FALSE,2);
  abo=gtk_button_new_with_label(" ... ");
  gtk_box_pack_start(GTK_BOX(ghb),abo,FALSE,FALSE,2);
  hel=gtk_button_new_with_label(" Help ");
  gtk_box_pack_start(GTK_BOX(ghb),hel,FALSE,FALSE,4);

  bhb=gtk_hbox_new(TRUE,4);
  gtk_box_pack_start(GTK_BOX(ghb),bhb,TRUE,TRUE,2);

  cb[0]=gtk_button_new_with_label(" Apply ");
  cb[1]=gtk_button_new_with_label(" Cancel ");
  cb[2]=gtk_button_new_with_label(" OK");

  cb[3]=gtk_label_new(" ");
  cb[4]=gtk_label_new(" ");

  gtk_box_pack_start(GTK_BOX(bhb),cb[3],FALSE,TRUE,4);
  gtk_box_pack_start(GTK_BOX(bhb),cb[4],FALSE,TRUE,4);
  gtk_box_pack_start(GTK_BOX(bhb),cb[0],FALSE,TRUE,4);
  gtk_box_pack_start(GTK_BOX(bhb),cb[1],FALSE,TRUE,4);
  gtk_box_pack_start(GTK_BOX(bhb),cb[2],FALSE,TRUE,4);

  gtk_tooltips_set_tip(tips,abo,"About...",NULL);

  for(i=0;i<5;i++)
    gtk_widget_show(cb[i]);
  for(i=0;i<22;i++)
    gtk_widget_show(aw[i]);
  for(i=0;i<2;i++)
    gtk_widget_show(nl[i]);
  gtk_widget_show(bhb);
  gtk_widget_show(abo);
  gtk_widget_show(hel);
  gtk_widget_show(ghb);
  gtk_widget_show(rp);
  gtk_widget_show(lt);
  gtk_widget_show(vs);
  gtk_widget_show(hs);
  gtk_widget_show(svb);
  gtk_widget_show(mhb);
  gtk_widget_show(nbook);
  gtk_widget_show(mvb);

  /* signal plumbing */
  gtk_signal_connect (GTK_OBJECT (applet), "delete_event",
		      GTK_SIGNAL_FUNC (applet_kill), NULL);
  gtk_signal_connect (GTK_OBJECT (applet), "destroy",
		      GTK_SIGNAL_FUNC (applet_destroy), NULL);

  gtk_signal_connect (GTK_OBJECT (isp_list), "select_row",
		      GTK_SIGNAL_FUNC (list_select), NULL);
  gtk_signal_connect (GTK_OBJECT (isp_list), "unselect_row",
		      GTK_SIGNAL_FUNC (list_unselect), NULL);

  gtk_signal_connect (GTK_OBJECT (dn_button), "clicked",
		      GTK_SIGNAL_FUNC (list_movedown), NULL);
  gtk_signal_connect (GTK_OBJECT (up_button), "clicked",
		      GTK_SIGNAL_FUNC (list_moveup), NULL);
  gtk_signal_connect (GTK_OBJECT (rem_button), "clicked",
		      GTK_SIGNAL_FUNC (list_remove), NULL);
  gtk_signal_connect (GTK_OBJECT (dup_button), "clicked",
		      GTK_SIGNAL_FUNC (list_duplicate), NULL);
  gtk_signal_connect (GTK_OBJECT (aw[1]), "clicked",
		      GTK_SIGNAL_FUNC (list_add), NULL);
  gtk_signal_connect (GTK_OBJECT (right_pane[7]), "clicked",
		      GTK_SIGNAL_FUNC (pop_advanced), NULL);

  gtk_signal_connect (GTK_OBJECT (right_pane[0]), "changed",
		      GTK_SIGNAL_FUNC (isp_rename), NULL);

  gtk_signal_connect (GTK_OBJECT (cb[0]), "clicked",
		      GTK_SIGNAL_FUNC (applet_apply), NULL);
  gtk_signal_connect (GTK_OBJECT (cb[1]), "clicked",
		      GTK_SIGNAL_FUNC (applet_destroy), NULL);
  gtk_signal_connect (GTK_OBJECT (cb[2]), "clicked",
		      GTK_SIGNAL_FUNC (applet_save_and_quit), NULL);

  gtk_signal_connect (GTK_OBJECT (abo), "clicked",
		      GTK_SIGNAL_FUNC (applet_about), (gpointer)GTK_WINDOW(applet));
  gtk_signal_connect (GTK_OBJECT (hel), "clicked",
		      GTK_SIGNAL_FUNC (pop_help), NULL);
  gtk_notebook_set_page(GTK_NOTEBOOK(nbook),0);
}

gint
applet_kill (GtkWidget * widget, GdkEvent * event, gpointer data)
{
  return FALSE;
}

void
applet_destroy (GtkWidget * widget, gpointer data)
{
  gtk_main_quit();
}

void
list_select(GtkCList *cl,
	    gint row,
	    gint column,
	    GdkEventButton *geb,
	    gpointer data)
{
  selected_entry=row;
  update_right_pane();
}

void
list_unselect(GtkCList *cl,
	      gint row,
	      gint column,
	      GdkEventButton *geb,
	      gpointer data)
{
  commit_back_to_db();
  selected_entry=-1;
  clear_and_disable_right_pane();
}

GtkWidget *
hlabel_new(char *s)
{
  GtkWidget *h,*l;
  h=gtk_hbox_new(FALSE,0);
  l=gtk_label_new(s);
  gtk_label_set_justify(GTK_LABEL(l),GTK_JUSTIFY_LEFT);
  gtk_box_pack_start(GTK_BOX(h),l,FALSE,FALSE,0);
  gtk_container_set_border_width(GTK_CONTAINER(h),1);
  gtk_widget_show(l);
  return(h);
}

void
clear_and_disable_right_pane(void)
{
  int i;

  for(i=0;i<6;i++) {
    gtk_editable_select_region(GTK_EDITABLE(right_pane[i]),0,-1);
    gtk_editable_delete_selection(GTK_EDITABLE(right_pane[i]));
    gtk_widget_set_sensitive(right_pane[i],FALSE);
  }

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(right_pane[6]),FALSE);
  gtk_widget_set_sensitive(right_pane[6],FALSE);
  gtk_widget_set_sensitive(right_pane[7],FALSE);
  gtk_widget_set_sensitive(rem_button,FALSE);
  gtk_widget_set_sensitive(up_button,FALSE);
  gtk_widget_set_sensitive(dn_button,FALSE);
  gtk_widget_set_sensitive(dup_button,FALSE);
}

void
update_right_pane(void)
{
  int i;

  if (selected_entry<0) {
    clear_and_disable_right_pane();
    return;
  }

  for(i=0;i<8;i++)
    gtk_widget_set_sensitive(right_pane[i],TRUE);

  gtk_entry_set_text(GTK_ENTRY(right_pane[0]),
		     pref_entries[selected_entry].LongName);
  gtk_entry_set_text(GTK_ENTRY(right_pane[1]),
		     pref_entries[selected_entry].ShortName);
  gtk_entry_set_text(GTK_ENTRY(right_pane[2]),
		     pref_entries[selected_entry].Phone);
  gtk_entry_set_text(GTK_ENTRY(right_pane[3]),
		     pref_entries[selected_entry].Device);
  gtk_entry_set_text(GTK_ENTRY(right_pane[4]),
		     pref_entries[selected_entry].Username);
  gtk_entry_set_text(GTK_ENTRY(right_pane[5]),
		     pref_entries[selected_entry].Password);

  if (atoi(pref_entries[selected_entry].PulseDial))
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(right_pane[6]),TRUE);
  else
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(right_pane[6]),FALSE);
  gtk_widget_set_sensitive(rem_button,TRUE);
  gtk_widget_set_sensitive(up_button,(selected_entry!=0));
  gtk_widget_set_sensitive(dn_button,(selected_entry<(isp_count-1)));
  gtk_widget_set_sensitive(dup_button,TRUE);
}

void
commit_back_to_db(void)
{
  if (selected_entry<0)
    return;

  strcpy(pref_entries[selected_entry].LongName,
	 gtk_entry_get_text(GTK_ENTRY(right_pane[0])));
  strcpy(pref_entries[selected_entry].ShortName,
	 gtk_entry_get_text(GTK_ENTRY(right_pane[1])));
  strcpy(pref_entries[selected_entry].Phone,
	 gtk_entry_get_text(GTK_ENTRY(right_pane[2])));
  strcpy(pref_entries[selected_entry].Device,
	 gtk_entry_get_text(GTK_ENTRY(right_pane[3])));
  strcpy(pref_entries[selected_entry].Username,
	 gtk_entry_get_text(GTK_ENTRY(right_pane[4])));
  strcpy(pref_entries[selected_entry].Password,
	 gtk_entry_get_text(GTK_ENTRY(right_pane[5])));

  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(right_pane[6])))
    strcpy(pref_entries[selected_entry].PulseDial,"1");
  else
    strcpy(pref_entries[selected_entry].PulseDial,"0");
}

void
load_rc_entries(void)
{
  char p[512],*q;
  int i;

  q=getenv("HOME");
  sprintf(p,"%s/.yawmppp2/yawmppprc",q);
  isp_count=GetISPInfo(p,&rc_entries[0],MAX_ISPS);

  read_ppp_options_from_rc(p);

  for(i=0;i<isp_count;i++)
    xlate_ppp_to_pref(&rc_entries[i],&pref_entries[i]);

  clear_and_disable_right_pane();

  fill_list();
}

void
xlate_ppp_to_pref(struct YAWMPPP_ISP_INFO *wii,struct PREF_ISP_INFO *pii)
{
  FILE *f;
  int i;
  char tmp[512],*p;

  /* defaults if we need to jump out of the train */
  memset(pii,0,sizeof(struct PREF_ISP_INFO));
  strcpy(pii->Device,"/dev/modem");
  if ((p=getenv("USER"))!=NULL)
    strcpy(pii->Username,p);
  strcpy(pii->ModemInit1,"ATZ");
  strcpy(pii->ModemInit2,"ATM1L2");
  strcpy(pii->PulseDial,"0");
  strcpy(pii->UserString,"ogin:");
  strcpy(pii->PassString,"word:");
  strcpy(pii->ModemSpeed,"115200");
  pii->nExpectPairs=0;

  memcpy(&(pii->ppp),&(wii->ppp),sizeof(struct ISP_PPP));

  /* entry name */
  strcpy(pii->LongName,wii->LongName);
  strcpy(pii->ShortName,wii->ShortName);
  pii->nologin=wii->nologin;

  /* get device and chat script filename from "Start" file */
  f=fopen(wii->PPPLine,"r");
  if (!f) return;

  fgets(pii->Device,63,f); strtok(pii->Device,"\n");
  fgets(pii->ModemSpeed,15,f); strtok(pii->ModemSpeed,"\n");
  fgets(pii->PulseDial,7,f); strtok(pii->PulseDial,"\n");
  fgets(pii->Phone,31,f); strtok(pii->Phone,"\n");
  fgets(pii->ModemInit1,255,f); strtok(pii->ModemInit1,"\n");
  fgets(pii->ModemInit2,255,f); strtok(pii->ModemInit2,"\n");

  fgets(pii->UserString,31,f); strtok(pii->UserString,"\n");
  fgets(pii->Username,31,f); strtok(pii->Username,"\n");

  fgets(pii->PassString,31,f); strtok(pii->PassString,"\n");
  fgets(pii->Password,31,f); strtok(pii->Password,"\n");

  fgets(tmp,511,f); strtok(tmp,"\n"); pii->nExpectPairs=atoi(tmp);
  for(i=0;i<pii->nExpectPairs;i++) {
    fgets(pii->s_expect[i],31,f); strtok(pii->s_expect[i],"\n");
    fgets(pii->s_send[i],31,f); strtok(pii->s_send[i],"\n");
  }
  fclose(f);
}

void
list_moveup(GtkWidget *gw,gpointer data)
{
  int j;
  struct PREF_ISP_INFO tmp;
  j=selected_entry;
  commit_back_to_db();
  selected_entry=-1;
  clear_and_disable_right_pane();

  memcpy(&tmp,&pref_entries[j],sizeof(struct PREF_ISP_INFO));
  memcpy(&pref_entries[j],&pref_entries[j-1],sizeof(struct PREF_ISP_INFO));
  memcpy(&pref_entries[j-1],&tmp,sizeof(struct PREF_ISP_INFO));

  fill_list();
  gtk_clist_select_row(GTK_CLIST(isp_list),j-1,0);
}

void
list_movedown(GtkWidget *gw,gpointer data)
{
  int j;
  struct PREF_ISP_INFO tmp;
  j=selected_entry;
  commit_back_to_db();
  selected_entry=-1;
  clear_and_disable_right_pane();

  memcpy(&tmp,&pref_entries[j],sizeof(struct PREF_ISP_INFO));
  memcpy(&pref_entries[j],&pref_entries[j+1],sizeof(struct PREF_ISP_INFO));
  memcpy(&pref_entries[j+1],&tmp,sizeof(struct PREF_ISP_INFO));

  fill_list();
  gtk_clist_select_row(GTK_CLIST(isp_list),j+1,0);
}

void
fill_list(void)
{
  int i;
  char *q,*pp[2];
  GtkStyle  *style;
  GdkBitmap *mask;
  GdkPixmap *tp;

  style=gtk_widget_get_style(applet);
  tp = gdk_pixmap_create_from_xpm_d (applet->window, &mask,
				     &style->bg[GTK_STATE_NORMAL],
				     (gchar **) pppdoc_xpm);

  gtk_clist_freeze(GTK_CLIST(isp_list));
  gtk_clist_clear(GTK_CLIST(isp_list));

  for(i=0;i<isp_count;i++) {
    q=pref_entries[i].LongName;
    pp[0]=pp[1]=q;
    gtk_clist_append(GTK_CLIST(isp_list),&q);
    gtk_clist_set_pixmap(GTK_CLIST(isp_list),i,0,tp,mask);
    gtk_clist_set_text(GTK_CLIST(isp_list),i,1,q);
    gtk_clist_set_shift(GTK_CLIST(isp_list),i,0,-1,0);
    gtk_clist_set_shift(GTK_CLIST(isp_list),i,1,11,0);
  }

  gtk_clist_thaw(GTK_CLIST(isp_list));
  gdk_pixmap_unref(tp);
}

void
list_remove(GtkWidget *gw,gpointer data)
{
  int i,j;

  j=selected_entry;
  selected_entry=-1;
  clear_and_disable_right_pane();

  for(i=j;i<(isp_count-1);i++)
    memcpy(&pref_entries[i],&pref_entries[i+1],
	   sizeof(struct PREF_ISP_INFO));
  isp_count--;
  fill_list();
}

void
list_duplicate(GtkWidget *gw,gpointer data)
{
  char tmp[128];
  int i;

  if (isp_count>=MAX_ISPS) {
    message_box(GTK_WINDOW(applet),
		ERR_MAX_ISPS,"Error",
		MSGBOX_OK,MSGBOX_ICON_EXCLAMATION);
    return;
  }

  commit_back_to_db();
  i=selected_entry;
  memcpy(&pref_entries[isp_count],&pref_entries[i],
	 sizeof(struct PREF_ISP_INFO));
  selected_entry=-1;
  clear_and_disable_right_pane();
  sprintf(tmp,"copy of %s",pref_entries[isp_count].LongName);
  strcpy(pref_entries[isp_count].LongName,tmp);
  isp_count++;
  fill_list();
  gtk_clist_select_row(GTK_CLIST(isp_list),i,0);
}

void
list_add(GtkWidget *gw,gpointer data)
{
  int i;
  struct PREF_ISP_INFO *ne;

  if (isp_count>=MAX_ISPS) {
    message_box(GTK_WINDOW(applet),
		ERR_MAX_ISPS,"Error",
		MSGBOX_OK,MSGBOX_ICON_EXCLAMATION);
    return;
  }

  i=selected_entry;
  commit_back_to_db();
  selected_entry=-1;
  clear_and_disable_right_pane();

  ne=&pref_entries[isp_count];
  memset(ne,0,sizeof(struct PREF_ISP_INFO));

  strcpy(ne->LongName,"New Entry");
  strcpy(ne->ShortName,"ISP");
  strcpy(ne->Device,"/dev/modem");
  strcpy(ne->ModemInit1,"ATZ");
  strcpy(ne->ModemInit2,"ATM1L2");
  strcpy(ne->UserString,"ogin:");
  strcpy(ne->PassString,"word:");
  strcpy(ne->ModemSpeed,"57600");
  ne->nExpectPairs=0;

  ne->ppp.override=0;
  ne->ppp.noipdefault=1;
  ne->ppp.noauth=1;
  ne->ppp.passive=0;
  ne->ppp.defaultroute=1;
  ne->ppp.chap=AUTH_DONTCARE;
  ne->ppp.pap=AUTH_DONTCARE;

  isp_count++;
  fill_list();
  if (i>=0)
    gtk_clist_select_row(GTK_CLIST(isp_list),i,0);
}

GtkWidget *adv_e[7],*advdlg,*adv_w[20],*adv_nl;
struct PREF_ISP_INFO adv_regret;

void
pop_advanced(GtkWidget *gw,gpointer data)
{
  GtkWidget *dlg,*l[7],*hs,*bhb,*bt[4],*esp,*esb;
  GtkWidget *av[30];
  GSList *pap,*chap;
  char tmp[128];
  int i;
  struct PREF_ISP_INFO *pii;

  memcpy(&adv_regret,&pref_entries[selected_entry],sizeof(struct PREF_ISP_INFO));

  advdlg=dlg=gtk_window_new(GTK_WINDOW_DIALOG);
  gtk_window_set_transient_for(GTK_WINDOW(advdlg),GTK_WINDOW(applet));
  sprintf(tmp,"Settings: %s",pref_entries[selected_entry].LongName);
  gtk_window_set_title(GTK_WINDOW(dlg),tmp);
  gtk_window_set_policy(GTK_WINDOW(dlg),TRUE,TRUE,TRUE);
  gtk_container_set_border_width(GTK_CONTAINER(dlg),4);

  /* main vbox */
  av[0]=gtk_vbox_new(FALSE,2);
  gtk_container_add(GTK_CONTAINER(dlg),av[0]);

  av[1]=gtk_hbox_new(FALSE,1);

  /* first vbox */
  av[2]=gtk_vbox_new(FALSE,1);
  /* second vbox */
  av[3]=gtk_vbox_new(FALSE,1);
  av[4]=gtk_vseparator_new();

  gtk_box_pack_start(GTK_BOX(av[0]),av[1],FALSE,TRUE,1);
  gtk_box_pack_start(GTK_BOX(av[1]),av[2],FALSE,TRUE,1);
  gtk_box_pack_start(GTK_BOX(av[1]),av[4],FALSE,TRUE,1);
  gtk_box_pack_start(GTK_BOX(av[1]),av[3],FALSE,TRUE,1);

  l[0]=hlabel_new("Modem Init String #1");
  l[1]=hlabel_new("Modem Init String #2 (sent after #1)");
  l[2]=hlabel_new("Port/Modem speed (bps)");
  l[3]=hlabel_new("Send username when this string is received:");
  l[4]=hlabel_new("Send password when this string is received:");

  adv_e[0]=gtk_entry_new_with_max_length(256);
  adv_e[1]=gtk_entry_new_with_max_length(256);
  adv_e[2]=gtk_entry_new_with_max_length(16);
  adv_e[3]=gtk_entry_new_with_max_length(32);
  adv_e[4]=gtk_entry_new_with_max_length(32);

  esb=gtk_hbox_new(FALSE,2);
  esp=gtk_button_new_with_label(" Additional conversation... ");

  adv_nl=gtk_check_button_new_with_label("Don't generate login/password pairs");

  for(i=0;i<5;i++) {
    gtk_box_pack_start(GTK_BOX(av[2]),l[i],FALSE,FALSE,1);
    gtk_box_pack_start(GTK_BOX(av[2]),adv_e[i],FALSE,FALSE,1);

    if (i==4) {
      gtk_box_pack_start(GTK_BOX(av[2]),esb,FALSE,TRUE,1);
      gtk_box_pack_end(GTK_BOX(esb),esp,FALSE,TRUE,1);
      gtk_widget_show(esb);
      gtk_widget_show(esp);
      gtk_box_pack_start(GTK_BOX(av[2]),adv_nl,FALSE,TRUE,1);
      gtk_widget_show(adv_nl);
    }

    gtk_widget_show(l[i]);
    gtk_widget_show(adv_e[i]);
  }

  /* second pane */

  av[5]=gtk_label_new(" ISP PPP Options ");

  av[6]=gtk_hseparator_new();
  adv_w[0]=av[7]=gtk_check_button_new_with_label("Override global PPP options");
  av[8]=gtk_hseparator_new();

  adv_w[1]=av[9]=gtk_check_button_new_with_label("defaultroute");
  adv_w[2]=av[10]=gtk_check_button_new_with_label("passive");
  adv_w[3]=av[11]=gtk_check_button_new_with_label("noauth");
  adv_w[4]=av[12]=gtk_check_button_new_with_label("noipdefault");
  av[13]=gtk_frame_new(" CHAP Authentication ");
  av[14]=gtk_frame_new(" PAP Authentication ");

  av[15]=gtk_vbox_new(FALSE,1);
  av[16]=gtk_vbox_new(FALSE,1);

  gtk_frame_set_shadow_type(GTK_FRAME(av[13]),GTK_SHADOW_ETCHED_IN);
  gtk_frame_set_shadow_type(GTK_FRAME(av[14]),GTK_SHADOW_ETCHED_IN);

  gtk_container_add(GTK_CONTAINER(av[13]),av[15]);
  gtk_container_add(GTK_CONTAINER(av[14]),av[16]);

  for(i=5;i<=14;i++)
    gtk_box_pack_start(GTK_BOX(av[3]),av[i],FALSE,FALSE,1);

  adv_w[5]=av[20]=gtk_radio_button_new_with_label(NULL,"require CHAP  ");
  chap=gtk_radio_button_group(GTK_RADIO_BUTTON(av[20]));
  adv_w[6]=av[21]=gtk_radio_button_new_with_label(chap,"refuse CHAP  ");
  chap=gtk_radio_button_group(GTK_RADIO_BUTTON(av[21]));
  adv_w[7]=av[22]=gtk_radio_button_new_with_label(chap,"don't care  ");

  gtk_box_pack_start(GTK_BOX(av[15]),av[20],FALSE,TRUE,1);
  gtk_box_pack_start(GTK_BOX(av[15]),av[21],FALSE,TRUE,1);
  gtk_box_pack_start(GTK_BOX(av[15]),av[22],FALSE,TRUE,1);

  adv_w[8]=av[23]=gtk_radio_button_new_with_label(NULL,"require PAP  ");
  pap=gtk_radio_button_group(GTK_RADIO_BUTTON(av[23]));
  adv_w[9]=av[24]=gtk_radio_button_new_with_label(pap,"refuse PAP  ");
  pap=gtk_radio_button_group(GTK_RADIO_BUTTON(av[24]));
  adv_w[10]=av[25]=gtk_radio_button_new_with_label(pap,"don't care  ");

  gtk_box_pack_start(GTK_BOX(av[16]),av[23],FALSE,TRUE,1);
  gtk_box_pack_start(GTK_BOX(av[16]),av[24],FALSE,TRUE,1);
  gtk_box_pack_start(GTK_BOX(av[16]),av[25],FALSE,TRUE,1);

  for(i=20;i<=25;i++)
    gtk_widget_show(av[i]);

  /* second pane */

  hs=gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(av[0]),hs,FALSE,FALSE,4);

  bhb=gtk_hbox_new(TRUE,4);

  gtk_box_pack_start(GTK_BOX(av[0]),bhb,FALSE,FALSE,4);

  bt[0]=gtk_button_new_with_label(" Help ");
  bt[1]=gtk_label_new(" ");
  bt[2]=gtk_button_new_with_label(" OK ");
  bt[3]=gtk_button_new_with_label(" Cancel ");

  for(i=0;i<4;i++) {
    gtk_box_pack_start(GTK_BOX(bhb),bt[i],FALSE,TRUE,4);
    gtk_widget_show(bt[i]);
  }

  gtk_widget_show(bhb);
  gtk_widget_show(hs);
  for(i=0;i<17;i++)
    gtk_widget_show(av[i]);

  gtk_widget_show(dlg);
  gtk_widget_grab_focus(adv_e[0]);

  pii=&pref_entries[selected_entry];
  gtk_entry_set_text(GTK_ENTRY(adv_e[0]),pii->ModemInit1);
  gtk_entry_set_text(GTK_ENTRY(adv_e[1]),pii->ModemInit2);
  gtk_entry_set_text(GTK_ENTRY(adv_e[2]),pii->ModemSpeed);
  gtk_entry_set_text(GTK_ENTRY(adv_e[3]),pii->UserString);
  gtk_entry_set_text(GTK_ENTRY(adv_e[4]),pii->PassString);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(adv_w[0]),!!(pii->ppp.override));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(adv_w[1]),!!(pii->ppp.defaultroute));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(adv_w[2]),!!(pii->ppp.passive));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(adv_w[3]),!!(pii->ppp.noauth));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(adv_w[4]),!!(pii->ppp.noipdefault));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(adv_w[5+pii->ppp.chap]),TRUE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(adv_w[8+pii->ppp.pap]),TRUE);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(adv_nl),!!(pii->nologin));

  if (!(pii->ppp.override))
    enable_local_ppp(FALSE);

  gtk_signal_connect (GTK_OBJECT (bt[0]), "clicked",
		      GTK_SIGNAL_FUNC (pop_help), NULL);
  gtk_signal_connect (GTK_OBJECT (bt[2]), "clicked",
		      GTK_SIGNAL_FUNC (adv_ok), NULL);
  gtk_signal_connect (GTK_OBJECT (bt[3]), "clicked",
		      GTK_SIGNAL_FUNC (adv_cancel), NULL);
  gtk_signal_connect (GTK_OBJECT (dlg), "destroy",
		      GTK_SIGNAL_FUNC (adv_destroy), NULL);

  gtk_signal_connect (GTK_OBJECT (esp), "clicked",
		      GTK_SIGNAL_FUNC (pop_expect), NULL);

  gtk_signal_connect (GTK_OBJECT (adv_w[0]), "toggled",
		      GTK_SIGNAL_FUNC (ppp_override_toggle), NULL);

  gtk_grab_add(dlg);
}

void
enable_local_ppp(gboolean e)
{
  int i;
  for(i=1;i<11;i++)
    gtk_widget_set_sensitive(adv_w[i],e);
}

void
ppp_override_toggle(GtkToggleButton *gtb,gpointer data)
{
  enable_local_ppp(gtb->active);
}

void
adv_ok (GtkWidget * widget, gpointer data)
{
  struct PREF_ISP_INFO *pii;

  pii=&pref_entries[selected_entry];
  strcpy(pii->ModemInit1,gtk_entry_get_text(GTK_ENTRY(adv_e[0])));
  strcpy(pii->ModemInit2,gtk_entry_get_text(GTK_ENTRY(adv_e[1])));
  strcpy(pii->ModemSpeed,gtk_entry_get_text(GTK_ENTRY(adv_e[2])));
  strcpy(pii->UserString,gtk_entry_get_text(GTK_ENTRY(adv_e[3])));
  strcpy(pii->PassString,gtk_entry_get_text(GTK_ENTRY(adv_e[4])));

  pii->nologin=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(adv_nl));

  pii->ppp.override=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(adv_w[0]));
  pii->ppp.defaultroute=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(adv_w[1]));
  pii->ppp.passive=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(adv_w[2]));
  pii->ppp.noauth=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(adv_w[3]));
  pii->ppp.noipdefault=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(adv_w[4]));

  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(adv_w[5])))
    pii->ppp.chap=AUTH_REQUIRE;
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(adv_w[6])))
    pii->ppp.chap=AUTH_REFUSE;
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(adv_w[7])))
    pii->ppp.chap=AUTH_DONTCARE;

  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(adv_w[8])))
    pii->ppp.pap=AUTH_REQUIRE;
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(adv_w[9])))
    pii->ppp.pap=AUTH_REFUSE;
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(adv_w[10])))
    pii->ppp.pap=AUTH_DONTCARE;

  gtk_widget_destroy(advdlg);
}

void
adv_cancel (GtkWidget * widget, gpointer data)
{
  memcpy(&pref_entries[selected_entry],&adv_regret,sizeof(struct PREF_ISP_INFO));
  gtk_widget_destroy(advdlg);
}

void
adv_destroy (GtkWidget * widget, gpointer data)
{
  gtk_grab_remove(widget);
  advdlg=NULL;
}

void
isp_rename(GtkEditable *ge,gpointer data)
{
  if (selected_entry<0) return;
  strcpy(pref_entries[selected_entry].LongName,
	 gtk_entry_get_text(GTK_ENTRY(ge)));
  gtk_clist_set_text(GTK_CLIST(isp_list),selected_entry,1,
		     pref_entries[selected_entry].LongName);
}

int
write_and_apply_data(void)
{
  char tmp[256],aux[768],yet[512],v_home[512],v_user[64];
  struct stat ss;
  FILE *f;
  int i,j,needroot;
  struct PREF_PPP_OPT modippp;

  get_ppp_pane();
  if (selected_entry>=0)
    commit_back_to_db();

  sprintf(tmp,"%s/.yawmppp2",getenv("HOME"));

  if (stat(tmp,&ss)<0) {
    if (mkdir(tmp,0700)<0) {
      message_box(GTK_WINDOW(applet),ERR_CANT_MKDIR,"Error",
		  MSGBOX_OK,
		  MSGBOX_ICON_ERROR);
      return(-1);
    }
  } else
    chmod(tmp,0700);

  /* the yawmppprc file */
  sprintf(aux,"%s/yawmppprc",tmp);
  f=fopen(aux,"w");
  if (!f) {
    message_box(GTK_WINDOW(applet),ERR_CANT_WRITE,"Error",
		MSGBOX_OK,
		MSGBOX_ICON_ERROR);
  } else {

    /* FILE 1: YAWMPPPRC mode 600 */

    fprintf(f,"# yawmppprc : configuration for yawmppp generated by\n");
    fprintf(f,"# yawmppp.pref version %s. You should avoid editing this manually.\n",
	    VERSION);
    fprintf(f,"# Use the yawmppp.pref program instead.\n");
    fprintf(f,"#\n# BTW, the format changed from YAWMPPP 1.x.x to 2.x.x,\n");
    fprintf(f,"# copying old yawmppprc files to this directory is a bad idea\n\n");

    for(i=0;i<isp_count;i++) {
      fprintf(f,"ISP.BEGIN\n");
      fprintf(f,"\t%s %s\n",KEY_LONGNAME,pref_entries[i].LongName);
      fprintf(f,"\t%s %s\n",KEY_SHORTNAME,pref_entries[i].ShortName);

      sprintf(yet,"%s/start.%d",tmp,i);
      fprintf(f,"\t%s %s\n",KEY_STARTACTION,yet);

      sprintf(yet,"%s/stop.%d",tmp,i);
      fprintf(f,"\t%s %s\n",KEY_STOPACTION,yet);

      sprintf(yet,"%s/ifdown.%d",tmp,i);
      fprintf(f,"\t%s %s\n",KEY_IFDOWNACTION,yet);

      sprintf(yet,"%s/opts.%d",tmp,i);
      fprintf(f,"\t%s %s\n",KEY_PPPSTUFF,yet);

      sprintf(yet,"%s/chat.%d",tmp,i);
      fprintf(f,"\t%s %s\n",KEY_CHATSTUFF,yet);

      fprintf(f,"\t%s %s\n",KEY_IFDOWNACTION,yet);
      fprintf(f,"\t%s %s\n",KEY_SPEEDACTION,"/etc/ppp/yagetmodemspeed");
      fprintf(f,"\t%s %s\n",KEY_USER,pref_entries[i].Username);
      fprintf(f,"\t%s %s\n",KEY_PHONE,pref_entries[i].Phone);

      fprintf(f,"\t%s %d\n",KEY_PPP_OVER,pref_entries[i].ppp.override);
      fprintf(f,"\t%s %d\n",KEY_PPP_DEFAULTROUTE,pref_entries[i].ppp.defaultroute);
      fprintf(f,"\t%s %d\n",KEY_PPP_PASSIVE,pref_entries[i].ppp.passive);
      fprintf(f,"\t%s %d\n",KEY_PPP_NOAUTH,pref_entries[i].ppp.noauth);
      fprintf(f,"\t%s %d\n",KEY_PPP_NOIPDEFAULT,pref_entries[i].ppp.noipdefault);
      fprintf(f,"\t%s %d\n",KEY_PPP_CHAP,pref_entries[i].ppp.chap);
      fprintf(f,"\t%s %d\n",KEY_PPP_PAP,pref_entries[i].ppp.pap);
      fprintf(f,"\t%s %d\n",KEY_NOLOGIN,pref_entries[i].nologin);

      fprintf(f,"ISP.END\n\n");
    }

    /* write ppp pane */
    fprintf(f,"Note: if you were meant to edit the line below I would have\n");
    fprintf(f,"made it readable by humans.\n");
    fprintf(f,"PPPOPTIONS %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
	    pppo.defaultroute,
	    pppo.lock,
	    pppo.passive,
	    pppo.debug,
	    pppo.kdebug,
	    pppo.noauth,
	    pppo.noipdefault,
	    pppo.linectl,
	    pppo.flowctl,
	    pppo.mtu,
	    pppo.mru,
	    pppo.lcp[0],
	    pppo.lcp[1],
	    pppo.lcp[2],
	    pppo.lcp[3],
	    pppo.lcp[4],
	    pppo.usepeerdns);

    fclose(f);
    chmod(aux,0600);
  }

  /* the start.x, stop.x, ifdown.x , chat.x and opts.x files */
  for(i=0;i<isp_count;i++) {

    /* FILE 2 : start.x mode 700 */

    sprintf(aux,"%s/start.%d",tmp,i);
    f=fopen(aux,"w");
    if (!f) {
      message_box(GTK_WINDOW(applet),ERR_CANT_WRITE,"Error",
		  MSGBOX_OK,
		  MSGBOX_ICON_ERROR);
      return(-1);
    }
    fprintf(f,"#!/bin/sh\n");
    fprintf(f,"# YAWMPPP dialing script generated by yawmppp.pref\n");
    fprintf(f,"# version %s. If possible, do not edit manually.\n\n",VERSION);

    fprintf(f,"%s call %s_yawmppp_isp%d\n",pppd_path,getenv("USER"),i);
    fclose(f);
    chmod(aux,0700);

    /* FILE 3 : opts.x mode 600 */

    sprintf(aux,"%s/opts.%d",tmp,i);
    f=fopen(aux,"w");
    if (!f) {
      message_box(GTK_WINDOW(applet),ERR_CANT_WRITE,"Error",
		  MSGBOX_OK,
		  MSGBOX_ICON_ERROR);
      return(-1);
    }

    fprintf(f,"%s\n",pref_entries[i].Device);
    fprintf(f,"%s\n",pref_entries[i].ModemSpeed);
    fprintf(f,"%s\n",pref_entries[i].PulseDial);
    fprintf(f,"%s\n",pref_entries[i].Phone);
    fprintf(f,"%s\n",pref_entries[i].ModemInit1);
    fprintf(f,"%s\n",pref_entries[i].ModemInit2);
    fprintf(f,"%s\n",pref_entries[i].UserString);
    fprintf(f,"%s\n",pref_entries[i].Username);
    fprintf(f,"%s\n",pref_entries[i].PassString);
    fprintf(f,"%s\n",pref_entries[i].Password);
    fprintf(f,"%d\n",pref_entries[i].nExpectPairs);
    for(j=0;j<pref_entries[i].nExpectPairs;j++) {
      fprintf(f,"%s\n",pref_entries[i].s_expect[j]);
      fprintf(f,"%s\n",pref_entries[i].s_send[j]);
    }

    fclose(f);
    chmod(aux,0600);

    /* FILE 4 : stop.x mode 700 */

    sprintf(aux,"%s/stop.%d",tmp,i);
    f=fopen(aux,"w");
    if (!f) {
      message_box(GTK_WINDOW(applet),ERR_CANT_WRITE,"Error",
		  MSGBOX_OK,
		  MSGBOX_ICON_ERROR);
      return(-1);
    }

    fprintf(f,"#!/bin/sh\n");
    fprintf(f,"# YAWMPPP dialing script generated by yawmppp.pref\n");
    fprintf(f,"# version %s. If possible, do not edit manually.\n\n",VERSION);

    fprintf(f,"DEVICE=ppp0\n\n");
    fprintf(f,"if [ -r /var/run/$DEVICE.pid ]; then\n");
    fprintf(f,"\tkill -INT `cat /var/run/$DEVICE.pid`\n\n");
    fprintf(f,"\tif [ ! \"$?\" = \"0\" ]; then\n");
    fprintf(f,"\t\trm -f /var/run/$DEVICE.pid\n");
    fprintf(f,"\t\techo >/dev/console \"ERROR: Removed stale pid file\"\n");
    fprintf(f,"\t\texit 1\n");
    fprintf(f,"\tfi\n\n");
    fprintf(f,"\techo >/dev/console \"PPP link to $DEVICE terminated.\"\n");
    fprintf(f,"\texit 0\n");
    fprintf(f,"fi\n\n");
    fprintf(f,"echo >/dev/console \"ERROR: PPP link not active on $DEVICE\"\n");
    fprintf(f,"exit 1\n");
    fclose(f);
    chmod(aux,0700);

    /* FILE 5 : ifdown.x mode 700 */

    sprintf(aux,"%s/ifdown.%d",tmp,i);
    f=fopen(aux,"w");
    if (!f) {
      message_box(GTK_WINDOW(applet),ERR_CANT_WRITE,"Error",
		  MSGBOX_OK,
		  MSGBOX_ICON_ERROR);
      return(-1);
    }

    fprintf(f,"#!/bin/sh\n");
    fprintf(f,"# YAWMPPP dialing script generated by yawmppp.pref\n");
    fprintf(f,"# version %s. If possible, do not edit manually.\n\n",VERSION);

    fprintf(f,"# uncomment the line below to have yawmppp restart\n");
    fprintf(f,"# connections automatically\n\n");
    fprintf(f,"# source %s/start.%d\n",tmp,i);
    fclose(f);
    chmod(aux,0700);

    /* FILE 6 : user_chat.x mode 700 */

    sprintf(aux,"%s/%s_chat.%d",tmp,getenv("USER"),i);
    f=fopen(aux,"w");
    if (!f) {
      message_box(GTK_WINDOW(applet),ERR_CANT_WRITE,"Error",
		  MSGBOX_OK,
		  MSGBOX_ICON_ERROR);
      return(-1);
    }
    fprintf(f,"ABORT \"NO CARRIER\"\n");
    fprintf(f,"ABORT \"NO DIALTONE\"\n");
    fprintf(f,"ABORT \"BUSY\"\n");
    fprintf(f,"ABORT \"ERROR\"\n");
    fprintf(f,"ABORT \"NO ANSWER\"\n");
    fprintf(f,"ABORT \"Username/Password Incorrect\"\n");
    fprintf(f,"ABORT \"Authentication Failure\"\n");
    fprintf(f,"ABORT \"Bad password\"\n");
    fprintf(f,"ABORT \"Bad Password\"\n");
    fprintf(f,"REPORT CARRIER\n");
    fprintf(f,"REPORT CONNECT\n");
    fprintf(f,"\"\" \"at\"\n");
    fprintf(f,"OK \"%s\"\n",pref_entries[i].ModemInit1);
    fprintf(f,"OK \"%s\"\n",pref_entries[i].ModemInit2);
    fprintf(f,"OK \"ATD%c%s\"\n",
	    (atoi(pref_entries[i].PulseDial)?'P':'T'),
	    pref_entries[i].Phone);

    if (!pref_entries[i].nologin) {
      fprintf(f,"\"%s\" \"%s\"\n",
	      pref_entries[i].UserString,
	      pref_entries[i].Username);
      fprintf(f,"\"%s\" \"%s\"\n",
	      pref_entries[i].PassString,
	      pref_entries[i].Password);
    } else {
      if (!pref_entries[i].nExpectPairs)
	fprintf(f,"CONNECT \"\"\n");
    }

    for(j=0;j<pref_entries[i].nExpectPairs;j++)
      fprintf(f,"\"%s\" \"%s\"\n",
	      pref_entries[i].s_expect[j],
	      pref_entries[i].s_send[j]);

    fclose(f);
    chmod(aux,0700);
  }

  /* FILE 7 : user_yawmppp_isp# mode 600 */

  for(i=0;i<isp_count;i++) {
    sprintf(aux,"%s/%s_yawmppp_isp%d",tmp,getenv("USER"),i);
    f=fopen(aux,"w");
    if (!f) {
      message_box(GTK_WINDOW(applet),ERR_CANT_WRITE,"Error",
		  MSGBOX_OK,
		  MSGBOX_ICON_ERROR);
      return(-1);
    }

    memcpy(&modippp,&pppo,sizeof(struct PREF_PPP_OPT));
    if (pref_entries[i].ppp.override) {
      modippp.defaultroute=pref_entries[i].ppp.defaultroute;
      modippp.passive=pref_entries[i].ppp.passive;
      modippp.noauth=pref_entries[i].ppp.noauth;
      modippp.noipdefault=pref_entries[i].ppp.noipdefault;
      modippp.chap=pref_entries[i].ppp.chap;
      modippp.pap=pref_entries[i].ppp.pap;
    }

    fprintf(f,
	    "%s\n%s\nuser \"%s\"\n%sconnect '%s -v -f"\
	    " /etc/ppp/peers/%s_chat.%d'\n",
	    pref_entries[i].Device,
	    pref_entries[i].ModemSpeed,
	    pref_entries[i].Username,
	    mk_ppp_string(&modippp,"\n"),
	    chat_path,
	    getenv("USER"),
	    i);
    fclose(f);
    chmod(aux,0600);
  }

  /* FILE 8 : root_apply.sh mode 700 */
  sprintf(aux,"%s/root_apply.sh",tmp);
  f=fopen(aux,"w");
  if (!f) {
    message_box(GTK_WINDOW(applet),ERR_CANT_WRITE,"Error",
		MSGBOX_OK,
		MSGBOX_ICON_ERROR);
    return(-1);
  }

  fprintf(f,"#!/bin/sh\n");
  fprintf(f,"\n# this script was generated by yawmppp.pref (the YAWMPPP\n");
  fprintf(f,"# GUI configuration app) and will copy some files from\n");
  fprintf(f,"# ~/.yawmppp2 to /etc/ppp/peers , because pppd will only\n");
  fprintf(f,"# acknowledge input files in that directory.\n");
  fprintf(f,"#\n\n");

  strcpy(v_user,getenv("USER"));
  strcpy(v_home,getenv("HOME"));

  fprintf(f,"# create /etc/ppp/peers if missing\n");
  fprintf(f,"if [ ! -d /etc/ppp/peers ]; then mkdir -m 0755 -p /etc/ppp/peers; fi\n\n");

  for(i=0;i<isp_count;i++) {

    fprintf(f,"# ISP entry %d\n",i);

    sprintf(yet,"%s_yawmppp_isp%d",v_user,i);
    fprintf(f,"chown root %s/.yawmppp2/%s\n",v_home,yet);
    fprintf(f,"cp -f %s/.yawmppp2/%s /etc/ppp/peers/%s\n",v_home,yet,yet);
    fprintf(f,"chown %s /etc/ppp/peers/%s\n",v_user,yet);
    fprintf(f,"chown %s %s/.yawmppp2/%s\n",v_user,v_home,yet);
    fprintf(f,"chmod 600 /etc/ppp/peers/%s\n\n",yet);

    sprintf(yet,"%s_chat.%d",v_user,i);
    fprintf(f,"chown root %s/.yawmppp2/%s\n",v_home,yet);
    fprintf(f,"cp -f %s/.yawmppp2/%s /etc/ppp/peers/%s\n",v_home,yet,yet);
    fprintf(f,"chown %s /etc/ppp/peers/%s\n",v_user,yet);
    fprintf(f,"chown %s %s/.yawmppp2/%s\n",v_user,v_home,yet);
    fprintf(f,"chmod 700 /etc/ppp/peers/%s\n\n",yet);

  }

  fprintf(f,"\n# remove tag file (to tell graphical interface this script did its job)\n");
  fprintf(f,"rm -f %s/.yawmppp2/.root_didnt_run_it\n",v_home);

  fclose(f);
  chmod(aux,0700);

  /* FILE 9 : consist check */
  sprintf(aux,"%s/.root_didnt_run_it",tmp);
  f=fopen(aux,"w");
  if (f) {
    fprintf(f,"flag file\n");
    fclose(f);
  }

  /* send signal to yawmppp dock applet (so it rereads the config) */
  sprintf(aux,"%s/yawmppp.pid",tmp);
  f=fopen(aux,"r");
  if (f) {
    fgets(aux,512,f);
    fclose(f);
    i=atoi(aux);
    if (i)
      kill(i,SIGUSR1);
  }
  check_client();

  /* check if root stuff must be really done
     /bin/sh -c needed because some OSs still use the archaic C shell
     as the default shell. They even use the archaic BSD license. eek! */
  needroot=0;
  for(i=0;i<isp_count;i++) {
    sprintf(aux,"/bin/sh -c \"diff -q /etc/ppp/peers/%s_yawmppp_isp%d %s/%s_yawmppp_isp%d >/dev/null 2>/dev/null\"",
	    v_user,i,tmp,v_user,i);
    if (system(aux)) {
      sprintf(aux,"/bin/sh -c \"cp -f %s/%s_yawmppp_isp%d /etc/ppp/peers/%s_yawmppp_isp%d >/dev/null 2>/dev/null\"",
	      tmp,v_user,i,v_user,i);
      if (system(aux))
	needroot=1;
    }
    if (needroot)
      break;
    sprintf(aux,"/bin/sh -c \"diff -q /etc/ppp/peers/%s_chat.%d %s/%s_chat.%d >/dev/null 2>/dev/null\"",
	    v_user,i,tmp,v_user,i);
    if (system(aux)) {
      sprintf(aux,"/bin/sh -c \"cp -f %s/%s_chat.%d /etc/ppp/peers/%s_chat.%d >/dev/null 2>/dev/null\"",
	      tmp,v_user,i,v_user,i);
      if (system(aux))
	needroot=1;
    }
    if (needroot)
      break;
  }

  if (needroot) {
    sprintf(aux,"%s/root_apply.sh",tmp);
    run_as_root(aux);
    sprintf(aux,"%s/.root_didnt_run_it",tmp);
    f=fopen(aux,"r");
    if (f!=NULL) {
      fclose(f);
      unlink(aux);
    }
  } else {
    sprintf(aux,"%s/root_apply.sh",tmp);
    unlink(aux);
    sprintf(aux,"%s/.root_didnt_run_it",tmp);
    unlink(aux);
  }
  return 0;
}

void
applet_apply (GtkWidget * widget, gpointer data)
{
  write_and_apply_data();
}

void
applet_save_and_quit (GtkWidget * widget, gpointer data)
{
  int i;
  i=write_and_apply_data();
  if (i<0)
    if (message_box(GTK_WINDOW(applet),ERR_DIDNT_WRITE,"Confirmation",
		    MSGBOX_YESNO,
		    MSGBOX_ICON_QUESTION)==MSGBOX_R_NO)
      return;
  gtk_widget_destroy(applet);
}

void
check_client(void)
{
  char *p;
  char temp[128];
  FILE *f;

  p=getenv("HOME");
  strcpy (temp, p);
  strcat (temp, "/.yawmppp2/.delayedupdate");

  usleep(100000L);

  f=fopen(temp,"r");
  if (!f) return;
  fclose(f);
  unlink(temp);

  message_box(GTK_WINDOW(applet),
              INFO_CANT_APPLY_NOW,
              "Apply request acknowledged",
	      MSGBOX_OK,
	      MSGBOX_ICON_INFO);
}

int
file_exists(char *s)
{
	FILE *f;

	f=fopen(s,"r");
	if (!f)
		return 0;
	fclose(f);
	return 1;
}

void
find_out_paths(void)
{
	strcpy(chat_path,"/usr/sbin/chat");
	strcpy(pppd_path,"/usr/sbin/pppd");

	test_set_path(chat_path,"/bin/chat");
	test_set_path(chat_path,"/usr/bin/chat");
	test_set_path(chat_path,"/usr/local/bin/chat");
	test_set_path(chat_path,"/usr/local/sbin/chat");
	test_set_path(chat_path,"/sbin/chat");
	test_set_path(chat_path,"/usr/sbin/chat");

	test_set_path(pppd_path,"/bin/pppd");
	test_set_path(pppd_path,"/usr/bin/pppd");
	test_set_path(pppd_path,"/usr/local/bin/pppd");
	test_set_path(pppd_path,"/usr/local/sbin/pppd");
	test_set_path(pppd_path,"/sbin/pppd");
	test_set_path(pppd_path,"/usr/sbin/pppd");
}

void
test_set_path(char *dest,char *path)
{
	if (file_exists(path))
		strcpy(dest,path);
}

/* PPP PANE */

GtkWidget *
make_ppp_pane(void)
{
  GtkWidget *bv,*h,*v[4];
  GtkTooltips *tt;
  int i;
  GtkWidget *wi[40],*ew[20];
  GSList *modem,*flow;

  bv=gtk_vbox_new(FALSE,0);

  tt=gtk_tooltips_new();

  ew[0]=gtk_label_new("These options are set to a safe default."\
                      " Don't change them unless you know what you are\n"\
                      "doing. All options are documented in pppd's man"\
                      " page. Hit the Help button for details.");
  gtk_label_set_justify(GTK_LABEL(ew[0]),GTK_JUSTIFY_LEFT);

  ew[1]=gtk_hseparator_new();

  gtk_box_pack_start(GTK_BOX(bv),ew[0],FALSE,TRUE,1);
  gtk_box_pack_start(GTK_BOX(bv),ew[1],FALSE,TRUE,1);

  h=gtk_hbox_new(TRUE,2);
  gtk_box_pack_start(GTK_BOX(bv),h,TRUE,TRUE,1);

  v[0]=gtk_vbox_new(FALSE,1);
  v[1]=gtk_vbox_new(FALSE,1);
  v[2]=gtk_vbox_new(FALSE,1);
  v[3]=gtk_vbox_new(FALSE,1);
  gtk_box_pack_start(GTK_BOX(h),v[0],FALSE,TRUE,4);
  gtk_box_pack_start(GTK_BOX(h),v[1],FALSE,TRUE,4);
  gtk_box_pack_start(GTK_BOX(h),v[2],FALSE,TRUE,4);
  gtk_box_pack_start(GTK_BOX(h),v[3],FALSE,TRUE,4);

  wi[0]=gtk_check_button_new_with_label("defaultroute");
  wi[1]=gtk_check_button_new_with_label("lock");
  wi[2]=gtk_check_button_new_with_label("passive");
  wi[3]=gtk_check_button_new_with_label("debug");
  wi[4]=gtk_check_button_new_with_label("kdebug : general");
  wi[5]=gtk_check_button_new_with_label("kdebug : receive");
  wi[6]=gtk_check_button_new_with_label("kdebug : send");
  wi[7]=gtk_check_button_new_with_label("noauth");
  wi[8]=gtk_check_button_new_with_label("noipdefault");
  wi[30]=gtk_check_button_new_with_label("usepeerdns");

  gtk_tooltips_set_tip(tt,wi[0],
		       "add a default route to the system routing tables",
		       NULL);
  gtk_tooltips_set_tip(tt,wi[1],
		       "create lock file for exclusive device access",
		       NULL);
  gtk_tooltips_set_tip(tt,wi[2],
		       "wait passively for LCP packet from peer",
		       NULL);
  gtk_tooltips_set_tip(tt,wi[3],
		       "logs the control packets sent and received "\
		       "in human readable form",
		       NULL);
  gtk_tooltips_set_tip(tt,wi[4],
		       "kernel level debugging",
		       NULL);
  gtk_tooltips_set_tip(tt,wi[5],
		       "kernel level debugging",
		       NULL);
  gtk_tooltips_set_tip(tt,wi[6],
		       "kernel level debugging",
		       NULL);
  gtk_tooltips_set_tip(tt,wi[7],
		       "do not require peer to authenticate itself",
		       NULL);
  gtk_tooltips_set_tip(tt,wi[8],
		       "require IP address to be negotiated via IPCP",
		       NULL);
  gtk_tooltips_set_tip(tt,wi[30],
		       "ask DNS server info to peer, requires pppd 2.3.10 or newer, 2.3.11 recommended",
		       NULL);

  /* --- */

  ew[2]=gtk_frame_new("Line control");
  gtk_frame_set_shadow_type(GTK_FRAME(ew[2]),GTK_SHADOW_ETCHED_IN);
  ew[3]=gtk_vbox_new(TRUE,1);

  gtk_container_add(GTK_CONTAINER(ew[2]),ew[3]);

  wi[9]=gtk_radio_button_new_with_label(NULL,"modem  ");
  modem=gtk_radio_button_group(GTK_RADIO_BUTTON(wi[9]));
  wi[10]=gtk_radio_button_new_with_label(modem,"local  ");
  gtk_box_pack_start(GTK_BOX(ew[3]),wi[9],FALSE,TRUE,1);
  gtk_box_pack_start(GTK_BOX(ew[3]),wi[10],FALSE,TRUE,1);

  gtk_tooltips_set_tip(tt,wi[9],
		       "use modem control lines (CD/DTR)",
		       NULL);
  gtk_tooltips_set_tip(tt,wi[10],
		       "don't use modem control lines (ignores CD/DTR)",
		       NULL);

  /* --- */

  ew[4]=gtk_frame_new("Flow control");
  gtk_frame_set_shadow_type(GTK_FRAME(ew[4]),GTK_SHADOW_ETCHED_IN);
  ew[5]=gtk_vbox_new(TRUE,1);
  gtk_container_add(GTK_CONTAINER(ew[4]),ew[5]);

  wi[11]=gtk_radio_button_new_with_label(NULL,"crtscts");
  flow=gtk_radio_button_group(GTK_RADIO_BUTTON(wi[11]));

  wi[12]=gtk_radio_button_new_with_label(flow,"xonxoff");
  flow=gtk_radio_button_group(GTK_RADIO_BUTTON(wi[12]));

  wi[13]=gtk_radio_button_new_with_label(flow,"nocrtscts");
  flow=gtk_radio_button_group(GTK_RADIO_BUTTON(wi[13]));

  wi[14]=gtk_radio_button_new_with_label(flow,"don't care");

  gtk_box_pack_start(GTK_BOX(ew[5]),wi[11],FALSE,TRUE,1);
  gtk_box_pack_start(GTK_BOX(ew[5]),wi[12],FALSE,TRUE,1);
  gtk_box_pack_start(GTK_BOX(ew[5]),wi[13],FALSE,TRUE,1);
  gtk_box_pack_start(GTK_BOX(ew[5]),wi[14],FALSE,TRUE,1);

  gtk_tooltips_set_tip(tt,wi[11],
		       "use hardware flow control (RTS/CTS)",
		       NULL);
  gtk_tooltips_set_tip(tt,wi[12],
		       "use software flow control (XON/XOFF)",
		       NULL);
  gtk_tooltips_set_tip(tt,wi[13],
		       "disable hardware flow control (RTS/CTS)",
		       NULL);

  /* -- */

  wi[15]=hlabel_new("MTU (minimum 128)");
  wi[16]=hlabel_new("MRU (minimum 128)");

  wi[17]=hlabel_new("lcp-echo-failure\n(0 disables)");
  wi[18]=hlabel_new("lcp-echo-interval\n(0 disables)");
  wi[19]=hlabel_new("lcp-max-configure");
  wi[20]=hlabel_new("lcp-max-terminate");
  wi[21]=hlabel_new("lcp-restart");

  wi[22]=gtk_entry_new_with_max_length(12);
  wi[23]=gtk_entry_new_with_max_length(12);

  wi[24]=gtk_entry_new_with_max_length(12);
  wi[25]=gtk_entry_new_with_max_length(12);
  wi[26]=gtk_entry_new_with_max_length(12);
  wi[27]=gtk_entry_new_with_max_length(12);
  wi[28]=gtk_entry_new_with_max_length(12);

  gtk_tooltips_set_tip(tt,wi[22],
		       "Maximum Transfer Unit (bytes)",NULL);
  gtk_tooltips_set_tip(tt,wi[23],
		       "Maximum Receive Unit (bytes)",NULL);

  for(i=15;i<17;i++) {
    gtk_box_pack_start(GTK_BOX(v[2]),wi[i],FALSE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(v[2]),wi[i+7],FALSE,FALSE,0);
  }

  for(i=17;i<22;i++) {
    gtk_box_pack_start(GTK_BOX(v[3]),wi[i],FALSE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(v[3]),wi[i+7],FALSE,FALSE,0);
  }

  /* - */

  wi[29]=gtk_button_new_with_label("Set defaults");
  gtk_box_pack_end(GTK_BOX(v[3]),wi[29],FALSE,TRUE,4);

  for(i=0;i<9;i++) {
    gtk_box_pack_start(GTK_BOX(v[0]),wi[i],FALSE,FALSE,1);
    gtk_widget_show(wi[i]);
  }
  gtk_box_pack_start(GTK_BOX(v[0]),wi[30],FALSE,FALSE,1);
  gtk_widget_show(wi[30]);

  gtk_box_pack_start(GTK_BOX(v[1]),ew[2],FALSE,TRUE,4);
  gtk_box_pack_start(GTK_BOX(v[1]),ew[4],FALSE,TRUE,4);

  for(i=9;i<30;i++)
    gtk_widget_show(wi[i]);

  for(i=0;i<6;i++)
    gtk_widget_show(ew[i]);

  for(i=0;i<4;i++)
    gtk_widget_show(v[i]);

  gtk_widget_show(h);
  gtk_widget_show(bv);

  for(i=0;i<31;i++)
    ppp_pw[i]=wi[i];

  gtk_signal_connect(GTK_OBJECT(wi[29]),"clicked",
		     GTK_SIGNAL_FUNC(make_ppp_default),NULL);

  return bv;
}

void
make_ppp_default(GtkWidget *w,gpointer data)
{
  pppo.defaultroute=1;
  pppo.lock=1;
  pppo.passive=0;
  pppo.debug=0;
  pppo.kdebug=0;
  pppo.noauth=1;
  pppo.noipdefault=1;
  pppo.flowctl=3;
  pppo.mtu=1500;
  pppo.mru=1500;
  pppo.lcp[0]=0;
  pppo.lcp[1]=0;
  pppo.lcp[2]=10;
  pppo.lcp[3]=10;
  pppo.lcp[4]=3;
  pppo.chap=AUTH_DONTCARE;
  pppo.pap=AUTH_DONTCARE;
  pppo.usepeerdns=1;
  if (w!=NULL)
    set_ppp_pane();
}

void get_ppp_pane(void)
{
  int i;
  pppo.defaultroute=!!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ppp_pw[0]));
  pppo.lock        =!!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ppp_pw[1]));
  pppo.passive     =!!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ppp_pw[2]));
  pppo.debug       =!!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ppp_pw[3]));
  pppo.kdebug      =!!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ppp_pw[4]))<<0;
  pppo.kdebug     |=!!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ppp_pw[5]))<<1;
  pppo.kdebug     |=!!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ppp_pw[6]))<<2;
  pppo.noauth      =!!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ppp_pw[7]));
  pppo.noipdefault =!!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ppp_pw[8]));

  pppo.linectl     =!!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ppp_pw[10]));
  pppo.usepeerdns  =!!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ppp_pw[30]));

  for(i=11;i<15;i++)
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ppp_pw[i])))
      pppo.flowctl=i-11;

  pppo.mtu=atoi(gtk_entry_get_text(GTK_ENTRY(ppp_pw[22])));
  pppo.mru=atoi(gtk_entry_get_text(GTK_ENTRY(ppp_pw[23])));

  for(i=0;i<5;i++)
    pppo.lcp[i]=atoi(gtk_entry_get_text(GTK_ENTRY(ppp_pw[24+i])));
}

void set_ppp_pane(void)
{
  int i;
  char b[32];

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ppp_pw[0]),pppo.defaultroute);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ppp_pw[1]),pppo.lock);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ppp_pw[2]),pppo.passive);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ppp_pw[3]),pppo.debug);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ppp_pw[4]),pppo.kdebug&0x01);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ppp_pw[5]),pppo.kdebug&0x02);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ppp_pw[6]),pppo.kdebug&0x04);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ppp_pw[7]),pppo.noauth);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ppp_pw[8]),pppo.noipdefault);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ppp_pw[30]),pppo.usepeerdns);

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ppp_pw[9+pppo.linectl]),TRUE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ppp_pw[11+pppo.flowctl]),TRUE);

  sprintf(b,"%d",pppo.mtu);
  gtk_entry_set_text(GTK_ENTRY(ppp_pw[22]),b);
  sprintf(b,"%d",pppo.mru);
  gtk_entry_set_text(GTK_ENTRY(ppp_pw[23]),b);

  for(i=0;i<5;i++) {
    sprintf(b,"%d",pppo.lcp[i]);
    gtk_entry_set_text(GTK_ENTRY(ppp_pw[24+i]),b);
  }
}

char *
mk_ppp_string(struct PREF_PPP_OPT *modi,char *sep)
{
  static char here[512];
  char local[256];

  here[0]=0;
  if (modi->defaultroute) {
    strcat(here,"defaultroute");
    strcat(here,sep);
  }
  if (modi->lock) {
    strcat(here,"lock");
    strcat(here,sep);
  }
  if (modi->passive) {
    strcat(here,"passive");
    strcat(here,sep);
  }
  if (modi->debug) {
    strcat(here,"debug");
    strcat(here,sep);
  }
  if (modi->kdebug) {
    sprintf(local,"kdebug %d",modi->kdebug);
    strcat(here,local);
    strcat(here,sep);
  }
  if (modi->noauth) {
    strcat(here,"noauth");
    strcat(here,sep);
  }
  if (modi->noipdefault) {
    strcat(here,"noipdefault");
    strcat(here,sep);
  }

  switch(modi->chap) {
  case AUTH_REQUIRE:
    strcat(here,"require-chap");
    strcat(here,sep);
    break;
  case AUTH_REFUSE:
    strcat(here,"refuse-chap");
    strcat(here,sep);
    break;
  }

  switch(modi->pap) {
  case AUTH_REQUIRE:
    strcat(here,"require-pap");
    strcat(here,sep);
    break;
  case AUTH_REFUSE:
    strcat(here,"refuse-pap");
    strcat(here,sep);
    break;
  }

  if (modi->linectl)
    strcat(here,"local");
  else
    strcat(here,"modem");
  strcat(here,sep);

  switch(modi->flowctl) {
    case 0: strcat(here,"crtscts"); break;
    case 1: strcat(here,"xonxoff"); break;
    case 2: strcat(here,"nocrtscts"); break;
  }
  strcat(here,sep);

  sprintf(local,"mtu %d%smru %d%s",
	  modi->mtu,sep,
	  modi->mru,sep);

  strcat(here,local);

  if (modi->lcp[0]) {
    sprintf(local,"lcp-echo-failure %d",modi->lcp[0]);
    strcat(here,local);
    strcat(here,sep);
  }
  if (modi->lcp[1]) {
    sprintf(local,"lcp-echo-interval %d",modi->lcp[1]);
    strcat(here,local);
    strcat(here,sep);
  }
  sprintf(local,"lcp-max-configure %d",modi->lcp[2]);
  strcat(here,local);
  strcat(here,sep);
  sprintf(local,"lcp-max-terminate %d",modi->lcp[3]);
  strcat(here,local);
  strcat(here,sep);
  sprintf(local,"lcp-restart %d",modi->lcp[4]);
  strcat(here,local);
  strcat(here,sep);

  if (modi->usepeerdns) {
    strcat(here,"usepeerdns");
    strcat(here,sep);
  }
  return(here);
}

void
read_ppp_options_from_rc(char *p)
{
  FILE *f;
  char b[512],*t;
  int i;

  make_ppp_default(NULL,NULL);

  f=fopen(p,"r");
  if (!f)
    return;

  while(fgets(b,511,f)) {
    if (strstr(b,"PPPOPTIONS")==&b[0]) {
      t=wrapped_strtok(b," \t\n");
      t=wrapped_strtok(NULL," \t\n"); pppo.defaultroute=atoi(t);
      t=wrapped_strtok(NULL," \t\n"); pppo.lock=atoi(t);
      t=wrapped_strtok(NULL," \t\n"); pppo.passive=atoi(t);
      t=wrapped_strtok(NULL," \t\n"); pppo.debug=atoi(t);
      t=wrapped_strtok(NULL," \t\n"); pppo.kdebug=atoi(t);
      t=wrapped_strtok(NULL," \t\n"); pppo.noauth=atoi(t);
      t=wrapped_strtok(NULL," \t\n"); pppo.noipdefault=atoi(t);
      t=wrapped_strtok(NULL," \t\n"); pppo.linectl=atoi(t);
      t=wrapped_strtok(NULL," \t\n"); pppo.flowctl=atoi(t);
      t=wrapped_strtok(NULL," \t\n"); pppo.mtu=atoi(t);
      t=wrapped_strtok(NULL," \t\n"); pppo.mru=atoi(t);
      for(i=0;i<5;i++) {
	t=wrapped_strtok(NULL," \t\n");
	pppo.lcp[i]=atoi(t);
      }
      t=wrapped_strtok(NULL," \t\n"); pppo.usepeerdns=atoi(t);
      break;
    }
  }
  fclose(f);
}

char *
wrapped_strtok(char *a,char *b)
{
  char *pt;
  static int wasnull=0;
  static char zero[]="0";

  if ((a==NULL)&&(wasnull))
    return zero;
  if (a!=NULL)
    wasnull=0;

  pt=strtok(a,b);
  if (pt==NULL) {
    wasnull=1;
    return(zero);
  } else
    return pt;
}

/* expect/send dialog */

GtkWidget *exsdlg,*exp_cl,*exp_ctl[5];
int exp_sel=-1;
struct PREF_ISP_INFO exp_regret;

void pop_expect(GtkWidget *w,gpointer data)
{
  GtkWidget *dlg,*v1,*hs,*bhb,*bt[4],*bb,*lb[5],*wl;
  GtkWidget *sw,*cl;
  GtkStyle *style;
  GdkBitmap *mask;
  GdkPixmap *tp;
  GtkWidget *px;
  char tmp[128];
  int i;

  memcpy(&exp_regret,&pref_entries[selected_entry],sizeof(struct PREF_ISP_INFO));

  exsdlg=dlg=gtk_window_new(GTK_WINDOW_DIALOG);
  gtk_window_set_transient_for(GTK_WINDOW(advdlg),GTK_WINDOW(advdlg));
  gtk_window_set_default_size(GTK_WINDOW(dlg),400,370);
  gtk_widget_realize(dlg);

  v1=gtk_vbox_new(FALSE,1);
  gtk_container_add(GTK_CONTAINER(dlg),v1);

  sprintf(tmp,"Conversation: %s",pref_entries[selected_entry].LongName);
  gtk_window_set_title(GTK_WINDOW(dlg),tmp);
  gtk_window_set_policy(GTK_WINDOW(dlg),TRUE,TRUE,TRUE);
  gtk_container_set_border_width(GTK_CONTAINER(dlg),4);

  wl=gtk_label_new("You only need to add something here if your connection has\n"\
                   "has non-standard prompt/timing between the login and the\n"\
		   "start of the PPP session. All expect/send pairs are placed\n"\
		   "right after the user/password ones in the chat script.\n"\
		   "If you checked the Don't generate login/password pairs\n"\
		   "checkbox then the expect/send pairs defined here will be the\n"\
		   "only ones in the chat script.");

  gtk_box_pack_start(GTK_BOX(v1),wl,FALSE,FALSE,1);

  /* list */
  sw=gtk_scrolled_window_new(NULL,NULL);
  gtk_box_pack_start(GTK_BOX(v1),sw,TRUE,TRUE,4);
  gtk_widget_show(sw);
  gtk_container_set_border_width(GTK_CONTAINER(sw),4);

  exp_cl=cl=gtk_clist_new(2);
  gtk_clist_set_shadow_type(GTK_CLIST(cl),GTK_SHADOW_IN);
  gtk_clist_set_selection_mode(GTK_CLIST(cl),GTK_SELECTION_SINGLE);
  gtk_clist_set_column_title(GTK_CLIST(cl),0,"Expect");
  gtk_clist_set_column_title(GTK_CLIST(cl),1,"Send");
  gtk_clist_column_titles_passive(GTK_CLIST(cl));
  gtk_clist_column_titles_show(GTK_CLIST(cl));
  gtk_clist_set_column_width(GTK_CLIST(cl),0,120);
  gtk_clist_set_column_width(GTK_CLIST(cl),1,120);
  gtk_clist_set_column_auto_resize(GTK_CLIST(cl),0,FALSE);
  gtk_clist_set_column_auto_resize(GTK_CLIST(cl),1,FALSE);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
				 GTK_POLICY_AUTOMATIC,
				 GTK_POLICY_ALWAYS);
  gtk_container_add(GTK_CONTAINER(sw),cl);
  gtk_widget_show(cl);
  /* list */

  bb=gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(v1),bb,FALSE,TRUE,2);

  exp_ctl[0]=lb[0]=gtk_button_new_with_label(" Add pair ");
  exp_ctl[1]=lb[1]=gtk_button_new_with_label(" Add delay ");
  exp_ctl[2]=lb[2]=gtk_button_new_with_label(" Remove ");


  exp_ctl[3]=lb[3]=gtk_button_new();
  exp_ctl[4]=lb[4]=gtk_button_new();

  style=gtk_widget_get_style(dlg);

  tp=gdk_pixmap_create_from_xpm_d (dlg->window, &mask,
				   &style->bg[GTK_STATE_NORMAL],
				   (gchar **) cmd_up_xpm);
  px=gtk_pixmap_new(tp,mask); gtk_widget_show(px); gdk_pixmap_unref(tp);
  gtk_container_add(GTK_CONTAINER(lb[3]),px);

  tp=gdk_pixmap_create_from_xpm_d (dlg->window, &mask,
				   &style->bg[GTK_STATE_NORMAL],
				   (gchar **) cmd_down_xpm);
  px=gtk_pixmap_new(tp,mask); gtk_widget_show(px); gdk_pixmap_unref(tp);
  gtk_container_add(GTK_CONTAINER(lb[4]),px);

  for(i=0;i<5;i++) {
    gtk_box_pack_start(GTK_BOX(bb),lb[i],FALSE,TRUE,2);
    gtk_widget_show(lb[i]);
  }

  hs=gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(v1),hs,FALSE,FALSE,4);

  bhb=gtk_hbox_new(TRUE,4);

  gtk_box_pack_start(GTK_BOX(v1),bhb,FALSE,FALSE,4);

  bt[0]=gtk_button_new_with_label(" Help ");
  bt[1]=gtk_label_new(" ");
  bt[2]=gtk_button_new_with_label(" OK ");
  bt[3]=gtk_button_new_with_label(" Cancel ");

  for(i=0;i<4;i++) {
    gtk_box_pack_start(GTK_BOX(bhb),bt[i],FALSE,TRUE,4);
    gtk_widget_show(bt[i]);
  }

  gtk_widget_show(bb);
  gtk_widget_show(bhb);
  gtk_widget_show(hs);
  gtk_widget_show(wl);
  gtk_widget_show(v1);
  gtk_widget_show(dlg);

  exp_update();

  gtk_signal_connect (GTK_OBJECT (bt[0]), "clicked",
		      GTK_SIGNAL_FUNC (pop_help), NULL);
  gtk_signal_connect (GTK_OBJECT (bt[2]), "clicked",
		      GTK_SIGNAL_FUNC (exp_ok), NULL);
  gtk_signal_connect (GTK_OBJECT (bt[3]), "clicked",
		      GTK_SIGNAL_FUNC (exp_cancel), NULL);
  gtk_signal_connect (GTK_OBJECT (dlg), "destroy",
		      GTK_SIGNAL_FUNC (exp_destroy), NULL);

  gtk_signal_connect (GTK_OBJECT (lb[0]), "clicked",
		      GTK_SIGNAL_FUNC (exp_add), NULL);
  gtk_signal_connect (GTK_OBJECT (lb[1]), "clicked",
		      GTK_SIGNAL_FUNC (exp_add_delay), NULL);
  gtk_signal_connect (GTK_OBJECT (lb[2]), "clicked",
		      GTK_SIGNAL_FUNC (exp_remove), NULL);

  gtk_signal_connect (GTK_OBJECT (lb[3]), "clicked",
		      GTK_SIGNAL_FUNC (exp_up), NULL);
  gtk_signal_connect (GTK_OBJECT (lb[4]), "clicked",
		      GTK_SIGNAL_FUNC (exp_down), NULL);

  gtk_signal_connect (GTK_OBJECT (cl), "select_row",
		      GTK_SIGNAL_FUNC (exp_select), NULL);
  gtk_signal_connect (GTK_OBJECT (cl), "unselect_row",
		      GTK_SIGNAL_FUNC (exp_unselect), NULL);


  gtk_grab_add(dlg);
}

void
exp_update(void)
{
  int i;
  struct PREF_ISP_INFO *pii;
  char *pp[2];

  pii=&pref_entries[selected_entry];

  gtk_clist_freeze(GTK_CLIST(exp_cl));
  gtk_clist_clear(GTK_CLIST(exp_cl));
  for(i=0;i<pii->nExpectPairs;i++) {
    pp[0]=pii->s_expect[i];
    pp[1]=pii->s_send[i];
    if (!strlen(pp[0])) {
      pp[0]="<delay>";
      pp[1]="<delay>";
    }
    gtk_clist_append(GTK_CLIST(exp_cl),pp);
  }
  gtk_clist_thaw(GTK_CLIST(exp_cl));
  exp_sel=-1;
  exp_disable();
}

void
exp_ok (GtkWidget * widget, gpointer data)
{
  gtk_widget_destroy(exsdlg);
}

void
exp_cancel (GtkWidget * widget, gpointer data)
{
  memcpy(&pref_entries[selected_entry],&exp_regret,sizeof(struct PREF_ISP_INFO));
  gtk_widget_destroy(exsdlg);
}

void
exp_destroy (GtkWidget * widget, gpointer data)
{
  gtk_grab_remove(widget);
  exsdlg=NULL;
}

void
exp_up(GtkWidget *widget,gpointer data)
{
  char aux[2][32];
  struct PREF_ISP_INFO *pii;
  int nsel;

  pii=&pref_entries[selected_entry];

  strcpy(aux[0],pii->s_expect[exp_sel]);
  strcpy(aux[1],pii->s_send[exp_sel]);

  strcpy(pii->s_expect[exp_sel],pii->s_expect[exp_sel-1]);
  strcpy(pii->s_send[exp_sel],pii->s_send[exp_sel-1]);
  strcpy(pii->s_expect[exp_sel-1],aux[0]);
  strcpy(pii->s_send[exp_sel-1],aux[1]);
  nsel=--exp_sel;
  exp_update();
  gtk_clist_select_row(GTK_CLIST(exp_cl),nsel,0);
}

void
exp_down(GtkWidget *widget,gpointer data)
{
  char aux[2][32];
  struct PREF_ISP_INFO *pii;
  int nsel;
  pii=&pref_entries[selected_entry];

  strcpy(aux[0],pii->s_expect[exp_sel]);
  strcpy(aux[1],pii->s_send[exp_sel]);

  strcpy(pii->s_expect[exp_sel],pii->s_expect[exp_sel+1]);
  strcpy(pii->s_send[exp_sel],pii->s_send[exp_sel+1]);
  strcpy(pii->s_expect[exp_sel+1],aux[0]);
  strcpy(pii->s_send[exp_sel+1],aux[1]);
  nsel=++exp_sel;
  exp_update();
  gtk_clist_select_row(GTK_CLIST(exp_cl),nsel,0);
}

void
exp_disable(void)
{
  gtk_widget_set_sensitive(exp_ctl[2],FALSE);
  gtk_widget_set_sensitive(exp_ctl[3],FALSE);
  gtk_widget_set_sensitive(exp_ctl[4],FALSE);
}

void
exp_enable(void)
{
  int n;
  n=pref_entries[selected_entry].nExpectPairs;
  n--;
  gtk_widget_set_sensitive(exp_ctl[2],TRUE);
  gtk_widget_set_sensitive(exp_ctl[3],!!(exp_sel>0));
  gtk_widget_set_sensitive(exp_ctl[4],!(exp_sel==n));
}

void
exp_select(GtkCList *cl,gint row,gint column,
	   GdkEventButton *geb,gpointer data)
{
  exp_sel=row;
  exp_enable();
}

void
exp_unselect(GtkCList *cl,gint row,gint column,
	     GdkEventButton *geb,gpointer data)
{
  exp_sel=-1;
  exp_disable();
}

void
exp_add_delay(void)
{
  struct PREF_ISP_INFO *pii;
  pii=&pref_entries[selected_entry];
  pii->s_expect[pii->nExpectPairs][0]=0;
  pii->s_send[pii->nExpectPairs][0]=0;
  pii->nExpectPairs++;
  exp_update();
}

void
exp_remove(void)
{
  struct PREF_ISP_INFO *pii;
  int i;
  pii=&pref_entries[selected_entry];
  pii->nExpectPairs--;
  for(i=exp_sel;i<pii->nExpectPairs;i++) {
    strcpy(pii->s_send[i],pii->s_send[i+1]);
    strcpy(pii->s_expect[i],pii->s_expect[i+1]);
  }
  exp_sel=-1;
  exp_update();
}

GtkWidget *expa,*expa_e[2];

void
exp_add(void)
{
  GtkWidget *v1,*ea[15];
  int i;

  expa=gtk_window_new(GTK_WINDOW_DIALOG);
  gtk_window_set_title(GTK_WINDOW(expa),"Add expect/send pair");
  gtk_window_set_transient_for(GTK_WINDOW(expa),GTK_WINDOW(exsdlg));
  gtk_window_set_default_size(GTK_WINDOW(expa),220,110);
  gtk_widget_realize(expa);

  v1=gtk_vbox_new(FALSE,0);
  gtk_container_add(GTK_CONTAINER(expa),v1);
  ea[0]=gtk_hbox_new(FALSE,0);
  ea[1]=gtk_label_new("Expect:");
  expa_e[0]=ea[2]=gtk_entry_new();

  ea[3]=gtk_hbox_new(FALSE,0);
  ea[4]=gtk_label_new("Send:");
  expa_e[1]=ea[5]=gtk_entry_new();

  gtk_box_pack_start(GTK_BOX(v1),ea[0],FALSE,TRUE,1);
  gtk_box_pack_start(GTK_BOX(ea[0]),ea[1],FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX(v1),ea[2],FALSE,TRUE,1);

  gtk_box_pack_start(GTK_BOX(v1),ea[3],FALSE,TRUE,1);
  gtk_box_pack_start(GTK_BOX(ea[3]),ea[4],FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX(v1),ea[5],FALSE,TRUE,1);

  ea[6]=gtk_hseparator_new();
  ea[7]=gtk_hbox_new(TRUE,4);

  ea[8]=gtk_label_new(" ");
  ea[9]=gtk_label_new(" ");
  ea[10]=gtk_button_new_with_label(" OK ");
  ea[11]=gtk_button_new_with_label(" Cancel ");

  gtk_box_pack_start(GTK_BOX(v1),ea[6],FALSE,TRUE,2);
  gtk_box_pack_start(GTK_BOX(v1),ea[7],FALSE,TRUE,1);

  for(i=0;i<4;i++)
    gtk_box_pack_start(GTK_BOX(ea[7]),ea[8+i],FALSE,TRUE,4);

  for(i=0;i<12;i++)
    gtk_widget_show(ea[i]);
  gtk_widget_show(v1);
  gtk_widget_show(expa);

  gtk_widget_grab_focus(ea[2]);
  gtk_grab_add(expa);

  gtk_signal_connect(GTK_OBJECT(ea[10]),"clicked",
		     GTK_SIGNAL_FUNC(expa_ok),NULL);
  gtk_signal_connect(GTK_OBJECT(ea[11]),"clicked",
		     GTK_SIGNAL_FUNC(expa_cancel),NULL);
  gtk_signal_connect(GTK_OBJECT(expa),"destroy",
		     GTK_SIGNAL_FUNC(expa_destroy),NULL);
}

void
expa_ok (GtkWidget * widget, gpointer data)
{
  struct PREF_ISP_INFO *pii;
  pii=&pref_entries[selected_entry];
  strncpy(pii->s_expect[pii->nExpectPairs],gtk_entry_get_text(GTK_ENTRY(expa_e[0])),31);
  strncpy(pii->s_send[pii->nExpectPairs],gtk_entry_get_text(GTK_ENTRY(expa_e[1])),31);
  pii->nExpectPairs++;
  gtk_widget_destroy(expa);
  exp_update();
}

void
expa_cancel (GtkWidget * widget, gpointer data)
{
  gtk_widget_destroy(expa);
}

void
expa_destroy (GtkWidget * widget, gpointer data)
{
  gtk_grab_remove(widget);
  expa=NULL;
}

void
extract_delimited_string(char *dest,char *src,int count,char delim,int max)
{
  char *p,*it;
  int cur,on;
  char last;

  dest[0]=0;

  p=dest;
  it=src;
  last=0;
  on=0;

  for(cur=0;it[0]!=0;it++) {
    if ((it[0]==delim)&&(last!='\\')) {
      if (!on)
	cur++;
      on=!on;
      last=it[0];
      continue;
    }

    if ((on)&&(cur==count)) {
      p[0]=it[0];
      p[1]=0;
      p++;
      if (strlen(dest)>=max)
	return;
    }

    last=it[0];
  }
}

GtkWidget *helpwnd=NULL;

void
pop_help (GtkWidget * widget, gpointer data)
{
  GtkWidget *cw[40];
  int i;

  if (helpwnd!=NULL) {
    gdk_window_raise(helpwnd->window);
    return;
  }

  helpwnd=gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(helpwnd),"YAWMPPP Help");

  if (gdk_screen_width()>800)
    gtk_window_set_default_size(GTK_WINDOW(helpwnd),800,420);
  else
    gtk_window_set_default_size(GTK_WINDOW(helpwnd),630,420);

  gtk_window_set_position(GTK_WINDOW(helpwnd),GTK_WIN_POS_CENTER);
  gtk_widget_realize(helpwnd);

  cw[0]=gtk_vbox_new(FALSE,1);
  gtk_container_add(GTK_CONTAINER(helpwnd),cw[0]);
  gtk_container_set_border_width(GTK_CONTAINER(helpwnd),4);

  cw[1]=gtk_notebook_new();

  /* the help pages */

  cw[2]=gtk_label_new("man yawmppp");
  cw[3]=gtk_hbox_new(FALSE,1);
  cw[4]=gtk_text_new(NULL,NULL);
  cw[7]=gtk_vscrollbar_new(GTK_TEXT(cw[4])->vadj);
  gtk_box_pack_start(GTK_BOX(cw[3]),cw[4],TRUE,TRUE,1);
  gtk_box_pack_start(GTK_BOX(cw[3]),cw[7],FALSE,TRUE,1);
  gtk_notebook_append_page(GTK_NOTEBOOK(cw[1]),cw[3],cw[2]);

  cw[8]=gtk_label_new("man pppd");
  cw[9]=gtk_hbox_new(FALSE,1);
  cw[10]=gtk_text_new(NULL,NULL);
  cw[11]=gtk_vscrollbar_new(GTK_TEXT(cw[10])->vadj);
  gtk_box_pack_start(GTK_BOX(cw[9]),cw[10],TRUE,TRUE,1);
  gtk_box_pack_start(GTK_BOX(cw[9]),cw[11],FALSE,TRUE,1);
  gtk_notebook_append_page(GTK_NOTEBOOK(cw[1]),cw[9],cw[8]);

  cw[12]=gtk_label_new("man chat");
  cw[13]=gtk_hbox_new(FALSE,1);
  cw[14]=gtk_text_new(NULL,NULL);
  cw[15]=gtk_vscrollbar_new(GTK_TEXT(cw[14])->vadj);
  gtk_box_pack_start(GTK_BOX(cw[13]),cw[14],TRUE,TRUE,1);
  gtk_box_pack_start(GTK_BOX(cw[13]),cw[15],FALSE,TRUE,1);
  gtk_notebook_append_page(GTK_NOTEBOOK(cw[1]),cw[13],cw[12]);

  cw[16]=gtk_label_new("README");
  cw[17]=gtk_hbox_new(FALSE,1);
  cw[18]=gtk_text_new(NULL,NULL);
  cw[19]=gtk_vscrollbar_new(GTK_TEXT(cw[18])->vadj);
  gtk_box_pack_start(GTK_BOX(cw[17]),cw[18],TRUE,TRUE,1);
  gtk_box_pack_start(GTK_BOX(cw[17]),cw[19],FALSE,TRUE,1);
  gtk_notebook_append_page(GTK_NOTEBOOK(cw[1]),cw[17],cw[16]);

  cw[20]=gtk_label_new("FAQ");
  cw[21]=gtk_hbox_new(FALSE,1);
  cw[22]=gtk_text_new(NULL,NULL);
  cw[23]=gtk_vscrollbar_new(GTK_TEXT(cw[22])->vadj);
  gtk_box_pack_start(GTK_BOX(cw[21]),cw[22],TRUE,TRUE,1);
  gtk_box_pack_start(GTK_BOX(cw[21]),cw[23],FALSE,TRUE,1);
  gtk_notebook_append_page(GTK_NOTEBOOK(cw[1]),cw[21],cw[20]);

  cw[24]=gtk_label_new("License");
  cw[25]=gtk_hbox_new(FALSE,1);
  cw[26]=gtk_text_new(NULL,NULL);
  cw[27]=gtk_vscrollbar_new(GTK_TEXT(cw[26])->vadj);
  gtk_box_pack_start(GTK_BOX(cw[25]),cw[26],TRUE,TRUE,1);
  gtk_box_pack_start(GTK_BOX(cw[25]),cw[27],FALSE,TRUE,1);
  gtk_notebook_append_page(GTK_NOTEBOOK(cw[1]),cw[25],cw[24]);


  gtk_box_pack_start(GTK_BOX(cw[0]),cw[1],TRUE,TRUE,4);
  cw[5]=gtk_hbox_new(FALSE,4);
  cw[6]=gtk_button_new_with_label(" Dismiss ");

  gtk_box_pack_start(GTK_BOX(cw[0]),cw[5],FALSE,TRUE,2);
  gtk_box_pack_end(GTK_BOX(cw[5]),cw[6],FALSE,FALSE,4);

  for(i=0;i<28;i++)
    gtk_widget_show(cw[i]);
  gtk_widget_show(helpwnd);
  gtk_grab_add(helpwnd);

  gtk_signal_connect(GTK_OBJECT(helpwnd),"destroy",
		     GTK_SIGNAL_FUNC(help_dead),NULL);
  gtk_signal_connect(GTK_OBJECT(cw[6]),"clicked",
		     GTK_SIGNAL_FUNC(help_die),NULL);

  gtk_window_set_title(GTK_WINDOW(helpwnd),
		       "YAWMPPP Help (retrieving man yawmppp...)");
  add_man("yawmppp",cw[4],0);
  gtk_window_set_title(GTK_WINDOW(helpwnd),
		       "YAWMPPP Help (retrieving man pppd...)");
  add_man("pppd",cw[10],1);
  gtk_window_set_title(GTK_WINDOW(helpwnd),
		       "YAWMPPP Help (retrieving man chat...)");
  add_man("chat",cw[14],2);
  gtk_window_set_title(GTK_WINDOW(helpwnd),
		       "YAWMPPP Help (retrieving README...)");
  add_docfile("README",cw[18],3);
  gtk_window_set_title(GTK_WINDOW(helpwnd),
		       "YAWMPPP Help (retrieving FAQ...)");
  add_docfile("FAQ",cw[22],4);
  gtk_window_set_title(GTK_WINDOW(helpwnd),
		       "YAWMPPP Help (retrieving GPL...)");
  add_docfile("COPYING",cw[26],5);
  gtk_window_set_title(GTK_WINDOW(helpwnd),
		       "YAWMPPP Help");
}

void
help_die (GtkWidget * widget, gpointer data)
{
  gtk_grab_remove(helpwnd);
  gtk_widget_destroy(helpwnd);
}

void
help_dead (GtkWidget * widget, gpointer data)
{
  helpwnd=NULL;
}

#define YAWMPPP_CANT_MAN "help system: unable to retrieve document"

void
add_man(char *manpage,GtkWidget *text,int index)
{
  char *buf;
  char line[512];
  FILE *man;
  long blen,tlen;

  if (man_flag[index]) {
    gtk_text_freeze(GTK_TEXT(text));
    gtk_text_insert(GTK_TEXT(text),fixedfont,NULL,NULL,man_help[index],
		    strlen(man_help[index]));
    gtk_text_thaw(GTK_TEXT(text));
    while(gtk_events_pending())
      gtk_main_iteration();
    return;
  }

  sprintf(line,"man %s",manpage);
  man=popen(line,"r");

  while(gtk_events_pending()) /* let man,troff,etc. work a little */
    gtk_main_iteration();

  gtk_text_freeze(GTK_TEXT(text));
  if (man) {
    buf=(char *)g_malloc(blen=(128<<10));
    buf[0]=0; tlen=0;
    while(fgets(line,511,man)) {
      unman(line);
      strcat(buf,line);
      tlen+=strlen(line);
      if (tlen>(blen-(4<<10)))
	buf=(char *)g_realloc(buf,blen+=(10<<10));
    }
    pclose(man);
    buf=g_realloc(buf,strlen(buf)+4);
    gtk_text_insert(GTK_TEXT(text),fixedfont,NULL,NULL,buf,strlen(buf));
    man_help[index]=buf;
    man_flag[index]=1;
  } else {
    gtk_text_insert(GTK_TEXT(text),fixedfont,NULL,NULL,YAWMPPP_CANT_MAN,
		    strlen(YAWMPPP_CANT_MAN));
  }
  gtk_text_thaw(GTK_TEXT(text));
  while(gtk_events_pending())
    gtk_main_iteration();
}

void
add_docfile(char *source,GtkWidget *text,int index)
{
  FILE *f;
  char *buf;
  char line[512];
  long tlen,blen;

  if (man_flag[index]) {
    gtk_text_freeze(GTK_TEXT(text));
    gtk_text_insert(GTK_TEXT(text),fixedfont,NULL,NULL,man_help[index],
		    strlen(man_help[index]));
    gtk_text_thaw(GTK_TEXT(text));
    while(gtk_events_pending())
      gtk_main_iteration();
    return;
  }

  sprintf(line,"%s/doc/yawmppp-%s/%s",IPREFIX,VERSION,source);
  f=fopen(line,"r");

  gtk_text_freeze(GTK_TEXT(text));
  if (f) {
    buf=(char *)g_malloc(blen=(128<<10));
    buf[0]=0; tlen=0;
    while(fgets(line,511,f)) {
      strcat(buf,line);
      tlen+=strlen(line);
      if (tlen>(blen-(4<<10)))
	buf=(char *)g_realloc(buf,blen+=(10<<10));
    }
    fclose(f);
    buf=g_realloc(buf,strlen(buf)+4);
    gtk_text_insert(GTK_TEXT(text),fixedfont,NULL,NULL,buf,strlen(buf));
    man_help[index]=buf;
    man_flag[index]=1;
  } else {
    gtk_text_insert(GTK_TEXT(text),fixedfont,NULL,NULL,YAWMPPP_CANT_MAN,
		    strlen(YAWMPPP_CANT_MAN));
  }
  gtk_text_thaw(GTK_TEXT(text));
  while(gtk_events_pending())
    gtk_main_iteration();
}

void
unman(char *s)
{
  int i,fs;
  fs=strlen(s);
  for(i=0;i<fs;i++)
    if (s[i]==8) {
      memmove(s+i-1,s+i+1,fs-i);
      fs-=2;
    }
}

GtkWidget *wpwd,*pwdb[3];
int pwd_over;
char cmdtorun[512];

void
run_as_root(char *what)
{
  GtkWidget *mv,*lb,*hs,*hb,*b[3],*ehb,*pix;
  char z[1024];
  GdkPixmap *padlock;
  GdkBitmap *mask;
  GtkStyle *style;

  strcpy(cmdtorun,what);

  wpwd=gtk_window_new(GTK_WINDOW_DIALOG);
  gtk_window_set_transient_for(GTK_WINDOW(wpwd),GTK_WINDOW(applet));
  gtk_window_set_title(GTK_WINDOW(wpwd),"Apply Changes");
  gtk_window_set_policy(GTK_WINDOW(wpwd),TRUE,TRUE,TRUE);
  gtk_container_set_border_width(GTK_CONTAINER(wpwd),4);
  gtk_widget_realize(wpwd);
  style=gtk_widget_get_style(wpwd);
  padlock = gdk_pixmap_create_from_xpm_d (wpwd->window, &mask,
					  &style->bg[GTK_STATE_NORMAL],
					  (gchar **) padlock_xpm);

  /* main vbox */
  mv=gtk_vbox_new(FALSE,2);
  gtk_container_add(GTK_CONTAINER(wpwd),mv);

  sprintf(z,"To commit the changes made to the configuration\n"\
	  "YAWMPPP must run some actions with superuser\n"\
	  "privileges to write down PPP configuration to the\n"\
	  "/etc/ppp/peers directory. The script that will be\n"\
	  "run is %s.\n"\
	  "If you click 'Proceed' an xterm window will pop up\n"\
	  "asking the root password to run the script.\n"\
	  "You can 'Skip' and run the script on your own,\n"\
	  "but the configuration changes will only take effect\n"\
	  "after the script is properly run as root.\n"\
	  "You can examine the script that will be run clicking the\n"\
	  "'View Script' button.",what);

  ehb=gtk_hbox_new(FALSE,8);
  gtk_box_pack_start(GTK_BOX(mv),ehb,FALSE,TRUE,4);

  pix=gtk_pixmap_new(padlock,mask);
  gtk_box_pack_start(GTK_BOX(ehb),pix,FALSE,TRUE,4);

  lb=gtk_label_new(z);
  gtk_label_set_justify(GTK_LABEL(lb),GTK_JUSTIFY_LEFT);

  gtk_box_pack_start(GTK_BOX(ehb),lb,FALSE,TRUE,4);

  hs=gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(mv),hs,FALSE,TRUE,4);

  hb=gtk_hbox_new(TRUE,8);
  gtk_box_pack_start(GTK_BOX(mv),hb,FALSE,TRUE,4);

  b[0]=gtk_button_new_with_label("Proceed");
  b[1]=gtk_button_new_with_label("Skip");
  b[2]=gtk_button_new_with_label("View Script");

  gtk_box_pack_start(GTK_BOX(hb),b[0],FALSE,TRUE,2);
  gtk_box_pack_start(GTK_BOX(hb),b[1],FALSE,TRUE,2);
  gtk_box_pack_start(GTK_BOX(hb),b[2],FALSE,TRUE,2);

  gtk_widget_show(mv);
  gtk_widget_show(hs);
  gtk_widget_show(lb);
  gtk_widget_show(pix);
  gtk_widget_show(ehb);
  gtk_widget_show(hb);
  gtk_widget_show(b[0]);
  gtk_widget_show(b[1]);
  gtk_widget_show(b[2]);
  gtk_widget_show(wpwd);

  gtk_grab_add(wpwd);

  gtk_signal_connect (GTK_OBJECT (wpwd), "delete_event",
		      GTK_SIGNAL_FUNC (pwd_kill), NULL);

  gtk_signal_connect (GTK_OBJECT (b[0]), "clicked",
		      GTK_SIGNAL_FUNC (pwd_ok), NULL);
  gtk_signal_connect (GTK_OBJECT (b[1]), "clicked",
		      GTK_SIGNAL_FUNC (pwd_cancel), NULL);
  gtk_signal_connect (GTK_OBJECT (b[2]), "clicked",
		      GTK_SIGNAL_FUNC (pwd_view), NULL);

  pwdb[0]=b[0];
  pwdb[1]=b[1];
  pwdb[2]=b[2];

  pwd_over=0;
  while(!pwd_over)
    if (gtk_events_pending())
      gtk_main_iteration();
}

gint
pwd_kill (GtkWidget * widget, GdkEvent * event, gpointer data)
{
  return TRUE;
}

void
pwd_cancel(GtkWidget *gw,gpointer data)
{
  gtk_grab_remove(wpwd);
  gtk_widget_destroy(wpwd);
  wpwd=0;
  pwd_over=1;
}

void
pwd_ok(GtkWidget *gw,gpointer data)
{
  gtk_widget_set_sensitive(pwdb[0],0);
  gtk_widget_set_sensitive(pwdb[1],0);
  gtk_widget_set_sensitive(pwdb[2],0);

  if (try_run_script(cmdtorun)) {
    gtk_grab_remove(wpwd);
    gtk_widget_destroy(wpwd);
    wpwd=0;
    pwd_over=1;
  } else {
    message_box(GTK_WINDOW(wpwd),
		ERR_CANT_SU,"Error",
		MSGBOX_OK,MSGBOX_ICON_EXCLAMATION);
    gtk_widget_set_sensitive(pwdb[0],TRUE);
    gtk_widget_set_sensitive(pwdb[1],TRUE);
    gtk_widget_set_sensitive(pwdb[2],TRUE);
  }
}

GtkWidget *swnd;

void
pwd_view(GtkWidget *gw,gpointer data)
{
  char z[512],x[256];
  FILE *f;
  GtkWidget *v,*txt,*hs,*hb,*b_close,*hh,*vsb;
  GdkFont *fixed;

  fixed=gdk_font_load("-*-fixed-medium-r-normal--14-*-*-*-*-*-*");

  sprintf(z,"View Script: %s",cmdtorun);

  swnd=gtk_window_new(GTK_WINDOW_DIALOG);
  gtk_window_set_default_size(GTK_WINDOW(swnd),600,500);
  gtk_window_set_transient_for(GTK_WINDOW(swnd),GTK_WINDOW(wpwd));
  gtk_window_set_title(GTK_WINDOW(swnd),z);
  gtk_window_set_policy(GTK_WINDOW(swnd),TRUE,TRUE,TRUE);
  gtk_container_set_border_width(GTK_CONTAINER(swnd),4);

  v=gtk_vbox_new(FALSE,2);
  gtk_container_add(GTK_CONTAINER(swnd),v);

  hh=gtk_hbox_new(FALSE,0);
  txt=gtk_text_new(NULL,NULL);
  vsb=gtk_vscrollbar_new(GTK_TEXT(txt)->vadj);

  gtk_box_pack_start(GTK_BOX(hh),txt,TRUE,TRUE,0);
  gtk_box_pack_start(GTK_BOX(hh),vsb,FALSE,TRUE,0);
  gtk_box_pack_start(GTK_BOX(v),hh,TRUE,TRUE,2);

  hs=gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(v),hs,FALSE,FALSE,4);

  hb=gtk_hbox_new(FALSE,8);
  gtk_box_pack_start(GTK_BOX(v),hb,FALSE,TRUE,4);

  b_close=gtk_button_new_with_label(" Close ");
  gtk_box_pack_end(GTK_BOX(hb),b_close,FALSE,TRUE,4);

  GTK_WIDGET_SET_FLAGS(b_close,GTK_CAN_DEFAULT);

  gtk_text_freeze(GTK_TEXT(txt));
  f=fopen(cmdtorun,"r");
  if (f) {
    while(fgets(x,255,f))
      gtk_text_insert(GTK_TEXT(txt),fixed,NULL,NULL,x,-1);
    fclose(f);
  }

  gtk_text_thaw(GTK_TEXT(txt));

  gtk_widget_show(b_close);
  gtk_widget_show(hb);
  gtk_widget_show(txt);
  gtk_widget_show(vsb);
  gtk_widget_show(hh);
  gtk_widget_show(hs);
  gtk_widget_show(v);
  gtk_widget_show(swnd);

  gtk_grab_add(swnd);
  gtk_widget_grab_default(b_close);

  gtk_signal_connect (GTK_OBJECT (swnd), "delete_event",
		      GTK_SIGNAL_FUNC (pwd_kill), NULL);
  gtk_signal_connect (GTK_OBJECT (b_close), "clicked",
		      GTK_SIGNAL_FUNC (spwd_close), NULL);

  if (fixed)
    gdk_font_unref(fixed);
}

void
spwd_close(GtkWidget *gw,gpointer data)
{
  gtk_grab_remove(swnd);
  gtk_widget_destroy(swnd);
}

int
try_run_script(char *cmd)
{
  int xtermpid,status;
  char fcmd[512],aux[256];
  FILE *f;

  xtermpid=fork();
  if (xtermpid<0) return 0;

  if (xtermpid) {
    /* parent */
    while (!waitpid(xtermpid,&status,WNOHANG))
      if (gtk_events_pending())
	gtk_main_iteration();
      else
	usleep(100000L);

    if (WIFEXITED(status))
      if (WEXITSTATUS(status))
	return 0;

    /* check if script ran successfully */
    sprintf(aux,"%s/.yawmppp2/.root_didnt_run_it",getenv("HOME"));
    f=fopen(aux,"r");
    if (f) {
      fclose(f);
      return 0;
    }

    return 1;
  } else {
    /* child */
    sprintf(fcmd,"echo \"(YAWMPPP)\" ; echo \"Enter root password for running %s\" ; su -c %s",
	    cmd,cmd);
    execlp("xterm","xterm",
	   "-geometry","75x10",
	   "-fn","-*-fixed-medium-r-normal--15-*-*-*-*-*-*",
	   "-title","xterm - YAWMPPP [Apply Changes]",
	   "-e","/bin/sh","-c",fcmd,0);
    execlp("rxvt","rxvt",
	   "-geometry","75x10",
	   "-fn","-*-fixed-medium-r-normal--15-*-*-*-*-*-*",
	   "-title","xterm - YAWMPPP [Apply Changes]",
	   "-e","/bin/sh","-c",fcmd,0);
    execlp("aterm","aterm",
	   "-geometry","75x10",
	   "-fn","-*-fixed-medium-r-normal--15-*-*-*-*-*-*",
	   "-title","xterm - YAWMPPP [Apply Changes]",
	   "-e","/bin/sh","-c",fcmd,0);
    execlp("eterm","eterm",
	   "-geometry","75x10",
	   "-fn","-*-fixed-medium-r-normal--15-*-*-*-*-*-*",
	   "-title","xterm - YAWMPPP [Apply Changes]",
	   "-e","/bin/sh","-c",fcmd,0);
    /* well, we really tried ... */
    exit(1);
  }
}
