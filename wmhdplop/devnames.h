#ifndef DEVNAMES_H
#define DEVNAMES_H
#include "util.h"

#define IS_PARTITION(DL) ((DL)->part_id!=0)

typedef struct DiskList {
  char *name, *dev_path;
  unsigned major, minor;
  int hd_id, part_id; /* part_id = 0 for disks */
  int enable_hddtemp;
  long nr, nw, touched_r, touched_w;
  struct DiskList *next;
} DiskList;

int is_displayed(int hd_id, int part_id); /* wmhdplop.c function */
int short_name_for_device(unsigned major, unsigned minor, char *name);

const char *stripdev(const char *s);
DiskList *find_dev(unsigned major, unsigned minor);
DiskList *find_dev_by_name(const char *name);
DiskList *find_id(int hd_id, int part_id);
DiskList *next_hd_in_list(DiskList *current);
DiskList *first_hd_in_list();
DiskList *first_dev_in_list();
int nb_hd_in_list();
int nb_dev_in_list();
int is_partition(unsigned major, unsigned minor);
int add_device_by_name(const char *devname, const char *mtab_name);
int add_device_by_id(unsigned major, unsigned minor, const char *mtab_name);
void add_swap(const char *swapdev);
int device_info(unsigned major, unsigned minor, char *name, int *hd_id, int *part_id);
strlist *swap_list();
#endif
