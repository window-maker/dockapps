#ifndef LINUX_SYSINFO_H
#define LINUX_SYSINFO_H
#include <string.h>
#include <fcntl.h>
#include <sys/utsname.h>


/* These are the possible fscanf formats used in /proc/stat
   reads during history processing.
   ( 5th number only for Linux 2.5.41 and above ) */
#define CPU_FMTS_JUST1  "cpu %Lu %Lu %Lu %Lu %Lu %Lu %Lu"
#define CPU_FMTS_MULTI  "cpu%*d %Lu %Lu %Lu %Lu %Lu %Lu %Lu"

/* Summary Lines specially formatted string(s) --
   see 'show_special' for syntax details + other cautions. */
#define STATES_line2x4  "%s\03" \
   " %#5.1f%% \02user,\03 %#5.1f%% \02system,\03 %#5.1f%% \02nice,\03 %#5.1f%% \02idle\03\n"
#define STATES_line2x5  "%s\03" \
   " %#5.1f%% \02user,\03 %#5.1f%% \02system,\03 %#5.1f%% \02nice,\03 %#5.1f%% \02idle,\03 %#5.1f%% \02IO-wait\03\n"
#define STATES_line2x6  "%s\03" \
   " %#4.1f%% \02us,\03 %#4.1f%% \02sy,\03 %#4.1f%% \02ni,\03 %#4.1f%% \02id,\03 %#4.1f%% \02wa,\03 %#4.1f%% \02hi,\03 %#4.1f%% \02si\03\n"

/* These typedefs attempt to ensure consistent 'ticks' handling */
typedef unsigned long long TIC_t;

/* This structure stores a frame's cpu tics used in history
   calculations.  It exists primarily for SMP support but serves
   all environments. */
typedef struct CPU_t {
	TIC_t u, n, s, i, w, x, y; // as represented in /proc/stat
	TIC_t charge, total;
} CPU_t;

#endif /* LINUX_SYSINFO_H */
