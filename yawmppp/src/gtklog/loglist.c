/*

   YAWMPPP - PPP dock app/helper for WindowMaker
   Copyright (C) 2000:

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

   $Id: loglist.c,v 1.1.1.1 2001/02/22 07:35:59 bergo Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <gtk/gtk.h>
#include "loglist.h"
#include "about.h"

#include "status_error.xpm"
#include "status_crash.xpm"
#include "status_ok.xpm"
#include "stepphone.xpm"

GtkWidget *applet,*loglist,*lresume[2],*hgraph,*wgraph;
GdkPixmap *iconmap[3];
GdkBitmap *maskmap[3];

GdkPixmap *hcanvas=NULL;
GdkPixmap *wcanvas=NULL;

GList *log=NULL;

/* start end difftime statuspix staustext handle name phone user */
static int colsize[8]={160,160,64,54,45,110,64,64};

static char *resumeo[]=
{
  "All time",
  "Today",
  "Yesterday",
  "This week",
  "This month",
  "This year",
  "Last 24 hours",
  "Last 7 days",
  "Last 30 days",
  "Last 365 days"
};

static char *titles[8]={"Start time",
			"End time",
			"Online time",
			"Status",
			"ISP",
			"Dialup entry",
			"Phone",
			"User"};

static char *wdays[]={
  "Sun","Mon","Tue","Wed","Thu","Fri","Sat"
};

static long hourly[24],weekly[7];

int nsrc[256];

GList *users=NULL;
GList *isps=NULL;

/* constraints */

time_t time_cons[2];
int useridx,ispidx;

int
main(int argc,char **argv)
{
  int i;

  for(i=0;i<24;i++)
    hourly[i]=0;
  for(i=0;i<7;i++)
    weekly[i]=0;
  for(i=0;i<256;i++)
    nsrc[i]=i;

  time_cons[0]=0;
  time_cons[1]=0;
  useridx=ispidx=0;

  gtk_init(&argc,&argv);
  gdk_rgb_init();
  load_log();
  create_loglist();
  update_list();

  make_resume();

  gtk_main();

  g_list_free(users);
  g_list_free(isps);
  for(i=0;i<3;i++)
    gdk_pixmap_unref(iconmap[i]);

  free_log();
  return 0;
}

void
create_loglist(void)
{
  GtkWidget *mvb,*sw,*dhb,*dhw[30];
  GdkBitmap *mask;
  GdkPixmap *myicon;
  GtkStyle *style;
  GList *pt;
  int i;

  applet=gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(applet),780,510);
  gtk_window_set_title (GTK_WINDOW (applet), "YAWMPPP Connection Log");
  gtk_window_set_wmclass(GTK_WINDOW(applet),"yawmppp","log");
  gtk_container_set_border_width(GTK_CONTAINER(applet),4);
  gtk_widget_show (applet);

  style=gtk_widget_get_style(applet);
  myicon = gdk_pixmap_create_from_xpm_d (applet->window, &mask,
				       &style->bg[GTK_STATE_NORMAL],
				       (gchar **) stepphone_xpm);
  gdk_window_set_icon (applet->window, NULL, myicon, mask);
  gdk_window_set_icon_name(applet->window,"The Log");

  mvb=gtk_vbox_new(FALSE,0);
  gtk_container_add(GTK_CONTAINER(applet),mvb);

  dhb=gtk_hbox_new(FALSE,0);
  gtk_box_pack_start(GTK_BOX(mvb),dhb,FALSE,TRUE,0);

  /* data */
  dhw[0]=gtk_label_new("Show summary for:");
  gtk_box_pack_start(GTK_BOX(dhb),dhw[0],FALSE,FALSE,4);

  dhw[1]=gtk_option_menu_new();
  gtk_box_pack_start(GTK_BOX(dhb),dhw[1],FALSE,FALSE,2);

  dhw[2]=gtk_menu_new();

  for(i=0;i<10;i++) {
    dhw[3]=gtk_menu_item_new_with_label(resumeo[i]);
    gtk_signal_connect(GTK_OBJECT(dhw[3]),"activate",
		       GTK_SIGNAL_FUNC(time_menu),&nsrc[i]);
    gtk_menu_append(GTK_MENU(dhw[2]),dhw[3]);
    gtk_widget_show(dhw[3]);
  }

  gtk_option_menu_set_menu(GTK_OPTION_MENU(dhw[1]),dhw[2]);

  dhw[12]=gtk_label_new("include entries from");
  gtk_box_pack_start(GTK_BOX(dhb),dhw[12],FALSE,FALSE,4);
  dhw[13]=gtk_option_menu_new();
  gtk_box_pack_start(GTK_BOX(dhb),dhw[13],FALSE,FALSE,2);

  dhw[14]=gtk_menu_new();

  for(i=0,pt=isps;pt!=NULL;pt=g_list_next(pt),i++) {
    dhw[3]=gtk_menu_item_new_with_label((char *)(pt->data));
    gtk_signal_connect(GTK_OBJECT(dhw[3]),"activate",
		       GTK_SIGNAL_FUNC(isp_menu),&nsrc[i]);
    gtk_menu_append(GTK_MENU(dhw[14]),dhw[3]);
    gtk_widget_show(dhw[3]);
  }

  gtk_option_menu_set_menu(GTK_OPTION_MENU(dhw[13]),dhw[14]);


  dhw[15]=gtk_label_new("include connections as");
  gtk_box_pack_start(GTK_BOX(dhb),dhw[15],FALSE,FALSE,4);
  dhw[16]=gtk_option_menu_new();
  gtk_box_pack_start(GTK_BOX(dhb),dhw[16],FALSE,FALSE,2);

  dhw[17]=gtk_menu_new();

  for(i=0,pt=users;pt!=NULL;pt=g_list_next(pt),i++) {
    dhw[3]=gtk_menu_item_new_with_label((char *)(pt->data));
    gtk_signal_connect(GTK_OBJECT(dhw[3]),"activate",
		       GTK_SIGNAL_FUNC(user_menu),&nsrc[i]);
    gtk_menu_append(GTK_MENU(dhw[17]),dhw[3]);
    gtk_widget_show(dhw[3]);
  }

  gtk_option_menu_set_menu(GTK_OPTION_MENU(dhw[16]),dhw[17]);

  dhw[5]=gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(mvb),dhw[5],FALSE,FALSE,3);

  dhw[3]=gtk_hbox_new(FALSE,4);
  gtk_box_pack_start(GTK_BOX(mvb),dhw[3],FALSE,TRUE,3);

  dhw[4]=lresume[0]=gtk_label_new("\n\n\n\n\n");
  dhw[6]=lresume[1]=gtk_label_new("\n\n\n\n\n");

  for(i=0;i<2;i++)
    gtk_label_set_justify(GTK_LABEL(lresume[i]),GTK_JUSTIFY_LEFT);

  gtk_box_pack_start(GTK_BOX(dhw[3]),dhw[4],FALSE,TRUE,4);
  gtk_box_pack_start(GTK_BOX(dhw[3]),dhw[6],FALSE,TRUE,4);

  hgraph=dhw[7]=gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA(dhw[7]),24*9+2,120);
  gtk_widget_set_events(dhw[7],GDK_EXPOSURE_MASK);

  gtk_box_pack_start(GTK_BOX(dhw[3]),dhw[7],FALSE,FALSE,4);

  wgraph=dhw[8]=gtk_drawing_area_new();
  gtk_drawing_area_size(GTK_DRAWING_AREA(dhw[8]),7*20+2,120);
  gtk_widget_set_events(dhw[8],GDK_EXPOSURE_MASK);

  gtk_box_pack_start(GTK_BOX(dhw[3]),dhw[8],FALSE,FALSE,4);

  dhw[11]=gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(mvb),dhw[11],FALSE,FALSE,4);

  dhw[9]=gtk_hbox_new(FALSE,2);
  dhw[10]=gtk_label_new("Raw log listing (unfiltered):");
  
  gtk_box_pack_start(GTK_BOX(mvb),dhw[9],FALSE,FALSE,4);
  gtk_box_pack_start(GTK_BOX(dhw[9]),dhw[10],FALSE,FALSE,2);

  /* list */

  sw=gtk_scrolled_window_new(NULL,NULL);
  gtk_box_pack_start(GTK_BOX(mvb),sw,TRUE,TRUE,4);
  gtk_widget_show(sw);
  gtk_container_set_border_width(GTK_CONTAINER(sw),0);

  loglist=gtk_clist_new(8);
  gtk_clist_set_shadow_type(GTK_CLIST(loglist),GTK_SHADOW_IN);
  gtk_clist_set_selection_mode(GTK_CLIST(loglist),GTK_SELECTION_SINGLE);
  for(i=0;i<8;i++) {
    gtk_clist_set_column_title(GTK_CLIST(loglist),i,titles[i]);
    gtk_clist_set_column_width(GTK_CLIST(loglist),i,colsize[i]);
  }
  gtk_clist_column_titles_passive(GTK_CLIST(loglist));
  gtk_clist_column_titles_show(GTK_CLIST(loglist));
  gtk_clist_set_row_height(GTK_CLIST(loglist),16);
  gtk_clist_set_column_auto_resize(GTK_CLIST(loglist),1,FALSE);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
				 GTK_POLICY_AUTOMATIC,
				 GTK_POLICY_ALWAYS);
  gtk_container_add(GTK_CONTAINER(sw),loglist);
  gtk_widget_show(loglist);

  /* bottom */

  dhw[18]=gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(mvb),dhw[18],FALSE,FALSE,4);

  dhw[19]=gtk_table_new(1,6,TRUE);
  dhw[21]=gtk_button_new_with_label(" About... ");
  dhw[22]=gtk_button_new_with_label(" Close ");

  dhw[23]=gtk_hbox_new(TRUE,6);

  gtk_box_pack_start(GTK_BOX(mvb),dhw[19],FALSE,FALSE,4);

  gtk_table_attach_defaults(GTK_TABLE(dhw[19]),dhw[23],4,6,0,1);

  gtk_box_pack_start(GTK_BOX(dhw[23]),dhw[21],FALSE,TRUE,4);
  gtk_box_pack_start(GTK_BOX(dhw[23]),dhw[22],FALSE,TRUE,4);

  for(i=0;i<24;i++)
    if (i!=20)
      gtk_widget_show(dhw[i]);
  gtk_widget_show(dhb);
  gtk_widget_show(mvb);

  iconmap[0] = gdk_pixmap_create_from_xpm_d (applet->window, &maskmap[0],
					     &style->bg[GTK_STATE_NORMAL],
					     (gchar **) status_ok_xpm);
  iconmap[1] = gdk_pixmap_create_from_xpm_d (applet->window, &maskmap[1],
					     &style->bg[GTK_STATE_NORMAL],
					     (gchar **) status_error_xpm);
  iconmap[2] = gdk_pixmap_create_from_xpm_d (applet->window, &maskmap[2],
					     &style->bg[GTK_STATE_NORMAL],
					     (gchar **) status_crash_xpm);

  /* signal plumbing */
  gtk_signal_connect (GTK_OBJECT (applet), "delete_event",
		      GTK_SIGNAL_FUNC (applet_kill), NULL);
  gtk_signal_connect (GTK_OBJECT (applet), "destroy",
		      GTK_SIGNAL_FUNC (applet_destroy), NULL);
  gtk_signal_connect (GTK_OBJECT (hgraph), "expose_event",
		      GTK_SIGNAL_FUNC (hgra_expose), NULL);
  gtk_signal_connect (GTK_OBJECT (hgraph), "configure_event",
		      GTK_SIGNAL_FUNC (hgra_configure), NULL);
  gtk_signal_connect (GTK_OBJECT (wgraph), "expose_event",
		      GTK_SIGNAL_FUNC (wgra_expose), NULL);
  gtk_signal_connect (GTK_OBJECT (wgraph), "configure_event",
		      GTK_SIGNAL_FUNC (wgra_configure), NULL);

  gtk_signal_connect (GTK_OBJECT (dhw[21]), "clicked",
		      GTK_SIGNAL_FUNC (applet_about),
		      (gpointer)GTK_WINDOW(applet));
  gtk_signal_connect (GTK_OBJECT (dhw[22]), "clicked",
		      GTK_SIGNAL_FUNC (applet_destroy), NULL);
}

void
load_log(void)
{
  FILE *f;
  char *p;
  char temp[256],aux[256];
  GList *pt;
  struct logentry *le;

  if (users)
    g_list_free(users);
  if (isps)
    g_list_free(isps);
  users=NULL;
  isps=NULL;

  users=g_list_append(users,"All users");
  isps=g_list_append(isps,"All ISPs");

  p=getenv("HOME");
  sprintf(temp,"%s/.yawmppp2/logfile",p);

  if (log)
    free_log();

  f=fopen(temp,"r");
  if (!f)
    return;

  while(fgets(aux,255,f))
    prepend_log_entry(aux);

  fclose(f);

  for(pt=log;pt!=NULL;pt=g_list_next(pt)) {
    le=(struct logentry *)(pt->data);

    if (!already_exists(le->user,users)) {
      users=g_list_append(users,(gpointer)(le->user));
    }

    if (!already_exists(le->shortname,isps))
      isps=g_list_append(isps,(gpointer)(le->shortname));
  }
}

void
prepend_log_entry(char *logline)
{
  char *p;
  struct logentry *le;

  le=(struct logentry *)g_malloc0(sizeof(struct logentry));
  
  if ((p=strtok(logline,"\t\n"))==NULL) { g_free(le); return; }
  le->start=(time_t)atol(p);

  if ((p=strtok(NULL,"\t\n"))==NULL) { g_free(le); return; }
  le->end=(time_t)atol(p);

  if ((p=strtok(NULL,"\t\n"))==NULL) { g_free(le); return; }
  le->status=atoi(p);

  if ((p=strtok(NULL,"\t\n"))==NULL) { g_free(le); return; }
  strncpy(le->longname,p,128);

  if ((p=strtok(NULL,"\t\n"))==NULL) { g_free(le); return; }
  strncpy(le->shortname,p,16);

  if ((p=strtok(NULL,"\t\n"))==NULL) { g_free(le); return; }
  strncpy(le->phone,p,32);
  
  if ((p=strtok(NULL,"\t\n"))==NULL) { g_free(le); return; }
  strncpy(le->user,p,32);

  log=g_list_prepend(log,(gpointer)(le));
}

void
free_log(void)
{
  GList *pt;

  for(pt=log;pt!=NULL;pt=g_list_next(pt))
    g_free(pt->data);

  g_list_free(log);
  log=NULL;
}

void
update_list(void)
{
  GList *pt;
  struct logentry *le;
  char tmp[8][128];
  gchar *pp[8];
  int  ontime,h,m,s,i;
  int lc;

  gtk_clist_freeze(GTK_CLIST(loglist));
  gtk_clist_clear(GTK_CLIST(loglist));

  for(i=0;i<8;i++)
    pp[i]=tmp[i];

  for(lc=0,pt=log;pt!=NULL;pt=g_list_next(pt),lc++) {
    le=(struct logentry *)(pt->data);
    strcpy(tmp[0],ctime(&(le->start)));
    strcpy(tmp[1],ctime(&(le->end)));
    ontime=(int)(le->end - le->start);
    h=ontime/3600;
    ontime-=3600*h;
    m=ontime/60;
    ontime-=60*m;
    s=ontime;
    sprintf(tmp[2],"%.4d:%.2d:%.2d",h,m,s);
    switch(le->status) {
    case 0:
      strcpy(tmp[3],"OK");
      break;
    case 1:
      strcpy(tmp[3],"Error");
      break;
    default:
      strcpy(tmp[3],"Crash");
    }
    strcpy(tmp[4],le->shortname);
    strcpy(tmp[5],le->longname);
    strcpy(tmp[6],le->phone);
    strcpy(tmp[7],le->user);

    gtk_clist_append(GTK_CLIST(loglist),pp);
    gtk_clist_set_pixtext(GTK_CLIST(loglist),lc,3,
			  tmp[3],6,
			  iconmap[le->status%3],
			  maskmap[le->status%3]);
  }

  gtk_clist_thaw(GTK_CLIST(loglist));
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
make_resume(void)
{
  static char temp[1024],aux[256];
  GList *pt;
  long olt,ldiff,avg,maior,menor;
  struct logentry *le;
  int h,m,s,
      i,n,t,
      j,o,u,
      k,p,v;
  int ec;
  int sv[3];

  for(i=0;i<24;i++)
    hourly[i]=0;
  for(i=0;i<7;i++)
    weekly[i]=0;

  strcpy(temp,"Summary:\n\n");

  olt=0;
  sv[0]=sv[1]=sv[2]=0;
  ec=0;
  maior=0;
  menor=time(NULL); /* unless you've been wired since
                       Jan 1 1970 this will work... ;-) */
  for(pt=log;pt!=NULL;pt=g_list_next(pt)) {
    le=(struct logentry *)(pt->data);

    if (time_cons[1]!=0)
      if ((le->end < time_cons[0])||(le->start > time_cons[1]))
	continue;
    if (useridx)
      if (strcmp((char *)(g_list_nth_data(users,useridx)),le->user))
	continue;
    if (ispidx)
      if (strcmp((char *)(g_list_nth_data(isps,ispidx)),le->shortname))
	continue;

    ec++;
    ldiff=(le->end - le->start);
    olt+=ldiff;
    sv[le->status]++;
    add_hourly(le->start,le->end);
    add_weekly(le->start,le->end);

    if (ldiff>maior)
      maior=ldiff;
    if (ldiff<menor)
      menor=ldiff;
  }

  sprintf(aux,"Connections: %d\n"\
	      "Disconnected OK: %d\n"\
	      "Disconnected Error: %d\n"\
              "Disconnected Crash: %d\n",ec,sv[0],sv[1],sv[2]);
  strcat(temp,aux);

  gtk_label_set_text(GTK_LABEL(lresume[0]),temp);
  
  if (ec)
    avg=olt/ec;
  else
    avg=0;

  h=olt/3600;
  olt-=3600*h;
  m=olt/60;
  olt-=60*m;
  s=olt;

  i=avg/3600;
  avg-=3600*i;
  n=avg/60;
  avg-=60*n;
  t=avg;

  j=maior/3600;
  maior-=3600*j;
  o=maior/60;
  maior-=60*o;
  u=maior;

  k=menor/3600;
  menor-=3600*k;
  p=menor/60;
  menor-=60*p;
  v=menor;

  strcpy(temp,"\n\n");
  sprintf(aux,"Online time: %.4d:%.2d:%.2d\n"\
	  "Average connection length: %.4d:%.2d:%.2d\n"\
          "Longer connection: %.4d:%.2d:%.2d\n"\
          "Shorter connection: %.4d:%.2d:%.2d\n",
	  h,m,s,
	  i,n,t,
	  j,o,u,
	  k,p,v);
  strcat(temp,aux);

  gtk_label_set_text(GTK_LABEL(lresume[1]),temp);
  gtk_widget_queue_resize(hgraph);
  gtk_widget_queue_resize(wgraph);
}

void
add_hourly(long s,long e)
{
  time_t t;
  struct tm *st;

  for(t=s;t<=e;t+=30) {
    st=localtime(&t);
    if (!st)
      exit(2);
    hourly[st->tm_hour]++;
  }
}

void
add_weekly(long s,long e)
{
  time_t t;
  struct tm *st;

  for(t=s;t<=e;t+=30) {
    st=localtime(&t);
    if (!st)
      exit(2);
    weekly[st->tm_wday]++;
  }
}

gint
hgra_expose(GtkWidget *widget,GdkEventExpose *ee,gpointer data)
{
  gdk_draw_pixmap(widget->window,
		  widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		  hcanvas,
		  ee->area.x, ee->area.y,
		  ee->area.x, ee->area.y,
		  ee->area.width, ee->area.height);
  return FALSE;
}

gint
hgra_configure(GtkWidget *widget,GdkEventConfigure *ee,gpointer data)
{
  GdkGC *mygc;
  int x,y,cx,xs;
  int i,v;
  long max;
  float fac;
  GdkFont *fn;
  char tmp[64];
  
  if (hcanvas!=NULL)
    gdk_pixmap_unref(hcanvas);
  
  mygc=gdk_gc_new(widget->window);
  
  hcanvas=gdk_pixmap_new(widget->window,x=widget->allocation.width,
			 y=widget->allocation.height,-1);
  gdk_draw_rectangle(hcanvas,widget->style->white_gc,TRUE,0,0,x,y);
  gdk_draw_rectangle(hcanvas,widget->style->black_gc,FALSE,0,0,x-1,y-1);

  max=0;
  for(i=0;i<24;i++)
    if (hourly[i]>max)
      max=hourly[i];

  if (max==0) max=1;
  fac=((float)(y-18-16))/((float)max);

  cx=1;
  xs=(x-2)/24;
  fn=gdk_font_load("-*-helvetica-medium-r-*-*-8-*-*-*-*-*-*-*");
  for (i=0;i<24;i++) {
    v=(int)(((float)hourly[i])*fac);
    gdk_rgb_gc_set_foreground(mygc,0xc0c0c0);
    gdk_draw_line(hcanvas,mygc,cx,1,cx,y-2);
    gdk_rgb_gc_set_foreground(mygc,0x004080);
    gdk_draw_rectangle(hcanvas,mygc,TRUE,cx,y-18-v,xs,v);
    cx+=xs;
  }

  gdk_rgb_gc_set_foreground(mygc,0x000000);
  for(cx=1,i=0;i<24;i+=2) {
    sprintf(tmp,"%2d",i);
    gdk_draw_string(hcanvas,fn,mygc,cx,y-6,tmp);
    cx+=2*xs;
  }

  gdk_draw_line(hcanvas,widget->style->black_gc,0,y-18,x-1,y-18);  

  gdk_rgb_gc_set_foreground(mygc,0xffffff);
  gdk_draw_rectangle(hcanvas,mygc,TRUE,0,0,22*4,13);
  gdk_rgb_gc_set_foreground(mygc,0x000000);
  gdk_draw_rectangle(hcanvas,mygc,FALSE,0,0,22*4,13);

  gdk_font_unref(fn);
  fn=gdk_font_load("-*-helvetica-medium-r-*-*-10-*-*-*-*-*-*-*");

  gdk_draw_string(hcanvas,fn,mygc,5,10,"hourly summary");

  gdk_font_unref(fn);
  gdk_gc_destroy(mygc);

  return FALSE;
}

gint
wgra_expose(GtkWidget *widget,GdkEventExpose *ee,gpointer data)
{
  gdk_draw_pixmap(widget->window,
		  widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		  wcanvas,
		  ee->area.x, ee->area.y,
		  ee->area.x, ee->area.y,
		  ee->area.width, ee->area.height);
  return FALSE;
}

gint
wgra_configure(GtkWidget *widget,GdkEventConfigure *ee,gpointer data)
{
  GdkGC *mygc;
  int x,y,cx,xs;
  int i,v;
  long max;
  float fac;
  GdkFont *fn;
  
  if (wcanvas!=NULL)
    gdk_pixmap_unref(wcanvas);
  
  mygc=gdk_gc_new(widget->window);
  
  wcanvas=gdk_pixmap_new(widget->window,x=widget->allocation.width,
			 y=widget->allocation.height,-1);
  gdk_draw_rectangle(wcanvas,widget->style->white_gc,TRUE,0,0,x,y);
  gdk_draw_rectangle(wcanvas,widget->style->black_gc,FALSE,0,0,x-1,y-1);

  max=0;
  for(i=0;i<7;i++)
    if (weekly[i]>max)
      max=weekly[i];

  if (!max) max=1;
  fac=((float)(y-18-16))/((float)max);

  cx=1;
  xs=(x-2)/7;
  fn=gdk_font_load("-*-helvetica-medium-r-*-*-8-*-*-*-*-*-*-*");
  for (i=0;i<7;i++) {
    v=(int)(((float)weekly[i])*fac);
    gdk_rgb_gc_set_foreground(mygc,0xc0c0c0);
    gdk_draw_line(wcanvas,mygc,cx,1,cx,y-2);
    gdk_rgb_gc_set_foreground(mygc,0x004080);
    gdk_draw_rectangle(wcanvas,mygc,TRUE,cx,y-18-v,xs,v);
    gdk_rgb_gc_set_foreground(mygc,0x000000);
    gdk_draw_string(wcanvas,fn,mygc,cx,y-6,wdays[i]);
    cx+=xs;
  }

  gdk_draw_line(wcanvas,widget->style->black_gc,0,y-18,x-1,y-18);  

  gdk_rgb_gc_set_foreground(mygc,0xffffff);
  gdk_draw_rectangle(wcanvas,mygc,TRUE,0,0,23*4,13);
  gdk_rgb_gc_set_foreground(mygc,0x000000);
  gdk_draw_rectangle(wcanvas,mygc,FALSE,0,0,23*4,13);

  gdk_font_unref(fn);
  fn=gdk_font_load("-*-helvetica-medium-r-*-*-10-*-*-*-*-*-*-*");

  gdk_draw_string(wcanvas,fn,mygc,5,10,"weekday summary");

  gdk_font_unref(fn);
  gdk_gc_destroy(mygc);

  return FALSE;
}

void
time_menu(GtkMenuItem *gmi,gpointer data)
{
  int x;
  struct tm *his;
  time_t t;

  x=*((int *)data);
  
  t=time(NULL);
  his=localtime(&t);

  switch(x) {
  case 0: /* all */
    time_cons[0]=time_cons[1]=0;
    break;
  case 1: /* today */
    his->tm_sec=0;
    his->tm_min=0;
    his->tm_hour=0;
    time_cons[0]=mktime(his);
    his->tm_sec=59;
    his->tm_min=59;
    his->tm_hour=23;
    time_cons[1]=mktime(his);
    break;
  case 2: /* yesterday */
    t-=24*3600;
    his=localtime(&t);
    his->tm_sec=0;
    his->tm_min=0;
    his->tm_hour=0;
    time_cons[0]=mktime(his);
    his->tm_sec=59;
    his->tm_min=59;
    his->tm_hour=23;
    time_cons[1]=mktime(his);
    break;
  case 3: /* this week */
    t-=((long)(24*3600))*((long)(his->tm_wday));
    his=localtime(&t);
    his->tm_sec=0;
    his->tm_min=0;
    his->tm_hour=0;
    time_cons[0]=mktime(his);
    time_cons[1]=time(NULL);
    break;
  case 4: /* this month */
    t-=((long)(24*3600))*((long)(his->tm_mday));
    his=localtime(&t);
    his->tm_sec=0;
    his->tm_min=0;
    his->tm_hour=0;
    time_cons[0]=mktime(his);
    time_cons[1]=time(NULL);
    break;
  case 5: /* this year */
    t-=((long)(24*3600))*((long)(his->tm_yday));
    his=localtime(&t);
    his->tm_sec=0;
    his->tm_min=0;
    his->tm_hour=0;
    time_cons[0]=mktime(his);
    time_cons[1]=time(NULL);
    break;
  case 6: /* Last 24 hours */
    time_cons[0]=t-24*3600;
    time_cons[1]=t;
    break;
  case 7: /* Last 7 days */
    time_cons[0]=t-7*24*3600;
    time_cons[1]=t;
    break;
  case 8: /* Last 30 days */
    time_cons[0]=t-30*24*3600;
    time_cons[1]=t;
    break;
  case 9:
    time_cons[0]=t-365*24*3600;
    time_cons[1]=t;
    break;
  default:
    return;
  }
  make_resume();
}

void
user_menu(GtkMenuItem *gmi,gpointer data)
{
  useridx=*((int *)data);
  make_resume();
}

void
isp_menu(GtkMenuItem *gmi,gpointer data)
{
  ispidx=*((int *)data);
  make_resume();
}

int
already_exists(char *s,GList *pt)
{
  GList *w;
  for(w=pt;w!=NULL;w=g_list_next(w)) {
    if (!strcmp((char *)(w->data),s))
      return 1;
  }
  return 0;
}
