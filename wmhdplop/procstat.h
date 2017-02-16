#ifndef PROCSTAT_H
#define PROCSTAT_H
#include "util.h"

struct pstat {
  unsigned long total;
  long nslice, cur_slice;
  unsigned long *slices;
  float update_interval;
};

typedef struct {
  /* counted in sectors */
  struct pstat swap_in, swap_out;
  struct pstat disk_read, disk_write;
} ProcStats;

void pstat_init(struct pstat *pst, long nslice, float update_interval);
float pstat_current(struct pstat *pst);
void pstat_add(struct pstat *pst, unsigned long v);
void pstat_advance(struct pstat *pst);
float pstat_meanval(struct pstat *pst);

/* given in MB/s */
float get_read_throughput();
float get_write_throughput();
float get_swapin_throughput();
float get_swapout_throughput();
float get_read_mean_throughput();
float get_write_mean_throughput();

void update_stats();
void init_stats(float update_interval);
void scan_all_hd(int add_partitions);
#endif
