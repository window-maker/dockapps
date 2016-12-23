/*
**  This file does all of the #include's for the driver.  It also
** sets up some english like functions...
**  Strictly speaking, it's poor style to put functions in a header file.
** The functions in here arn't really functions though...
*/
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>

#define true  1
#define false 0

#define daemon_setup()  chdir("/"); setsid();
#define loop()          while(true)

// Note:  This macro MUST follow an if statement without {}'s!!!
#define freak_out() { \
    printf("Something bad happend near line %i of %s:  %s.\n", \
       __LINE__, __FILE__, strerror(errno) ); exit(1); }

