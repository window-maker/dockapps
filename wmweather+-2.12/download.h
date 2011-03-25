#ifndef DOWNLOAD_H
#define DOWNLOAD_H

/* flags for download_file */
#define DOWNLOAD_NO_404              1
#define DOWNLOAD_KILL_OTHER_REQUESTS 2

void download_init(char *email);
void download_process(unsigned long sleep_time);
int download_kill(char *filename);
int download_file(char *filename, char *from_addr, char *postdata, int flags, void (*callback)(char *filename, void *data), void *data);

#endif
