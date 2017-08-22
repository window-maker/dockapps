
#ifndef YAWMPPP_COMMON_H
#define YAWMPPP_COMMON_H

#define MAX_ISPS 40

struct LogStruct {
  time_t start;
  time_t end;
  int    status; /* 0=ok 1=error 2=crash */
  char   longname[128];
  char   shortname[16];
  char   phone[32];
  char   user[32];
};

void clean_guards(void);
void make_guards(void);
void write_log(void);
void warn_pref(void);

void open_ppp(void);
void close_ppp(void);

void write_pid_file(void);
void remove_pid_file(void);
void make_config_dir(void);
void grab_isp_info(int rof);
void run_pref_app(void);
void run_log_app(void);

int get_statistics (char *devname, long *ip, long *op, long *is, long *os);
int stillonline (char *ifs);

#endif

