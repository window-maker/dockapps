#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "global.h"
#include "procstat.h"
#include "devnames.h"

static ProcStats ps;
int use_proc_diskstats;

void pstat_init(struct pstat *pst, int nslice, float update_interval) {
  pst->nslice = nslice;
  ALLOC_VEC(pst->slices, nslice);
  pst->cur_slice = 0;
  pst->total = 0;
  pst->update_interval = update_interval;
}

float pstat_current(struct pstat *pst) {
  int idx = pst->cur_slice ? pst->cur_slice-1 : pst->nslice-1;
  return pst->slices[idx]/pst->update_interval;
}

void pstat_add(struct pstat *pst, unsigned long v) {
  pst->slices[pst->cur_slice] += v;
}

void pstat_advance(struct pstat *pst) {
  unsigned long v = pst->slices[pst->cur_slice];
  if (pst->total)
    pst->slices[pst->cur_slice] -= pst->total; 
  else pst->slices[pst->cur_slice] = 0;
  pst->total = v;
  if (++pst->cur_slice >= pst->nslice) pst->cur_slice = 0;
  pst->slices[pst->cur_slice] = 0;
}

float pstat_meanval(struct pstat *pst) {
  unsigned long sum = 0;
  int i;
  for (i = 0; i < pst->nslice; ++i) sum += pst->slices[i];
  return sum / pst->update_interval / (pst->nslice-1);
}

float get_read_throughput() {
  return pstat_current(&ps.disk_read) / 2048.; /* 2048 because we log sector counts */
}

float get_write_throughput() {
  return pstat_current(&ps.disk_write) / 2048.;
}

float get_swapin_throughput() {
  return pstat_current(&ps.swap_in) / 2048.;
}

float get_swapout_throughput() {
  return pstat_current(&ps.swap_out) / 2048.;
}

float get_read_mean_throughput() {
  return pstat_meanval(&ps.disk_read) / 2048.;
}

float get_write_mean_throughput() {
  return pstat_meanval(&ps.disk_write) / 2048.;
}

void update_stats() {
  FILE *f;
  char line[1024];
  char hdname[200];
  int readok = 0, in_our_disk = 0;
  const char *proc_fname;
  if (!use_proc_diskstats) proc_fname = "/proc/partitions";
  else proc_fname = "/proc/diskstats";

  f = fopen(proc_fname, "r");
  if (!f) { perror(proc_fname); return; }
  while (fgets(line, sizeof(line), f)) {
    int major, minor;
    unsigned long nr, nw;
    const char *fmt = use_proc_diskstats ? 
      "%d %d %200s %*d %*d %lu %*d %*d %*d %lu" :
      "%d %d %*u %200s %*d %*d %lu %*d %*d %*d %lu";
    if (sscanf(line, fmt, &major, &minor, hdname, &nr, &nw) == 5 ||
        (use_proc_diskstats && is_partition(major,minor) &&
         /* .. kernel 2.5 uses a different format for partitions and disc */
         sscanf(line, "%d %d %200s %*d %lu %*d %lu", &major, &minor, hdname, &nr, &nw) == 5)) {
      DiskList *dl;
      if (readok == 0) readok = 1;
      if ((dl=find_dev(major,minor))) {
        dl->touched_r = (dl->nr - nr) ? 10 : dl->touched_r; 
        dl->touched_w = (dl->nw - nw) ? 10 : dl->touched_w; 
        dl->nr = nr; dl->nw = nw;
        if (!is_partition(major,minor)) in_our_disk = 1;
        if (is_displayed(dl->hd_id,dl->part_id) && 
            ((dl->part_id && (!find_id(dl->hd_id, 0) || !is_displayed(dl->hd_id, 0))) || /* partition without host disk */
             (dl->part_id == 0))) /* disk */
          {
            if (!Prefs.debug_disk_rd) {
              pstat_add(&ps.disk_read, nr);
            } else {
              static int cntr = 0; cntr+=(rand()%30) == 0 ? Prefs.debug_disk_rd : 0;
              pstat_add(&ps.disk_read, nr + cntr);
            }
            if (!Prefs.debug_disk_wr) {
              pstat_add(&ps.disk_write, nw);
            } else {
              static int cntw = 0; cntw+=(rand()%30) == 0 ? Prefs.debug_disk_wr : 0;
              pstat_add(&ps.disk_write, nw + cntw);
            }
            readok = 2;
          }
      } else if (!is_partition(major,minor)) in_our_disk = 0;
      /*      if (in_our_disk || find_dev(major,minor))*/ {
        strlist *sl;
        for (sl = swap_list(); sl; sl=sl->next) {
          if (strcmp(stripdev(hdname), stripdev(sl->s)) == 0) {
            if (!Prefs.debug_swapio) {
              pstat_add(&ps.swap_in, nr);
              pstat_add(&ps.swap_out, nw);
            } else {
              static int cnt = 0; cnt+=Prefs.debug_swapio;
              pstat_add(&ps.swap_in, nr + cnt);
              pstat_add(&ps.swap_out, nw + cnt);
            }
          }
        }
      }
    }
  }
  fclose(f);
  pstat_advance(&ps.disk_read);pstat_advance(&ps.disk_write);
  pstat_advance(&ps.swap_in);pstat_advance(&ps.swap_out);
  if (readok == 0) { fprintf(stderr, "warning: could not find any info in %s (kernel too old?)\n", proc_fname); exit(1); }
  else if (readok == 1) { ONLY_ONCE(fprintf(stderr, "warning: could not find any monitored disc in %s\n", proc_fname)); }
  BLAHBLAH(1,printf("swap: %5.2f,%5.2f disc: %5.2f,%5.2f MB/s\n", 
                    get_swapin_throughput(), get_swapout_throughput(), 
                    get_read_throughput(), get_write_throughput()));
}

void init_stats(float update_interval) {
  char s[512];
  FILE *f;

  pstat_init(&ps.swap_in, (int)(0.5/update_interval)+1, update_interval);
  pstat_init(&ps.swap_out, (int)(0.5/update_interval)+1, update_interval);
  pstat_init(&ps.disk_read, (int)(0.5/update_interval)+1, update_interval);
  pstat_init(&ps.disk_write, (int)(0.5/update_interval)+1, update_interval);
  f = fopen("/proc/swaps","r");
  //if (!f) { perror("/proc/swaps"); exit(1); }
  if (f) {
    while (fgets(s, 512, f)) {
      char *s2 = strchr(s,' ');
      if (s2 && s2 != s && strncmp(s, "/dev/", 5) == 0) {
        *s2 = 0;
        add_swap(s);
        BLAHBLAH(1,printf("found swap partition: %s\n", swap_list()->s));
      }
    }
    fclose(f);
  }
  if (!swap_list()) {
    fprintf(stderr, "Warning: no swap partition found in /proc/swaps !!\n");
  }
  if ((f=fopen("/proc/diskstats","r"))) { 
    use_proc_diskstats = 1; fclose(f); 
  }
  else { use_proc_diskstats = 0; }
  BLAHBLAH(1,printf("using %s for disc statistics\n", use_proc_diskstats ? "/proc/diskstats" : "/proc/partitions"));
}



/* hello I am brain-dead */
void scan_all_hd(int add_partitions) {
  char s[512];
  FILE *f = NULL;
  /* find mounted partitions */
  if (add_partitions && (f = fopen("/etc/mtab","r"))) {
    while (fgets(s,512,f)) {
      char partname[512], mountpoint[512];
      mountpoint[0] = 0;
      if (sscanf(s,"%500s %500s", partname, mountpoint)>=1) {
        char *p = strchr(mountpoint, '/');
        //printf("add_device_by_name(%s, %s)\n", partname, p);
        add_device_by_name(partname, p);
      }
    }
  }
  if (f) fclose(f);

  /* second try .. look for hard drives listed in proc/partitions (and only hard-drives .. their partitions
   are supposed to be listed in mtab) */
  if ((f = fopen("/proc/partitions","r"))) {
    while (fgets(s,512,f)) {
      unsigned major, minor;
      char name[512]; name[0] = 0;
      if (sscanf(s,"%d %d %*d %500s", &major, &minor, name)==3) {        
        int hd_id, part_id;
        /* on n'ajoute que les disques dont au moins une partoche etait dans mtab */
        if (device_info(major,minor,NULL,&hd_id,&part_id) != 0 && part_id == 0 && find_id(hd_id,-1)) {
          add_device_by_id(major,minor,NULL);
        }
      }
    }
  }

  if (f) fclose(f);
  /*  for (c = 'a'; c <= 'z'; ++c) {
    snprintf(s,512,"/proc/ide/hd%c/media",c);
    if ((f = fopen(s,"r")) && fgets(s, 512, f) && strcmp(s, "disk\n")==0) {
      snprintf(s, 512, "/dev/hd%c",c);
      add_device(s, NULL);
    }
    if (f) fclose(f);
    }*/
}
