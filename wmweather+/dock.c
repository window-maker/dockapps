#include "config.h"

/*  Copyright (C) 2002  Brad Jorsch <anomie@users.sourceforge.net>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include <stdio.h>
#include <stdlib.h>
#if TM_IN_SYS_TIME
# if TIME_WITH_SYS_TIME
#  include <sys/time.h>
#  include <time.h>
# else
#  if HAVE_SYS_TIME_H
#   include <sys/time.h>
#  else
#   include <time.h>
#  endif
# endif
#else
#include <time.h>
#endif
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>

#include "wmgeneral/wmgeneral-x11.h"
#include "wmgeneral/mouse_regions.h"
#include "wmgeneral/xpm_trans.h"

#include "wmweather_master.xpm"
static int wmweather_mask_width;
static int wmweather_mask_height;
static char *wmweather_mask_bits;

#include "wmweather+.h"
#include "convert.h"
#include "metar.h"
#include "avn.h"
#include "eta.h"
#include "mrf.h"
#include "warnings.h"
#include "forecast.h"
#include "radar.h"
#include "animation.h"
#include "die.h"
#include "font.h"

/* Globals */
int current_mode;
struct forecast *cur_forecast;
int window_X, window_Y;

#define X 4
#define P 4
#define M (((100/P)+1)*X)
static struct forecast *last_fcst;
static int last_font=-1;
static time_t last_time=0;
static int but_stat=-1;
static int dclick=0;
static int dclick_counter=-1;
static time_t update_time;
static int forecast_priority, last_priority;
static struct animation anim;
static int counter_timer=0;
static int show_counter;
static int min_pct;
static int sigs=0;


/* Prototypes */

void DrawDisplay(int force);


/* Functions */

void sigusr2(int i){
    sigs |= 2;
    if(signal(SIGUSR2, sigusr2)==SIG_ERR)
        warn("Error setting SIGUSR2 signal handler!");
}

void sigusr1(int i){
    sigs |= 1;
    if(signal(SIGUSR1, sigusr1)==SIG_ERR)
        warn("Error setting SIGUSR1 signal handler!");
}

void sigfunc(int i){
    sigs |= i<<8;
    if(signal(i, sigfunc)==SIG_ERR)
        warn("Error setting %d signal handler!", i);
}

void do_cleanup(void){
    metar_cleanup();
    warnings_cleanup();
    avn_cleanup();
    eta_cleanup();
    mrf_cleanup();
    radar_cleanup();
}

void init_dock(int argc, char **argv){
    sscanf(wmweather_master_xpm[0], "%d %d %*s", &wmweather_mask_width, &wmweather_mask_height);
    wmweather_mask_bits=malloc((wmweather_mask_width+7)/8*wmweather_mask_height);
    if(!wmweather_mask_bits) die("malloc failed");
    createXBMfromXPM(wmweather_mask_bits, wmweather_master_xpm, wmweather_mask_width, wmweather_mask_height);
    openDockWindow(argc, argv, wmweather_master_xpm, wmweather_mask_bits, wmweather_mask_width, wmweather_mask_height);

    AddMouseRegion(0, 5, 5, 23, 14);   /* Cur button */
    AddMouseRegion(1, 23, 5, 41, 14);  /* Fcst burron */
    AddMouseRegion(2, 41, 5, 59, 14);  /* Map button */
    AddMouseRegion(3, 5, 17, 59, 59); /* Large window */
    AddMouseRegion(4, 5, 17, 11, 24);   /* left forecast arrow */
    AddMouseRegion(5, 53, 17, 59, 24);  /* right forecast arrow */
    AddMouseRegion(6, 14, 17, 50, 24); /* forecast little window */
    AddMouseRegion(7, 5, 27, 59, 59); /* forecast big window */

    init_metar();
    if(warning_zones) init_warnings();
    if(avn_station) init_avn();
    if(eta_station) init_eta();
    if(mrf_station) init_mrf();
    init_radar();
    errno=0;
    if(atexit(do_cleanup)) warn("atexit() failed, files will not be cleaned up\n");

    current_mode=starting_mode;
    window_X=0; window_Y=0;
    cur_forecast=NULL;
    last_fcst=NULL;
    last_font=-1;
    last_time=0;
    anim.do_animate=start_do_animation;
    anim.show_counter=0;
    anim.changed=1;
    anim.min_pct=1;
    anim.old_pct=0;
    min_pct=20;
    show_counter=0;
    but_stat=-1;
    dclick=0;
    dclick_counter=-1;
    update_time=0;
    forecast_priority=4;
    last_priority=-1;

    if(signal(SIGUSR1, sigusr1)==SIG_ERR)
        warn("Error setting SIGUSR1 signal handler!");
    if(signal(SIGUSR2, sigusr2)==SIG_ERR)
        warn("Error setting SIGUSR2 signal handler!");
    if(signal(SIGHUP, sigfunc)==SIG_ERR)
        warn("Error setting SIGHUP signal handler!");
    if(signal(SIGINT, sigfunc)==SIG_ERR)
        warn("Error setting SIGINT signal handler!");
    if(signal(SIGPIPE, sigfunc)==SIG_ERR)
        warn("Error setting SIGPIPE signal handler!");
    if(signal(SIGTERM, sigfunc)==SIG_ERR)
        warn("Error setting SIGTERM signal handler!");

    DrawDisplay(1);
}


void update_dock(){
    XEvent Event;
    int i=0, j;
    int exposeflag=0;

    j=sigs;
    sigs=0;
    if(j){
        if(j&~3) exit(j);
        if(j&1) i=current_mode;
        if(j&2) i=-1;
        if(j&3){
            switch(i){
              case 0:
                update_metar(1);
                break;
              case 1:
                update_avn(1);
                update_eta(1);
                update_mrf(1);
                break;
              case 2:
                update_radar(1);
                break;
              default:
                update_metar(1);
                update_avn(1);
                update_eta(1);
                update_mrf(1);
                update_radar(1);
                break;
            }
        }
    }

    if(update_time<time(NULL)){
        update_time=time(NULL)+2;
        update_metar(0);
        update_avn(0);
        update_eta(0);
        update_mrf(0);
        update_radar(0);
        forecast_priority=(forecast_priority+1)%5;
        DrawDisplay(0);
    }
    if(counter_timer>0) if(!--counter_timer){
        anim.changed=1;
        show_counter=anim.show_counter=0;
    }

    if(dclick_counter>-1) dclick_counter--;

    while(XPending(display)){
        XNextEvent(display, &Event);
        switch (Event.type){
          case GraphicsExpose:
          case NoExpose:
          case Expose:
            exposeflag=1;
            break;
          case DestroyNotify:
            XCloseDisplay(display);
            exit(0);
            break;
          case ButtonPress:
            but_stat = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
            if((dclick!=but_stat+1 && dclick!=-but_stat-1) || dclick_counter<0){
                dclick=-but_stat-1;
                dclick_counter=4;
            } else if(dclick==-but_stat-1) dclick=but_stat+1;

            switch(but_stat){
              case 0:
              case 1:
              case 2:
                if(current_mode!=but_stat){
                    current_mode=but_stat;
                    if(!anim.do_animate && !show_counter){
                        show_counter=1;
                        counter_timer=10;
                    }
                    DrawDisplay(1);
                }
                break;
              case 3:
              case 7:
                switch(Event.xbutton.button){
                  case 2:
                    if(current_mode==2){
                        if(radar_cross!=NULL){
                            do_radar_cross=1;
                            DrawDisplay(1);
                        }
                    } else {
                        anim.changed=1;
                        anim.show_counter=0;
                        anim.do_animate=!anim.do_animate;
                        anim.min_pct=1;
                        if(!anim.do_animate && current_mode==1){
                            anim.min_pct=min_pct;
                            show_counter=anim.show_counter=1;
                            counter_timer=20;
                        }
                    }
                    break;

                  case 4:
                    if(current_mode==1 && !anim.do_animate && min_pct<100){
                        if(Event.xbutton.state&ShiftMask){
                            min_pct+=10;
                            if(min_pct>100) min_pct=100;
                        } else min_pct++;
                        anim.min_pct=min_pct;
                        show_counter=anim.show_counter=1;
                        counter_timer=20;
                        anim.changed=1;
                    }
                    break;

                  case 5:
                    if(current_mode==1 && !anim.do_animate && min_pct>0){
                        if(Event.xbutton.state&ShiftMask){
                            min_pct-=10;
                            if(min_pct<0) min_pct=0;
                        } else min_pct--;
                        anim.min_pct=min_pct;
                        anim.show_counter=1;
                        counter_timer=20;
                        anim.changed=1;
                    }
                    break;

                  case 6:
                    if(current_mode==1){
                        show_counter=anim.show_counter=1;
                        anim.changed=1;
                        counter_timer=0;
                    }
                    break;
                }
                break;
                
              case 4:
                copyPixmapArea(123, 96, 6, 7, 65, 17);
                break;
              case 5:
                copyPixmapArea(129, 96, 6, 7, 113, 17);
                break;
            }
            break;

          case ButtonRelease:
            i = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
            if(dclick_counter<0) dclick=0;

            if(but_stat==4){
                copyPixmapArea(123, 89, 6, 7, 65, 17);
            }
            if(but_stat==5){
                copyPixmapArea(129, 89, 6, 7, 113, 17);
            }
            if(current_mode==2 && (but_stat==3 || but_stat==7)
               && Event.xbutton.button==2){
                do_radar_cross=0;
                DrawDisplay(1);
            }

            if(but_stat == i && but_stat >= 0){
                switch(but_stat){
                  case 3:
                  case 7:
                    if(Event.xbutton.button==1){
                        if(dclick==but_stat+1) kill(getpid(), SIGUSR1);
                        else if(warning_zones) output_warnings(0);
                    }
                    if(Event.xbutton.button==2){
                        if(dclick==but_stat+1 && current_mode==1){
                            show_counter=anim.show_counter=!anim.show_counter;
                            anim.changed=1;
                            counter_timer=0;
                        }
                    }
                    if(Event.xbutton.button==3 && warning_zones) output_warnings(1);
                    if(Event.xbutton.button==6){
                        show_counter=anim.show_counter=0;
                        anim.changed=1;
                    }
                    break;
                  case 4:
                    current_forecast_next(-1);
                    forecast_priority=4;
                    last_priority=0;
                    DrawDisplay(0);
                    break;
                  case 5:
                    current_forecast_next(1);
                    forecast_priority=4;
                    last_priority=0;
                    DrawDisplay(0);
                    break;
                  case 6:
                    if(Event.xbutton.button==3) current_forecast_next(-1);
                    if(Event.xbutton.button==2){{
                        struct forecast *f=current_forecast_get();
                        struct forecast *g;
                        current_forecast_next(1);
                        while((g=current_forecast_get())!=f){
                            if((f->hour<0 && g->hour>=0) ||
                               (f->hour>=0 && g->hour<0)) break;
                            current_forecast_next(1);
                        }
                    }}
                    if(Event.xbutton.button==1) current_forecast_next(1);
                    forecast_priority=4;
                    last_priority=0;
                    DrawDisplay(0);
                    break;
                }
                but_stat=-1;
            }
            break;
        }
    }
    if(exposeflag){
        setMaskXY(-window_X, -window_Y);
        RedrawWindowXY(window_X, window_Y);
    }
    DoAnimation(&anim);
}


void DrawDisplay(int force){
    int font=0;
    int x, y, z;
    struct forecast *f;
    time_t t;
    struct tm *tm;

    if(current_warnings) font=1;
    if(force || last_font!=font) last_time=-1;
    last_font=font;
    
    switch(current_mode){
      case 0:
        if(last_time==current.last_update) break;
        last_time=current.last_update;
        EnableMouseRegion(3);
        DisableMouseRegion(4);
        DisableMouseRegion(5);
        DisableMouseRegion(6);
        DisableMouseRegion(7);
        window_X=0; window_Y=0;

        copyPixmapArea(124, 0, 54, 9, 5, 5);
        copyPixmapArea(124, 9, 18, 9, 5, 5);
        copyPixmapArea(124, 18, 54, 42, 5, 17);

        DrawString(7, 17, metar_station, font+1);
        if(current.month>0 && current.month<13 && current.date!=-1){
            snprintf(bigbuf, BIGBUF_LEN, "%s %d", monthnames[(int)current.month], current.date);
            DrawString(32, 17, bigbuf, font+1);
        }
        if(current.time!=-1){
            snprintf(bigbuf, BIGBUF_LEN, "%04dL  (%04dZ)", current.time, local2utc(current.time, NULL, NULL, NULL, NULL));
            DrawString(7, 23, bigbuf, font+1);
        }
        if(current.temp!=999){
            x=(temp_mode==0)?temp_C2F(current.temp):current.temp;
            if(x<-99) x=-99;
            if(x>199) x=199;
            snprintf(bigbuf, BIGBUF_LEN, "%d", x);
            DrawString(32, 29, bigbuf, font);
        }
        if(current.rh!=-1){
            DrawChar(55, 29, '%', font);
            DrawNumber(54, 29, current.rh, font);
        }
        if(current.windspeed==0){
            x=GetStringWidth("CALM");
            DrawString(32+(26-x)/2, 35, "CALM", font);
        } else {
            if(current.winddir>=0 && current.winddir<=16){
                x=GetStringWidth(directions[current.winddir]);
                DrawString(45-x, 35, directions[current.winddir], font);
            }
            switch(windspeed_mode){
              case 0:
                x=knots2mph(current.windspeed);
                break;
              case 1:
                x=knots2kph(current.windspeed);
                break;
              case 3:
                x=knots2mps(current.windspeed);
                break;
              case 4:
                x=knots2beaufort(current.windspeed);
                break;
            }
            if(x>=0 && x<1000)
                DrawNumber(58, 35, x, font);
        }
        if(current.pressure>0){
            switch(pressure_mode){
              case 1:
                snprintf(bigbuf, BIGBUF_LEN, "P:%4.0f", inHg2hPa(current.pressure));
                break;
              case 2:
                snprintf(bigbuf, BIGBUF_LEN, "P:%5.1f", inHg2mmHg(current.pressure));
                break;
              case 3:
                snprintf(bigbuf, BIGBUF_LEN, "P:%5.3f", inHg2atm(current.pressure));
                break;
              default:
                snprintf(bigbuf, BIGBUF_LEN, "P:%5.2f", current.pressure);
                break;
            }
            DrawString(32, 41, bigbuf, font);
        }
        if(current.heatindex!=999){
            x=(temp_mode==0)?current.heatindex:temp_F2C(current.heatindex);
            if(x<-99) x=-99;
            if(x>199) x=199;
            snprintf(bigbuf, BIGBUF_LEN, "HI: %d", x);
            DrawString(32, 47, bigbuf, font);
        }
        if(current.windchill!=999){
            x=(temp_mode==0)?current.windchill:temp_F2C(current.windchill);
            if(x<-99) x=-99;
            if(x>199) x=199;
            snprintf(bigbuf, BIGBUF_LEN, "WC: %d", x);
            DrawString(32, 53, bigbuf, font);
        }

        anim.show_counter=0;
        anim.min_pct=1;
        SetAnimation(&anim, 5, 28, current.sky, current.obs, current.vis,
                     current.frz, current.snow, current.rain, current.tstorm, 0,
                     current.moon);

        break;

      case 1:
        f=current_forecast_get();
        if(last_fcst!=f) last_time=-1;
        last_fcst=f;
        if(f!=NULL){
            if(last_time==f->last_update)
                goto case_1_end; /* still check bottom line priority */
            else last_time=f->last_update;
        }

        DisableMouseRegion(3);
        EnableMouseRegion(4);
        EnableMouseRegion(5);
        EnableMouseRegion(6);
        EnableMouseRegion(7);
        window_X=60; window_Y=0;
        last_priority=-1;

        copyPixmapArea(124, 0, 54, 9, 65, 5);
        copyPixmapArea(142, 9, 18, 9, 83, 5);
        copyPixmapArea(123, 89, 6, 7, 65, 17);
        copyPixmapArea(129, 89, 6, 7, 113, 17);
        copyPixmapArea(124, 18, 36, 7, 74, 17);
        copyPixmapArea(124, 18, 54, 32, 65, 27);

        if(f==NULL) break;

        t=time(NULL);
        tm=localtime(&t);
        bigbuf[0]='\0';
        if(tm->tm_mon+1==f->month && tm->tm_mday==f->day){
            if(f->hour<0) snprintf(bigbuf, BIGBUF_LEN, "TODAY");
            else snprintf(bigbuf, BIGBUF_LEN, "TODAY   %dL", f->hour);
        } else {
            x=tm->tm_mon+1;
            y=tm->tm_mday+1;
            fix_date(&x, &y, NULL, NULL);
            if(x==f->month && y==f->day){
                if(f->hour<0) snprintf(bigbuf, BIGBUF_LEN, "TOMORROW");
                else snprintf(bigbuf, BIGBUF_LEN, "TMRW   %dL", f->hour);
            } else {
                z=0;
                if(f->wday!=-1){
                    for(z=0; z<5; z++){
                        y++;
                        fix_date(&x, &y, NULL, NULL);
                        if(x==f->month && y==f->day){
                            if(f->hour<0) snprintf(bigbuf, BIGBUF_LEN, "%s",
                                                   wdaynames[(int)f->wday]);
                            else snprintf(bigbuf, BIGBUF_LEN, "%.3s   %dL",
                                          wdaynames[(int)f->wday], f->hour);
                            z=99;
                        }
                    }
                }
                if(z<99 && f->month>0 && f->day>0){
                    if(f->hour<0)
                        snprintf(bigbuf, BIGBUF_LEN, "%.3s   %d",
                                 monthnames[(int)f->month], f->day);
                    else snprintf(bigbuf, BIGBUF_LEN, "%.3s %d %dL",
                                  monthnames[(int)f->month], f->day,
                                  f->hour);
                }
            }
        }
        x=GetStringWidth(bigbuf);
        DrawString(60+(64-x)/2, 18, bigbuf, font);
        
        x=GetStringWidth(f->station);
        DrawString(118-x, 28, f->station, font+1);

        if(f->high!=999 || f->low!=999){
            DrawChar(104, 35, '/', font);
            if(f->high!=999){
                x=(temp_mode==0)?f->high:temp_F2C(f->high);
                if(x<-99) x=-99;
                if(x>199) x=199;
                DrawNumber(103, 35, x, font);
            }
            if(f->low!=999){
                x=(temp_mode==0)?f->low:temp_F2C(f->low);
                if(x<-99) x=-99;
                if(x>199) x=199;
                DrawNumber(118, 35, x, font);
            }
        }
        if(f->temp!=999){
            x=(temp_mode==0)?f->temp:temp_F2C(f->temp);
            if(x<-99) x=-99;
            if(x>199) x=199;
            snprintf(bigbuf, BIGBUF_LEN, "%d", x);
            DrawString(92, 41, bigbuf, font);
        }
        if(f->rh!=-1){
            DrawChar(115, 41, '%', font);
            DrawNumber(114, 41, f->rh, font);
        }
        if(f->windspeed==0){
            x=GetStringWidth("CALM");
            DrawString(92+(26-x)/2, 47, "CALM", font);
        } else {
            if(f->winddir>=0 && f->winddir<=16){
                x=GetStringWidth(directions[f->winddir]);
                DrawString(105-x, 47, directions[f->winddir], font);
            }
            switch(windspeed_mode){
              case 0:
                x=knots2mph(f->windspeed);
                break;
              case 1:
                x=knots2kph(f->windspeed);
                break;
              case 3:
                x=knots2mps(f->windspeed);
                break;
              case 4:
                x=knots2beaufort(f->windspeed);
                break;
            }
            if(x>=0 && x<1000)
                DrawNumber(118, 47, x, font);
        }

        anim.show_counter=show_counter;
        anim.min_pct=min_pct;
        SetAnimation(&anim, 65, 28, f->sky, f->obs, f->vis,
                     f->frz, f->snow, f->rain, f->tstorm, f->svtstorm,
                     f->moon);

case_1_end:
        if(f==NULL || forecast_priority==last_priority) break;

        /* This is a little tricky. We use the switch as a calculated goto
         * (ick) to determine which order to try things in. Fall-through is
         * intended. */
        switch(forecast_priority){
          default:
            /* WTF? Oh well, just start at the beginning */

          case 0:
            if(f->heatindex>=80 && f->heatindex!=999){
                copyPixmapArea(124, 18, 26, 5, 92, 53);
                x=(temp_mode==0)?f->heatindex:temp_F2C(f->heatindex);
                snprintf(bigbuf, BIGBUF_LEN, "HI: %d", x);
                DrawString(92, 53, bigbuf, font);
                forecast_priority=0;
                break;
            }

          case 1:
            if(f->windchill<=40 && f->windchill!=999){
                copyPixmapArea(124, 18, 26, 5, 92, 53);
                x=(temp_mode==0)?f->windchill:temp_F2C(f->windchill);
                snprintf(bigbuf, BIGBUF_LEN, "WC: %d", x);
                DrawString(92, 53, bigbuf, font);
                forecast_priority=1;
                break;
            }

          case 2:
            if(f->snowamt>0){
                copyPixmapArea(124, 18, 26, 5, 92, 53);
                if(f->snowamt==1){ x=1; y=2; }
                else if(f->snowamt==8){ x=8; y=-1; }
                else { x=f->snowamt; y=x+2; }
                if(length_mode==1){
                    x=in2cm(x); y=in2cm(y);
                }
                if(x>9 && y>9) x=9;
                if(y==-1) snprintf(bigbuf, BIGBUF_LEN, "SN:>%d", x);
                else snprintf(bigbuf, BIGBUF_LEN, "SN:%d-%d", x, y);
                DrawString(92, 53, bigbuf, font);
                forecast_priority=2;
                break;
            }

          case 3:
            if(f->precipamt>0){
                copyPixmapArea(124, 18, 26, 5, 92, 53);
                switch(f->precipamt){
                  case 1:
                    if(length_mode==0) DrawString(92, 53, "P:.01-.1", font);
                    if(length_mode==1) DrawString(92, 53, "P: 0-.25", font);
                    break;
                  case 2:
                    if(length_mode==0) DrawString(92, 53, "P:.1-.25", font);
                    if(length_mode==1) DrawString(92, 53, "P:.25-.6", font);
                    break;
                  case 3:
                    if(length_mode==0) DrawString(92, 53, "P:.25-.5", font);
                    if(length_mode==1) DrawString(92, 53, "P:.6-1.3", font);
                    break;
                  case 4:
                    if(length_mode==0) DrawString(92, 53, "P: .5-1", font);
                    if(length_mode==1) DrawString(92, 53, "P: 1-2.5", font);
                    break;
                  case 5:
                    if(length_mode==0) DrawString(92, 53, "P: 1-2", font);
                    if(length_mode==1) DrawString(92, 53, "P: 2.5-5", font);
                    break;
                  case 6:
                    if(length_mode==0) DrawString(92, 53, "P: >2", font);
                    if(length_mode==1) DrawString(92, 53, "P: >5", font);
                    break;
                  case 7:
                    if(length_mode==0) DrawString(92, 53, "P: >3", font);
                    if(length_mode==1) DrawString(92, 53, "P: >7.6", font);
                    break;
                }
                forecast_priority=3;
                break;
            }

          case 4:
            copyPixmapArea(124, 18, 26, 5, 92, 53);
            x=GetStringWidth("<")+1;
            y=GetStringWidth(f->ID)+1;
            z=GetStringWidth(">");
            DrawChar(118-x-y-z, 53, '<', font+1);
            DrawString(118-y-z, 53, f->ID, font+1);
            DrawChar(118-z, 53, '>', font+1);
            forecast_priority=4;
            break;
        }
        last_priority=forecast_priority;
        break;

      case 2:
        if(last_time==radar_update_time) break;
        last_time=radar_update_time;
        EnableMouseRegion(3);
        DisableMouseRegion(4);
        DisableMouseRegion(5);
        DisableMouseRegion(6);
        DisableMouseRegion(7);
        window_X=0; window_Y=0;

        copyPixmapArea(124, 0, 54, 9, 5, 5);
        copyPixmapArea(160, 9, 18, 9, 41, 5);
        put_radar(6, 18, font);

        anim.active=0;
        break;
    }
    DoAnimation(&anim);
}
