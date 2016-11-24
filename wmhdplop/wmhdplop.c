/*
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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, 
  USA.
*/
  
#include "config.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#ifdef ENABLE_HDDTEMP_QUERY
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#endif
#define GLOBALS_HERE
#include "global.h"
#include "wmhdplop.h"

uid_t euid, uid;



void update_swap_matrix(App *app) {
  SwapMatrix *sm = &app->sm;
  unsigned i;
  int col,row;
  unsigned sw_in = ceil(get_swapin_throughput()*4), sw_out = ceil(get_swapout_throughput()*4);
  for (i=0; i < sw_in+sw_out; i++) {
    col = rand() % sm->ncol;
    row = rand() % sm->nrow;
    if (sm->intensity[row][col] == 0) {
      sm->pre_cnt[row][col] = rand() % 10; //(app->update_stats_mult);
    }
    sm->intensity[row][col] = app->swap_matrix_lighting * ((i < sw_in) ? -1 : +1);
  }
}

void update_io_matrix_rw(App *app, float mbs, int op) {
  IOMatrix *io = &app->iom;
  IO_op_lst *l;
  if (mbs > 10000) mbs = 10000; /* very quick and ugly fix for the strange bug which only occurs in gkhdplop */
  int bsz = MAX(SQR(16*64./(app->dock->w + app->dock->h)),2); /* 1024 for a 64x64 applet */
  while (mbs>1e-5) {
    float v = MIN(mbs, bsz); /* a throughput > 1MB/s will create more than one spot */
    mbs -= v;
    l = calloc(1,sizeof(*l)); assert(l);
    l->next = io->ops;
    l->n = (int)(log2f(1+v*1024)/10.);
    l->op = op;
    l->i = rand() % io->h; 
    l->j = rand() % io->w;
    io->ops = l;
  }
}

void update_io_matrix(App *app) {
  update_io_matrix_rw(app, get_read_throughput(), OP_READ);
  update_io_matrix_rw(app, get_write_throughput(), OP_WRITE);
}

#define IOSCAL 32768

void evolve_io_matrix(App *app, DATA32 * __restrict buff) {
  IOMatrix *io = &app->iom;
  int i,j;
  int * __restrict pl, * __restrict tmp;
  IO_op_lst *o = io->ops, *po = NULL, *no;
  while (o) {
    io->v[o->i+1][o->j+1] = ((o->op == OP_READ) ? +50000000 : -50000000);
    no = o->next;
    if (--o->n <= 0) {
      if (po) { po->next = no; }
      else { io->ops = no; }
      free(o);
    } else po = po->next;
    o = no;
  }
  
  /* brain-dead diffusion */
  pl = io->v[io->h+2];
  int * __restrict l = io->v[io->h+3];
  for (j=1; j < io->w+1; ++j) pl[j] = 0;  
  for (i=1; i < io->h+1; ++i) {
    /*
    for (j=1; j < io->w+1; ++j) {
      l[j] = io->v[i][j]*0.99 + (io->v[i][j-1] + io->v[i][j+1] + pl[j] + io->v[i+1][j] - 4*io->v[i][j])/5.;
    }
    */
  /*  
    float *__restrict nl = io->v[i+1]+1;
    float *__restrict cl = io->v[i];
    for (j=1; j < io->w+1; ++j, ++cl) {
      l[j] = (cl[1])*0.99 + (cl[0] + cl[2] + pl[j] + *nl++ - 4*cl[1])/5.;
    }
*/    
    int *__restrict dest;
    int *__restrict pn = io->v[i+1]+1;
    int *__restrict pc = io->v[i]+1;
    int *__restrict pp = pl+1;
    int pj,cj=0,nj=*pc++;
    for (j=0, dest=l+1, pj = 0.; j < io->w; ++j) {
      pj = cj; cj = nj; nj = *pc++;
      *dest = (cj * 37)/200 + (pj + nj + *pp++ + *pn++)/5;
      /* *dest = 99*(cj + (pj + nj + *pp++ + *pn++)/(4))/200; */
    //} for (j=0, dest=l+1; j < io->w; ++j) {
      int v = (int)(*dest++ >> 10);
      if (v == 0) { *buff++ = io->cm.p[CMAPSZ/2]; continue; }
      if (v > CMAPSZ/4) { /* cheaper than a log(vv) ... */
        v = MIN(CMAPSZ/4  + (v-CMAPSZ/4)/16,CMAPSZ/2-1);
      } else if (v < -CMAPSZ/4) {
        v = MAX(-CMAPSZ/4 + (v+CMAPSZ/4)/16,-CMAPSZ/2);
      }
      *buff++ = io->cm.p[v+CMAPSZ/2];
    }
      
    tmp = pl; pl = io->v[i]; io->v[i] = l; l = tmp;
    io->v[io->h+2] = pl; io->v[io->h+3] = l;
  }
}


#if 0
static void draw_io_matrix(App *app, DATA32 * __restrict buff) {
  IOMatrix *io = &app->iom;
  int i,j;
  for (i=0; i < io->h; ++i) {
    for (j=0; j < io->w; ++j) {  
      float vv = io->v[i+1][j+1];
      int v = (int)(vv * 32768); //((int)ldexpf(vv,15)); /* (int)v*(2^15) */
      //if (v == 0) continue; 
      //if (v < 0) v*=3; /* write op are rare, so they are brighter .. */
      if (v > CMAPSZ/4) { /* cheaper than a log(vv) ... */
        v = MIN(CMAPSZ/4  + (v-CMAPSZ/4)/16,CMAPSZ/2-1);
      } else if (v < -CMAPSZ/4) {
        v = MAX(-CMAPSZ/4 + (v+CMAPSZ/4)/16,-CMAPSZ/2);
      }
      //assert(v+CMAPSZ/2>=0);
      //assert(v+CMAPSZ/2 < sizeof io->cm.p);
      *buff++ /*[j+i*io->w]*/ = io->cm.p[v+CMAPSZ/2];
    }
  }
}
#endif

void draw_swap_matrix(App *app) {
  SwapMatrix *sm = &app->sm;
  int rcnt[sm->nrow+1], ccnt[sm->ncol+1];
  int row, col;
  int isswapping = 0;
  static int darkcnt = 0;
  memset(rcnt, 0, sizeof rcnt); memset(ccnt, 0, sizeof ccnt);
  for (row = 0; row < sm->nrow; row++) {
    for (col = 0; col < sm->ncol; col++) {
      if (sm->pre_cnt[row][col]>0) { isswapping = 1; sm->pre_cnt[row][col]--; }
    }
  }
  if (isswapping) darkcnt = MIN(darkcnt+1,7);
  else darkcnt = MAX(darkcnt-1, 0);

  /* darken everything */
  if (darkcnt) {
    imlib_context_set_color(0, 0, 0, (darkcnt+1)*16);
    imlib_image_fill_rectangle(0, 0, app->dock->w, app->dock->h);
  }
  /* draw squares */
  for (row = 0; row < sm->nrow; row++) {
    for (col = 0; col < sm->ncol; col++) {
      if (sm->intensity[row][col] && sm->pre_cnt[row][col]==0) {
	int v = sm->intensity[row][col];
        v = (v * app->swap_matrix_luminosity)/(int)app->swap_matrix_lighting;
	rcnt[row  ] = MAX(rcnt[row  ],abs(v)); ccnt[col  ] = MAX(ccnt[col  ],abs(v));
	rcnt[row+1] = MAX(rcnt[row+1],abs(v)); ccnt[col+1] = MAX(ccnt[col+1],abs(v));
        //v = (v*155)/255;
        /*if (v > 0) imlib_context_set_color(100+v, 155-v, 155-v, 250);
          else     imlib_context_set_color(0, 100-v, 0, 250);*/
        if (v > 0) imlib_context_set_color(255, 50, 50, MIN(v+80,255));
        else     imlib_context_set_color(50, 255, 50, MIN(v+80,255));
        imlib_image_fill_rectangle(row*sm->w+1,col*sm->w+1,sm->w-1,sm->w-1);
	
	if (sm->intensity[row][col] > 0) {
	  sm->intensity[row][col]--;
	} else if (sm->intensity[row][col] < 0) {
	  sm->intensity[row][col]++;
	}
      }
    }
  }

  /* draw lines */
  for (row = 0; row < sm->nrow+1; row++) {
    if (rcnt[row]) {
      imlib_context_set_color(255, 255, 255, MIN(rcnt[row]*2/3, 255));
      imlib_image_draw_line(row*sm->w,0,row*sm->w,app->dock->w,0);
    }
  }
  for (col = 0; col < sm->ncol+1; col++) {
    if (ccnt[col]) {
      imlib_context_set_color(255, 255, 255, MIN(ccnt[col]*2/3, 255));
      imlib_image_draw_line(0,col*sm->w,app->dock->h,col*sm->w,0);
    }
  }
}

float f2celsius(float Tf) {
  return (5./9.)*(Tf-32.);
}

float celsius2f(float Tc) {
  return (9./5.)*Tc+32.;
}

static void query_hddtemp(App *app) {
  int fd;
  struct hostent *h;
  struct sockaddr_in addr;
  char buff[1024];
  SET_VEC(app->disk_temperature, -1, 0, app->nb_hd);
  if ((h = gethostbyname("127.0.0.1")) == NULL) {
    fprintf(stderr, "gethostbyname(localhost) failed : %s\n", strerror(errno)); return;
  }
  if ((fd = socket(h->h_addrtype, SOCK_STREAM,0)) == -1) {
    fprintf(stderr, "can't create socket : %s\n", strerror(errno)); return;
  }
  memset(&addr,0,sizeof(struct sockaddr_in));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(Prefs.hddtemp_port);
  addr.sin_addr.s_addr = ((struct in_addr*)(h->h_addr))->s_addr;
  if (connect(fd, (struct sockaddr *)&addr, sizeof addr) < 0) {
    close(fd);
    fprintf(stderr,"can't connect to 127.0.0.1:%d : %s\n", Prefs.hddtemp_port, strerror(errno));
#ifdef GKRELLM
    Prefs.enable_hddtemp = 0;
#endif 
    return;
  }
  int n = 0;
  do {
    int nn=read(fd,buff+n,MAX((int)(sizeof buff) - n,0));
    if (nn <= 0) { if (nn < 0) n=nn; break; }
    n+=nn;
  } while (n != sizeof buff);

  BLAHBLAH(2,printf("n=%d, err=%s\n",n, strerror(errno)));
  if (n != -1) {
    char *s;
    DiskList *dl;
    int cnt;
    buff[MIN(n,(int)sizeof buff - 1)] = 0;
    for (dl = first_hd_in_list(), cnt = 0; dl; dl = next_hd_in_list(dl), cnt++) {
      if (dl->enable_hddtemp) {
        int found = 0;
        if ((s=strstr(buff, dl->dev_path))) { found = 1; s+= strlen(dl->name); }
        if (found) {
          s=strchr(s,'|');
          if (s && ((s=strchr(s+1,'|')))) {
	    int unit = 'C';
            char *p = strchr(++s,'|'), oldv=0; 
	    if (p) { 
	      oldv = *p; 
	      if (*p && toupper(p[1]) == 'F') unit = 'F';
	      *p = 0; 
	    }
            BLAHBLAH(1,printf("temperature of '%s' : %s %c\n", dl->name, s, unit));
            if (strcmp(s,"SLP") == 0) app->disk_temperature[cnt] = -2;
            else {
	      int temp = atoi(s);
	      if (unit == 'C' && Prefs.temperatures_unit == 'F') {
		temp = (int)(floor(celsius2f(temp)+.5));
	      } else if (unit == 'F' && Prefs.temperatures_unit == 'C') {
		temp = (int)(floor(f2celsius(temp)+.5));
	      }
	      app->disk_temperature[cnt] = temp;
	    }
            if (p) *p = oldv; /* or bug... */
          }
        } else {
          ONLY_NTIMES(4,fprintf(stderr, "could not find device '%s' in the output of hddtemp: '%s'\n", dl->name, buff));
        }
      }
    }
  } else { fprintf(stderr, "error with hddtemp: %s\n", strerror(errno)); }  
  close(fd);
}

const char *power_mode_str(int m) {
  switch (m) {
  case HD_ACTIVE: return "active/idle";
  case HD_STANDBY: return "standby";
  case HD_SLEEP: return "sleep";
  case HD_UNKNOWN: 
  default:
    return "unknown/error";
  }
}

void sethw(App *app, int lw, int lh, int pos, int *px, int *py, int *pw, int *ph) {
  *px = *py = 0; *pw = lw; *ph = lh;
  if (!(pos & (AL_RIGHT | AL_LEFT | AL_HCENTER))) *pw = app->dock->w;
  if (pos & AL_RIGHT) *px = app->dock->w - lw;
  else if (pos & AL_HCENTER) *px = (app->dock->w - lw)/2;
  else if (pos & AL_LEFT) *px = 0;
  else { *px = 0; *pw = app->dock->w; }
  if (pos & AL_BOTTOM) *py = app->dock->h - lh;
  else if (pos & AL_VCENTER) *py = (app->dock->h - lh)/2;
  else if (pos & AL_TOP) *py = 0;
  else { *py = 0; *ph = app->dock->h; }
}

static void my_imlib_text_draw(int x, int y, const char *s) {
  char s2[100]; snprintf(s2,100,"%s ",s); /* my imlib2 often forgets half of the last character... */
  imlib_text_draw(x,y,s2);
}

int is_displayed(int hd_id, int part_id) {
  return (((hd_id == app->filter_hd) || (app->filter_hd == -1)) &&
          ((part_id == app->filter_part) || (app->filter_part == -1)));
}

void change_displayed_hd(int dir) {
  DiskList *dl = find_id(app->filter_hd, app->filter_part);
  if (dl == NULL) { 
    app->filter_hd = -1; app->filter_part = -1; 
    dl = find_id(app->filter_hd, app->filter_part); assert(dl); 
  }
  else if (dir > 0) {
    if (app->filter_hd == -1 && app->filter_part == -1) {
      app->filter_hd = -1; app->filter_part = 0;
    } else if (app->filter_hd == -1 && app->filter_part == 0) {
      app->filter_hd = first_dev_in_list()->hd_id;
      app->filter_part = -1;
    } else if (app->filter_hd != -1 && app->filter_part != -1) {
      DiskList *dl2 = dl->next;
      if (dl2) {
        if (dl2->hd_id == app->filter_hd)
          app->filter_part = dl2->part_id;
        else {
          app->filter_hd = dl2->hd_id;
          app->filter_part = -1;
        }
      } else {
        app->filter_hd = -1;
        app->filter_part = -1;
      }
    } else if (app->filter_hd != -1) {
      app->filter_part = dl->part_id;
    } else {
      app->filter_hd = dl->hd_id;
    }
  } else if (dir < 0) { /* very very bourin */
    int ph, pph, pp, ppp;
    ph = app->filter_hd; pp = app->filter_part;
    do {
      pph = app->filter_hd; ppp = app->filter_part;
      change_displayed_hd(+1);
    } while (!(app->filter_hd == ph && app->filter_part == pp));
    app->filter_hd = pph; app->filter_part = ppp;
  }
  app->displayed_hd_changed = 1;
}

void next_displayed_hd() {
  BLAHBLAH(1,printf("next_displayed_hd() : filter_hd=%d, filter_part=%d\n", app->filter_hd, app->filter_part));
  change_displayed_hd(-1);
  init_stats(app->update_display_delay_ms*1e-3*app->update_stats_mult);
}

void prev_displayed_hd() {
  BLAHBLAH(1,printf("prev_displayed_hd() : filter_hd=%d, filter_part=%d\n", app->filter_hd, app->filter_part));
  change_displayed_hd(+1);
  init_stats(app->update_display_delay_ms*1e-3*app->update_stats_mult);
}

const char *shorten_name(DiskList *dl) {
  static char s[8];
  if (dl->name && strlen(dl->name)) {
    const char *p = dl->name; 
    if (strchr(p,'/')) p = strrchr(p,'/')+1;
    if (strlen(p) == 0) p = dl->name;
    snprintf(s, sizeof s, "%s%s", dl->part_id ? " " : "", p);
  } else strncpy(s, dl->name, sizeof s);
  return s;
}

static void draw_hdlist(App *app) {
  if (Prefs.hdlist_pos == AL_NONE) return;
  DiskList *dl;
  
  int dev_cnt, hd_cnt, w;
  static int lw = -1, lh = -1, lx = -1, ly = -1, h = -1, 
    reshape_cnt = 0;
  
  if (!app->smallfont) return;
  if (app->displayed_hd_changed) { lx = -1; app->displayed_hd_changed = 0; }
  imlib_context_set_font(app->smallfont);
  /* get dimensions */
  if (lx == -1 || reshape_cnt != app->reshape_cnt) {
    int wtemp = 1;
    lw = 0; lh = 0;
    //printf("update : first displayed(%d) = %p\n", cnt, dl);

    for (dl = first_dev_in_list(), dev_cnt=hd_cnt=-1; dl; dl = dl->next) {
      if (dl->part_id == 0) ++hd_cnt; if (!is_displayed(dl->hd_id, dl->part_id)) continue; ++dev_cnt;
      imlib_get_text_size(shorten_name(dl),&w,&h);
      lw = MAX(w,lw);
      lh += h;
    }
    if (!Prefs.disable_hd_leds) { lw += 5; }
    if (Prefs.enable_hddtemp) imlib_get_text_size(" 000",&wtemp,&h);
    if (lw + wtemp > (int)(app->dock->w*2/3)) { lw = app->dock->w; }
    else lw += wtemp;
    sethw(app,lw,lh,Prefs.hdlist_pos,&lx,&ly,&lw,&lh);
    reshape_cnt = app->reshape_cnt;
  }
  
  imlib_context_set_color(100, 100, 100, 150);
  imlib_image_fill_rectangle(lx,ly,lw,lh);
  imlib_context_set_color(100, 100, 100, 200);
  imlib_image_draw_rectangle(lx-1,ly-1,lw+2,lh+2);
  
  for (dl = first_dev_in_list(), dev_cnt=hd_cnt=-1; dl; dl = dl->next) {
    if (dl->part_id==0) ++hd_cnt; if (!is_displayed(dl->hd_id, dl->part_id)) continue; ++dev_cnt;
    int x = lx, y = ly + lh - dev_cnt * h;
    if (!Prefs.disable_hd_leds) {
      if (dl->touched_r) { 
        imlib_context_set_color(50, 255, 50, 25*dl->touched_r--); 
        imlib_image_fill_rectangle(lx+1,y-5,3,3); 
      }
      if (dl->touched_w) { 
        imlib_context_set_color(255,100-10*dl->touched_w,100-10*dl->touched_w, 25*dl->touched_w--); 
        imlib_image_fill_rectangle(lx+1,y-9,3,3); 
      }
      x += 5;
    }
    imlib_context_set_color(200, 255, 255, 200);
    my_imlib_text_draw(x,y-h,shorten_name(dl));
    if (dl->part_id==0 && app->disk_temperature[hd_cnt] != -1) {
      char s[200];
      if (app->disk_temperature[hd_cnt] != -2) { 
        snprintf(s,200,"%d",app->disk_temperature[hd_cnt]); 
      } else { 
        strcpy(s,"SLP");
      }
      imlib_get_text_size(s,&w,&h);
      x = lx + lw - w + ((app->disk_temperature[hd_cnt] != -2) ? -7 : -3);
      imlib_context_set_color(255, 250, 100, 255);
      my_imlib_text_draw(x, y-h, s);
      imlib_context_set_color(255, 255, 255, 200);

      /* below is a quick fix for the degree sign which is not drawn
	 properly by recent release of imlib -- many thanks to
	 Krzysztof Kotlenga for the patch */

      //if (app->disk_temperature[hd_cnt] != -2) imlib_image_draw_ellipse(x+w+3, y - h + 2, 1, 1);

      //if (app->disk_temperature[hd_cnt] != -2) imlib_image_draw_ellipse(x+w+3, y - h + 2, 1, 1);
      /* imlib2 >= 1.1.1 workaround - imlib_image_draw_ellipse() can't properly draw circle with radius = 1 */
      if (app->disk_temperature[hd_cnt] != -2) {
        /*ImlibPolygon poly;
        poly = imlib_polygon_new();
        imlib_polygon_add_point(poly, x+w+3, y-h+3);
        imlib_polygon_add_point(poly, x+w+2, y-h+2);
        imlib_polygon_add_point(poly, x+w+3, y-h+1);
        imlib_polygon_add_point(poly, x+w+4, y-h+2);
        imlib_image_draw_polygon(poly, 1);*/
	/*imlib_image_draw_pixel(x+w+3, y-h+3, 0);
	imlib_image_draw_pixel(x+w+2, y-h+2, 0);
	imlib_image_draw_pixel(x+w+3, y-h+1, 0);
	imlib_image_draw_pixel(x+w+4, y-h+2, 0);*/

	int i=x+w+2, j=y-h;
	imlib_image_draw_pixel(i+1,j,0);
	imlib_image_draw_pixel(i+2,j,0);
	imlib_image_draw_pixel(i+3,j+1,0);
	imlib_image_draw_pixel(i+3,j+2,0);
	imlib_image_draw_pixel(i+1,j+3,0);
	imlib_image_draw_pixel(i+2,j+3,0);
	imlib_image_draw_pixel(i,j+1,0);
	imlib_image_draw_pixel(i,j+2,0);
      }
    }
  }
}

static void draw_throughput(App *app) {
  static int tpstep = 0, tpw, tph;
  static char tpmsg[20];
  static int lw = -1, lh = -1, lx = -1, ly = -1;
  static int reshape_cnt = 0;
  if (Prefs.popup_throughput_pos == AL_NONE) return;
  
  if (!app->bigfont) return;
  imlib_context_set_font(app->bigfont);
  /* get dimensions (only once) */
  if (lx == -1 || app->reshape_cnt != reshape_cnt) {    
    imlib_get_text_size("00.0M/s",&lw,&lh);
    if (lw > (int)(app->dock->w*3/4)) { lw = app->dock->w; }
    sethw(app,lw,lh,Prefs.popup_throughput_pos,&lx,&ly,&lw,&lh);
    reshape_cnt = app->reshape_cnt;
  }
  
  if (get_read_mean_throughput() + get_write_mean_throughput() > Prefs.popup_throughput_threshold) {
    tpstep = MIN(tpstep+1,4);
    snprintf(tpmsg,sizeof tpmsg, "%.1fM/s",get_read_mean_throughput() + get_write_mean_throughput());
    imlib_get_text_size(tpmsg,&tpw,&tph);
    if (tpw > lw) { 
      snprintf(tpmsg,sizeof tpmsg, "%.1fM",get_read_mean_throughput() + get_write_mean_throughput());
      imlib_get_text_size(tpmsg,&tpw,&tph);
    }
  } else if (tpstep) tpstep--;
  if (tpstep) {
    imlib_context_set_color(128, 128, 128, tpstep*30);
    imlib_image_draw_rectangle(lx-1,ly-1,lw+2,lh+2);
    imlib_context_set_color(128, 128, 128, 10+tpstep*25);
    imlib_image_fill_rectangle(lx,ly,lw,lh);
    imlib_context_set_color(255, 255, 255, 50+tpstep*50);
    my_imlib_text_draw(lx + (lw - tpw)/2, ly, tpmsg);
  }
}

static void draw(App *app) {
  DATA32 *buff = imlib_image_get_data();

  if (!Prefs.disable_io_matrix) {
    evolve_io_matrix(app,buff);
    //draw_io_matrix(app,buff);
  } else memset(buff, 0, sizeof(DATA32)*app->dock->w*app->dock->h);
  imlib_image_put_back_data(buff);
  draw_hdlist(app);
  if (!Prefs.disable_swap_matrix) draw_swap_matrix(app);
  draw_throughput(app);
}

void reshape(int w, int h) {
  DockImlib2 *dock = app->dock;
  static int isinit = 0;
  dock->w = w; dock->h = h;
  dock->win_width = dock->w + dock->x0;
  dock->win_height = dock->h + dock->y0;
  app->reshape_cnt++;
  app->sm.w = 6;
  app->sm.nrow = (dock->w-1) / app->sm.w;
  app->sm.ncol = (dock->h-1) / app->sm.w;
  if (isinit) FREE_ARR(app->sm.pre_cnt);
  ALLOC_ARR(app->sm.pre_cnt, app->sm.nrow, app->sm.ncol);
  if (isinit) FREE_ARR(app->sm.intensity)
  ALLOC_ARR(app->sm.intensity, app->sm.nrow, app->sm.ncol);
  app->iom.w = dock->w; app->iom.h = dock->h;
  if (isinit) FREE_ARR((void*)app->iom.v);
  ALLOC_ARR(app->iom.v,app->iom.h+4, app->iom.w+2);
  if (isinit) { dockimlib2_reset_imlib(dock); }

  isinit = 1;
}

#ifndef GKRELLM
static void event_loop(App *app) {
  int tic_cnt = 0;
  while (1) {
    XEvent ev;
    tic_cnt++;
    if (tic_cnt % 5 == 0) {
      XWindowAttributes attr;
      if (app->dock->normalwin) {
        XGetWindowAttributes(app->dock->display, app->dock->normalwin, &attr);
        app->dock->normalwin_mapped = (attr.map_state == IsViewable);
      }
      if (app->dock->iconwin) {
        XGetWindowAttributes(app->dock->display, app->dock->iconwin, &attr);
        app->dock->iconwin_mapped = (attr.map_state == IsViewable);
      }
    }

    while (XPending(app->dock->display)) {
      XNextEvent(app->dock->display, &ev);
      switch (ev.type) {
      case ClientMessage:
        if (ev.xclient.message_type == app->dock->atom_WM_PROTOCOLS
            && ev.xclient.format == 32 
            && (Atom)ev.xclient.data.l[0] == app->dock->atom_WM_DELETE_WINDOW) {
          exit(0);
        } 
        break;
      case ButtonRelease:
        //exit(0);
        if (ev.xbutton.button == Button4) prev_displayed_hd(-1);
        else if (ev.xbutton.button == Button5 || ev.xbutton.button == Button1) next_displayed_hd(+1);
        break;
      case ConfigureNotify: {
        if (app->dock->iconwin == None && 
            (ev.xconfigure.width != (int)app->dock->win_width || 
             ev.xconfigure.height != (int)app->dock->win_height)) {
          app->dock->w = app->dock->win_width = ev.xconfigure.width;
          app->dock->h = app->dock->win_height = ev.xconfigure.height;
          reshape(ev.xconfigure.width, ev.xconfigure.height); //app->dock->w, app->dock->h, None);
          /*printf("ConfigureNotify : %dx%d %dx%d\n", 
                 ev.xconfigure.width, ev.xconfigure.height, 
                 app->sm.nrow, app->sm.ncol);*/
        }
      } break;
      }
    } 
    if (tic_cnt % app->update_stats_mult == 0) {
      update_stats();
      if (!Prefs.disable_io_matrix)   update_io_matrix(app);
      if (!Prefs.disable_swap_matrix) update_swap_matrix(app);
    }
    if (tic_cnt % 100 == 5) {
#    ifdef ENABLE_HDDTEMP_QUERY
      if (Prefs.enable_hddtemp) {
        query_hddtemp(app);
      }
#    endif
#    ifdef ENABLE_POWER_QUERY
      if (Prefs.enable_power_status) {
        DiskList *dl; int cnt;
        for (cnt = 0, dl = first_hd_in_list(); dl; dl = next_hd_in_list(dl), ++cnt) {
          if (dl->enable_power_status && !dl->is_scsi) {
            char devname[512]; snprintf(devname, 512, "/dev/%s", dl->name);
            app->disk_power_mode[cnt] = query_power_mode(devname);
          }
        }
      }
#    endif
    }
    //if (tic_cnt > 500) exit(1);
    usleep(app->update_display_delay_ms * 1000);
    draw(app);
    dockimlib2_render(app->dock);
  }
}
#endif /* ndef GKRELLM */

void setup_cmap(cmap *m) {
  struct {
    float x0; 
    DATA32 c;
  } colors0[] = {{-128, 0xff8080},
                 {- 70, 0xF00000},
                 { -60, 0xDf0080},
                 { -20, 0x800000},
                 {   0, 0x000000},
                 {  10, 0x008000},
                 {  60, 0xf09000},
                 {  90, 0xffa000},
                 { 116, 0xffd000},
                 { 127, 0xffff00}},
    colors1[] = {{-128, 0xff0000},
                 {- 64, 0x808080},
                 { 0, 0x404040},
                 //{  , 0x000000},
                 //{  0 , 0x000000},
                 {  1, 0x208020},
                 {  64, 0x509050},
                 {+ 90, 0x60C060},
                 {+127, 0x008000}},
      colors2[] = {{-128, 0x400000},
                   { -60, 0xA00000},
                   { -34, 0xff0000},
                   { -16, 0x400000},
                   {   0, 0x000000},
                   {  16, 0x000040},
                   {  34, 0x0000ff},
                   {  60, 0x0000A0},
                   {+128, 0x000040}}, 
      colors3[] = {{-128, 0x500060},
                   { -60, 0x500050},
                   { -34, 0x000000},
                   {   0, 0x000000},
                   {  34, 0x000000},
                   {  60, 0x206020},
                   {+128, 0x205020}},
      colors4[] = {{-128, 0x5000F0},
                   { -70, 0x0000C0},
                   { -50, 0x0000A0},
                   { -40, 0x707090},
                   { -30, 0x000080},
                   { -20, 0x505070},
                   { -10, 0x000060},
                   {   0, 0x000000},
                   {  10, 0x006000},
                   {  20, 0x707070},
                   {  30, 0x008000},
                   {  40, 0x909090},
                   {  50, 0x00A000},
                   {  70, 0x00C000},
                   {+128, 0x20D020}},

    *cdef = NULL;

#define SELMAP(n) \
if (Prefs.iomatrix_colormap == n) { sz = sizeof(colors##n)/sizeof(*colors0); cdef = colors##n; }

  unsigned i, sz=0;
  SELMAP(0); SELMAP(1); SELMAP(2); SELMAP(3); SELMAP(4);
  float x0 = cdef[0].x0, x1 = cdef[sz-1].x0;
  for (i = 0; i < sz - 1; ++i) {
    int i0 = (int)((cdef[i].x0-x0)*CMAPSZ/(x1-x0));
    int i1 = (int)((cdef[i+1].x0-x0)*CMAPSZ/(x1-x0));
    int r1 = cdef[i].c>>16 & 0xff, r2 = cdef[i+1].c>>16 & 0xff;
    int g1 = cdef[i].c>> 8 & 0xff, g2 = cdef[i+1].c>> 8 & 0xff;
    int b1 = cdef[i].c     & 0xff, b2 = cdef[i+1].c     & 0xff;
    int j;
    for (j=i0; j <= MIN(i1,CMAPSZ-1); ++j) {
      float alpha = (j-i0+.5)/(float)(i1-i0);
      m->p[j] = 
        (MIN((int)(r1*(1-alpha) + alpha*r2),255)<<16) +
        (MIN((int)(g1*(1-alpha) + alpha*g2),255)<<8) +
        (MIN((int)(b1*(1-alpha) + alpha*b2),255));
      //printf("cmap[%d] = 0x%06x\n", j, m->p[j]);
    }
  }
}

unsigned getpos(const char *pp) {
  char p[2];
  unsigned v = AL_NONE, i;  
  if (!pp || !pp[0]) return AL_NONE;
  if (strlen(pp) > 2) { fprintf(stderr, "invalid position specification: '%s'\n", pp); exit(1); }
  strncpy(p,pp,2);
  if (p[0] == 'c') { char tmp = p[0]; p[0] = p[1]; p[1] = tmp; }
  for (i=0; i < 2 && p[i]; ++i) {
    if (p[i] == 'r') { v |= AL_RIGHT; }
    else if (p[i] == 'l') { v |= AL_LEFT; }
    else if (p[i] == 't') { v |= AL_TOP; }
    else if (p[i] == 'b') { v |= AL_BOTTOM; }
    else if (p[i] == 'c') {
      if (v & (AL_LEFT | AL_RIGHT | AL_HCENTER)) v |= AL_VCENTER; else v |= AL_HCENTER; 
    } else {
      fprintf(stderr, "unknown position specifier: '%c'\n", p[i]); exit(1);
    }
  }
  return v;
}

void init_prefs(int argc, char **argv) {
#ifndef GKRELLM
  /* Prefs already read from the gkrellm config file: */
  Prefs.disable_swap_matrix = 0;
  Prefs.disable_io_matrix = 0;
  Prefs.disable_hd_leds = 0;
  Prefs.iomatrix_colormap = 0;
  Prefs.popup_throughput_threshold = 0.5; /* MB/s */
  Prefs.hdlist_pos = AL_BOTTOM + AL_LEFT;
  Prefs.enable_hddtemp = 0;
  Prefs.bigfontname = Prefs.smallfontname = NULL;
#endif
  Prefs.xprefs.dockapp_size = 64;
  Prefs.verbosity = 0;
  Prefs.hddtemp_port = 7634;
  Prefs.enable_power_status = 0;
  Prefs.debug_swapio = 0; Prefs.debug_disk_wr = Prefs.debug_disk_rd = 0;
  Prefs.popup_throughput_pos = AL_TOP;
  Prefs.xprefs.argc = argc; Prefs.xprefs.argv = argv;
  Prefs.xprefs.flags = 0;
  Prefs.temperatures_unit = 'C';
}

#ifndef GKRELLM
void parse_options(int argc, char **argv) {
  int d_opt_used = 0;
  enum { OPT_DISPLAY=1, OPT_SMALLFONT, OPT_BIGFONT, OPT_FONTPATH, 
         OPT_NOSWAP, OPT_NOIO, OPT_NOLEDS, OPT_HDLIST, OPT_THROUGHPUT, OPT_32, OPT_56, OPT_48 };
  static struct option long_options[] = {
    {"help",0,0,'h'},
    {"verbose",0,0,'v'},
    {"version",0,0,'V'},
    {"hddtemp",0,0,'t'},
    {"farenheit",0,0,'F'},
    {"display",1,0,OPT_DISPLAY},
    {"geometry",2,0,'g'},
    {"smallfont",1,0,OPT_SMALLFONT},
    {"bigfont",1,0,OPT_BIGFONT},
    {"fontpath",1,0,OPT_FONTPATH},
    {"colormap",1,0,'c'},
    {"noswap",0,0,OPT_NOSWAP},
    {"noio",0,0,OPT_NOIO},
    {"noleds",0,0,OPT_NOLEDS},
    {"hdlist",2,0,OPT_HDLIST},
    {"throughput",2,0,OPT_THROUGHPUT},
    {"32",0,0,OPT_32},
    {"48",0,0,OPT_48},
    {"56",0,0,OPT_56},
    {NULL,0,0,0}
  };
  int long_options_index;
  const char *help = 
    "wmhdplop " VERSION " - monitor hard-drive (or partition) activity\n"
    "A recent kernel is required (>2.4.20)\n"
    "Usage: wmhdplop [options]\n"
    "Option list:\n"
    "  -h, --help      print this.\n"
    "  -v, --verbose   increase verbosity\n"
    "  -V, --version   print version\n"
    "  -t[=port], --hddtemp[=port]\n"
    "                  display hd temperatures, in Celsius degrees (requires hddtemp daemon running)\n"
    "  -F, --farenheit display hd temperatures in Farenheit degrees\n"
    /*    "  -p : monitor harddrive power status (idle, standby or sleeping, as reported by hdparm -C)\n"
          "       note that this options requires that wmhd is suid..\n"*/ /* hddtemp already reports when a drive is asleep ... */
    "  --noswap        disable the animation reflecting swap activity\n"
    "  --noio          disable background animation reflecting disk activity\n"
    "  --noleds        hide the small led indicating disk activity\n"
    "  --fontpath path add a new directory to the font search directory list\n"
    "                  default: --fontpath=/usr/share/fonts/truetype (and subdirectories)\n"
    "                           --fontpath=/usr/share/fonts/ttf      (and subdirectories)\n"
    "                           --fontpath=$HOME/.fonts              (and subdirectories)\n"
    "  --smallfont  fontname/size\n"
    "  --bigfont    fontname/size\n"
    "                  Set the 'small font' and the 'big font' name/size in pixel\n"
    "                  (default: --smallfont=Vera/7 --bigfont=Arial_Black/10)\n"
    "                  The font name are case-sensitive, and must correspound to the name\n"
    "                  of a .ttf file which can be found in one of the fontpaths\n"
    "                  By default, wmhdplop tries to load the following fonts:\n"
    "                    * Arial_Black/10, FreeSansBold/11, VeraMoBd/9\n"
    "                    * Vera/7, FreeSans/7, Trebuchet_MS/7\n"
    "  -c n, --colormap=n\n"
    "                  select colormap number n\n" 
    "  --hdlist[=pos]  hide completely the list of drives with leds and temperatures (if no position given)\n"
    "                  or change its location (default: 'B'). Possible positions are: 't','b','l','bl','bc','br',...\n"
    "                  (t=top, b=bottom, l=left, r=right, c=centered, etc)\n"
    "  --throughput=mbs[,pos]\n"
    "                  minimum io throughput (MB/s) that will be displayed in the popup display (default 0.5)\n"
    "                  and position of the popup (same format than hdlist)\n"
    "  -g[=WxH+x+y], --geometry[=WxH+x+y]\n"
    "                  start in window (i.e. undocked) mode with specified geometry (i.e -g 96x32 or -g 64x64+0+0)\n"
    "  --32,  --48,  --56\n"
    "                  start in a reduced dockapp, for people whose dock is too small too contain 64x64 dockapps\n"
    "  -d device[:name] monitor activity on the specified hard-drive. This option can be\n"
    "                  used many times. If you do not use it, all hard-drives will be monitored.\n"
    "                  If an optional 'name' is given, it will be displayed instead of the /dev/hdx name\n"
    "                  The device may also be a single partition.\n";
  init_prefs(argc, argv);
  while (1) {
    int c;
    c = getopt_long(argc, argv, "g::hvVt::Fc:LP:d:@:",long_options,&long_options_index); /* -g option is handled in dockapp_imlib2 */
    if (c == -1)
      break;
    switch (c) {
      case ':':
      case '?': 
        exit(1);
      case OPT_DISPLAY:
        Prefs.xprefs.flags |= DOCKPREF_DISPLAY; Prefs.xprefs.display = strdup(optarg);
        break;
      case 'g':
        Prefs.xprefs.flags |= DOCKPREF_GEOMETRY; if (optarg) Prefs.xprefs.geometry = strdup(optarg);
        break;
      case 'h':
        puts(help); exit(0);
      case 'v':
        Prefs.verbosity++;
        break;
      case 'V':
        printf("wmhdplop %s\n",VERSION); exit(0);
        break;
      case 't':
        Prefs.enable_hddtemp = 1;
        if (optarg) Prefs.hddtemp_port = atoi(optarg);
        break;
        /*case 'p':
          Prefs.enable_power_status = 1;
          break;*/
      case 'F':
        Prefs.enable_hddtemp = 1;
	Prefs.temperatures_unit = 'F';
	break;
      case OPT_NOIO:
        Prefs.disable_io_matrix = 1;
        break;
      case OPT_NOSWAP:
        Prefs.disable_swap_matrix = 1;
        break;
      case OPT_NOLEDS:
        Prefs.disable_hd_leds = 1;
        break;
      case OPT_FONTPATH:
        printf("add font path: %s\n", optarg);
        imlib_add_path_to_font_path(optarg);
        break;
      case OPT_SMALLFONT:
        Prefs.smallfontname = strdup(optarg);
        break;
      case OPT_BIGFONT:
        Prefs.bigfontname = strdup(optarg);
        break;
      case OPT_HDLIST:
        Prefs.hdlist_pos = getpos(optarg);
        break;
      case 'c':
        Prefs.iomatrix_colormap = atoi(optarg);
        if (Prefs.iomatrix_colormap > 4) { fprintf(stderr,"invalid colormap number\n"); exit(1); }
        break;
      case OPT_THROUGHPUT:
        if (optarg) {
          Prefs.popup_throughput_threshold = atof(optarg);
          if (strchr(optarg,',')) Prefs.popup_throughput_pos = getpos(strchr(optarg,',')+1);
        } break;
      case OPT_56:
        Prefs.xprefs.dockapp_size = 56;
        break;
      case OPT_48:
        Prefs.xprefs.dockapp_size = 48;
        break;
      case OPT_32:
        Prefs.xprefs.dockapp_size = 32;
        break;
      case 'd':
        if (optarg) {
          char *p = strchr(optarg, ':');
          assert(optarg);
          if (p) *p++ = 0;
          BLAHBLAH(1,printf("adding device %s to monitored disc list\n",optarg));
          if (add_device_by_name(optarg, p) != 0) 
            fprintf(stderr, "Warning: device %s not found or not recognized -- try option -v to get additionnal information\n", optarg);
          d_opt_used = 1;
        } break;
      case '@':
        sscanf(optarg,"%d,%d,%d",&Prefs.debug_swapio,&Prefs.debug_disk_rd,&Prefs.debug_disk_wr);
        break;
      default:
        assert(0);
    }
  }
  if (optind != argc) {
    fprintf(stderr, "unknown option: %s\n", argv[optind]); exit(1);
  }
  if (!d_opt_used) scan_all_hd(1);
}
#endif

char *xstrdup(const char *s) {
  if (s) return strdup(s);
  else return NULL;
}

void init_fonts(App *app) {
  char *bigfontlist[] = {"Arial_Black/10", "luxisb/11", "VeraMoBd/9", "arialbd/12", "Vera/9", "Verdana_Bold/10", "VerdanaBd/10", "Verdana/10", "FreeSansBold/11", NULL};
  char *smallfontlist[] = {"Vera/7","Trebuchet_MS/7", "luxisr/7", "Verdana/7","Arial/7","FreeSans/7", NULL};
  if (app->bigfont) {
    imlib_context_set_font(app->bigfont); imlib_free_font(); app->bigfont = NULL;
  }
  if (app->smallfont) {
    imlib_context_set_font(app->smallfont); imlib_free_font(); app->smallfont = NULL;
  }
  app->bigfont = load_font(Prefs.bigfontname, bigfontlist);
  if (app->bigfont) 
    app->current_bigfont_name = strdup(dockimlib2_last_loaded_font());
  app->smallfont = load_font(Prefs.smallfontname, smallfontlist);
  if (app->smallfont) 
    app->current_smallfont_name = strdup(dockimlib2_last_loaded_font());
}

#ifndef GKRELLM
int main(int argc, char**argv)
#else
int hdplop_main(int width, int height, GdkDrawable *gkdrawable)
#endif
{
  euid = geteuid(); uid = getuid(); seteuid(uid);
  ALLOC_OBJ(app);
  srand(time(NULL));
  /* Initialize options */
#ifndef GKRELLM
  parse_options(argc,argv);
#else
  init_prefs(0, NULL);
  scan_all_hd(1);
#endif
  /* Initialize imlib2 */
#ifndef GKRELLM
  app->dock = dockimlib2_setup(2, 2, Prefs.xprefs.dockapp_size-5, Prefs.xprefs.dockapp_size-5, &Prefs.xprefs);
#else
  app->dock = dockimlib2_gkrellm_setup(0, 0, width, height, &Prefs.xprefs, gkdrawable);
#endif
  app->bigfont = app->smallfont = NULL; 
  app->current_bigfont_name = app->current_smallfont_name = NULL;
  app->reshape_cnt = 0;
  if (find_id(-1,0) == 0) {
    app->filter_hd = -1; app->filter_part = -1; /* only partitions */
  } else {
    app->filter_hd = -1; app->filter_part = 0;
  }
  app->displayed_hd_changed = 1;
  if (nb_dev_in_list() == 0) {
#ifndef GKRELLM
    fprintf(stderr, "No common hard-drives found in /etc/mtab and /proc/partitions..\n"
            "Please use option -d to add devices (i.e. %s -d /dev/hda -d /dev/hdb ...)\n", argv[0]);
    exit(1);
#else
    fprintf(stderr, "No hard drive found...\n");
    /* Euh, on fait quoi maintenant ? exit ? */
#endif
  }
  init_fonts(app);
  app->update_display_delay_ms = 50;
  app->update_stats_mult = 2;

  app->swap_matrix_lighting = (int)(300/app->update_display_delay_ms);
  app->swap_matrix_luminosity = 255;

  app->nb_hd = nb_hd_in_list();
  app->nb_dev = nb_dev_in_list();
  ALLOC_VEC(app->disk_power_mode, app->nb_hd); SET_VEC(app->disk_power_mode, HD_ACTIVE,0,app->nb_hd);
  ALLOC_VEC(app->disk_temperature, app->nb_hd); SET_VEC(app->disk_temperature, -1, 0, app->nb_hd);

  init_stats(app->update_display_delay_ms*1e-3*app->update_stats_mult);
  BLAHBLAH(1, {
      DiskList *dl = first_dev_in_list();
      for ( ; dl; dl=dl->next) {
        printf("Monitored: %s (%s) major=%d, minor=%d is_partition=%d\n", 
               dl->dev_path, dl->name, dl->major, dl->minor, is_partition(dl->major,dl->minor));
      }
    });

  reshape(app->dock->w,app->dock->h);
  app->iom.ops = NULL;
  setup_cmap(&app->iom.cm);
#ifndef GKRELLM
  event_loop(app);
#endif
  return 0;
}

#ifdef GKRELLM
void gkrellm_hdplop_update(int update_options) {
  static int tic_cnt = 0;
  if (update_options) {
    setup_cmap(&app->iom.cm);
    if (!Prefs.enable_hddtemp)
      SET_VEC(app->disk_temperature, -1, 0, app->nb_hd);
  }
  // update the data:

  if (tic_cnt % app->update_stats_mult == 0) {
    update_stats();
    if (!Prefs.disable_io_matrix)
      update_io_matrix(app);
    if (!Prefs.disable_swap_matrix)
      update_swap_matrix(app);
  }
  if (tic_cnt % 100 == 5 && Prefs.enable_hddtemp) {
    query_hddtemp(app);
  }

  // draw the Imlib2 pixmap:
  draw(app);
  dockimlib2_render(app->dock);
  ++tic_cnt;
}
#endif
