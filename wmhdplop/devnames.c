#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/major.h>
#include <ctype.h>
#include "global.h"
#include "devnames.h"

static DiskList *dlist = NULL;

const char *stripdev(const char *s) {
  if (strncmp(s,"/dev",4) == 0) s+= 4;
  if (s[0] == '/') s++;
  return s;
}

DiskList *find_dev(unsigned major, unsigned minor) {
  DiskList *dl;
  for (dl = dlist; dl; dl = dl->next) {
    if (dl->major == major && dl->minor == minor) return dl;
  }
  return NULL;
}

DiskList *find_id(int hd_id, int part_id) {
  DiskList *dl;
  for (dl = dlist; dl; dl = dl->next) {
    if (((dl->hd_id == hd_id) || hd_id == -1) && 
        ((dl->part_id == part_id) || part_id == -1)) return dl;
  }
  return NULL;
}
/*
DiskList *find_dev_by_name(const char *name) {
  DiskList *dl;
  const char *sname = stripdev(name);
  for (dl = dlist; dl; dl = dl->next) {
    if (strcmp(sname, dl->name)==0) return dl;
  }
  return NULL;
  }*/

DiskList *next_hd_in_list(DiskList *prev) {
  DiskList *dl = (prev ? prev->next : dlist);
  while (dl && IS_PARTITION(dl)) dl = dl->next;
  return dl;
}

DiskList *first_hd_in_list() {
  return next_hd_in_list(NULL);
}

DiskList *first_dev_in_list() {
  return dlist;
}

int nb_hd_in_list() {
  int i=0;
  DiskList *dl;
  for (dl = first_hd_in_list(); dl; dl = next_hd_in_list(dl)) ++i;
  return i;
}

int nb_dev_in_list() {
  int i=0;
  DiskList *dl;
  for (dl = dlist; dl; dl = dl->next) ++i;
  return i;
}

/*int is_partition_name(const char *name) {
  int l = strlen(name);
  if (l && name[l-1] >= '0' && name[l-1] <= '9') return 1;
  return 0;
}
*/

static const char *to_num_(unsigned minor) {
  static char id[16];
  if (minor == 0) return "";
  else snprintf(id,16,"%d",minor);
  return id;
}

int device_info(unsigned major, unsigned minor, char *name, int *hd_id, int *part_id) {
  switch(major) {
#ifdef SCSI_DISK0_MAJOR
    case (SCSI_DISK0_MAJOR): 
#else
    case (SCSI_DISK_MAJOR): 
#endif
      if (name) sprintf(name, "sd%c%s", "abcdefghijklmnop"[(minor)/16], to_num_(minor%16)); 
      if (hd_id) *hd_id = (minor)/16;
      if (part_id) *part_id = minor%16;
      return 1;
    case IDE0_MAJOR: 
      if (name) sprintf(name, "hd%c%s", "ab"[(minor)/64], to_num_(minor%64));
      if (hd_id) *hd_id = 100 + (minor)/64;
      if (part_id) *part_id = minor%64;
      return 2;
    case IDE1_MAJOR: 
      if (name) sprintf(name, "hd%c%s", "cd"[(minor)/64], to_num_(minor%64)); 
      if (hd_id) *hd_id = 200 + (minor)/64;
      if (part_id) *part_id = minor%64;
      return 3;
#ifdef IDE2_MAJOR
    case IDE2_MAJOR: 
      if (name) sprintf(name, "hd%c%s", "ef"[(minor)/64], to_num_(minor%64));
      if (hd_id) *hd_id = 300 + (minor)/64;
      if (part_id) *part_id = minor%64;
      return 4;
#endif
#ifdef IDE3_MAJOR
    case IDE3_MAJOR: 
      if (name) sprintf(name, "hd%c%s", "gh"[(minor)/64], to_num_(minor%64));
      if (hd_id) *hd_id = 400 + (minor)/64;
      if (part_id) *part_id = minor%64;      
      return 5;
#endif
#ifdef IDE4_MAJOR
    case IDE4_MAJOR: 
      if (name) sprintf(name, "hd%c%s", "ij"[(minor)/64], to_num_(minor%64));
      if (hd_id) *hd_id = 400 + (minor)/64;
      if (part_id) *part_id = minor%64;      
      return 6;
#endif

#ifdef IDE5_MAJOR
    case IDE5_MAJOR: 
      if (name) sprintf(name, "hd%c%s", "kl"[(minor)/64], to_num_(minor%64));
      if (hd_id) *hd_id = 400 + (minor)/64;
      if (part_id) *part_id = minor%64;      
      return 7;
#endif

#ifdef IDE6_MAJOR
    case IDE6_MAJOR: 
      if (name) sprintf(name, "hd%c%s", "mn"[(minor)/64], to_num_(minor%64));
      if (hd_id) *hd_id = 400 + (minor)/64;
      if (part_id) *part_id = minor%64;      
      return 8;
#endif


#ifdef MD_MAJOR
    case (MD_MAJOR):       
      if (name) sprintf(name, "md%s", to_num_(minor));
      if (hd_id) *hd_id = 400 + minor;
      if (part_id) *part_id = 0;            
      return 9;      
#endif
  }
  if (name) *name = 0;
  if (hd_id) *hd_id = -1;
  if (part_id) *part_id = -1;
  return 0;
}

int short_name_for_device(unsigned major, unsigned minor, char *name) {
  return device_info(major,minor,name,NULL,NULL);
}

int is_partition(unsigned major, unsigned minor) {
  int part_id;
  if (!device_info(major,minor,NULL,NULL,&part_id)) return 0;
  return (part_id != 0);
}

int device_id_from_name(const char *devname__, unsigned *pmajor, unsigned *pminor) {
  if (strlen(devname__)>=512) return -1;
  char *devname_, devname[512];
  
  devname_ = str_substitute(devname__, "/dev/mapper", "/dev");

  struct stat stat_buf;
  BLAHBLAH(1,printf("looking for %s in /dev..\n", devname_));
  
  if (devname_[0] != '/') snprintf(devname,512,"/dev/%s", devname_); 
  else snprintf(devname,512,"%s",devname_);

  free(devname_); devname_ = 0;

  if (lstat(devname,&stat_buf)) { BLAHBLAH(1,perror(devname)); return -1; }
  if (S_ISLNK(stat_buf.st_mode)) {
    devname_ = realpath(devname, NULL);
    if(!devname_) { BLAHBLAH(1,perror(devname)); return -1; }
    strncpy(devname, devname_, 512); devname[511] = 0;
    free(devname_);
    if (stat(devname,&stat_buf)) { BLAHBLAH(1,perror(devname)); return -1; }
  }
  if (!S_ISBLK(stat_buf.st_mode)) {
    fprintf(stderr, "%s is not a block device..\n", devname);
    return -2;
  }
  *pmajor = major(stat_buf.st_rdev);
  *pminor = minor(stat_buf.st_rdev);
  return 0;
}



/* add a hard-drive if it is a regular ide/scsi/md disk/partition, or return NULL */
static DiskList *create_device(unsigned major, unsigned minor, const char *mtab_name) {
  DiskList *dl; 
  char dev_path[512];
  ALLOC_OBJ(dl);
  if (mtab_name && strlen(mtab_name)) dl->name = strdup(mtab_name);
  else {
    char s[512]; short_name_for_device(major,minor,s); dl->name = strdup(s);
  }
  BLAHBLAH(1, printf("create_device(major=%d, minor=%d, mtab_name=%s) : name=%s\n", major,minor,mtab_name,dl->name));
  dl->major = major; dl->minor = minor;
  if (device_info(major,minor,dev_path,&dl->hd_id,&dl->part_id) == 0) { 
    BLAHBLAH(1, printf("(%d,%d) is not a known disc type..\n", major,minor));
    free(dl); return NULL; 
  }
  dl->dev_path = malloc(strlen(dev_path)+6); assert(dl->dev_path); 
  sprintf(dl->dev_path, "/dev/%s", dev_path);
  dl->next = NULL;
  if (dl->part_id == 0)
    dl->enable_hddtemp = 1;
  return dl;
}

/* add a device (after checking that it is a known device type) */
int add_device_by_name(const char *devname, const char *mtab_name) {
  unsigned major, minor;

  BLAHBLAH(1,printf("add_device_by_name(%s,%s)\n", devname, mtab_name));
  if (device_id_from_name(devname, &major, &minor) != 0) return -1;

  return add_device_by_id(major,minor,mtab_name);
}

/* add a device (after checking that it is a known device type) */
int add_device_by_id(unsigned major, unsigned minor, const char *mtab_name) {
  DiskList *dl, *next, *prev;
  BLAHBLAH(1,printf("add_device_by_id(%d,%d,%s)\n", major, minor,mtab_name));
  /* already known ? */
  if (find_dev(major,minor)) return -1;
    /* check for known ide/scsi/md disks and then create */
  dl = create_device(major, minor,  mtab_name); if (!dl) return -1;

  for (next = dlist, prev = NULL; next; next=next->next) {
    if (dl->hd_id > next->hd_id || (dl->hd_id == next->hd_id && dl->part_id > next->part_id)) break;
    prev = next;
  }
  if (prev == NULL) {
    dl->next = dlist; dlist = dl;
  } else {
    dl->next = prev->next; prev->next = dl;
  }
  return 0;
}

strlist *swaplst = NULL;
void add_swap(const char *swapdev) {
  swaplst = strlist_ins(swaplst, swapdev);
}
strlist *swap_list() {
  return swaplst;
}
