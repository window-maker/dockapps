#ifndef __MISC_H
#define __MISC_H

#include <unistd.h>

extern void parse_command(char *, char ***, int *);

extern pid_t execCommand(char *);
#endif /* __MISC_H */
