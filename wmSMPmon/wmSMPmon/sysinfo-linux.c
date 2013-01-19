/*######################################################################
  #                                                                    #
  # This file contains all functions necessary to provide system       #
  # information on a Linux 2.2-2.6 system.                             #
  # All routines were taken from/based on code from top and/or procps. #
  #                                                                    #
  # With thanks to the original authors:                               #
  #        - James C. Warner <warnerjc@worldnet.att.net>               #
  #        - Michael K. Johnson <johnsonm@redhat.com>                  #
  #        - Albert D. Cahalan, <albert@users.sf.net>                  #
  #        - Fabian Frederick                                          #
  #                                                                    #
  # This file is placed under the conditions of the GNU Library        #
  # General Public License, version 2, or any later version.           #
  # See file COPYING for information on distribution conditions.       #
  #                                                                    #
  ######################################################################*/

#include "sysinfo-linux.h" /* include self to verify prototypes */
#include "general.h"
#include "standards.h"

#define BAD_OPEN_MESSAGE					\
"Error: /proc must be mounted\n"				\
"  To mount /proc at boot you need an /etc/fstab line like:\n"	\
"      /proc   /proc   proc    defaults\n"			\
"  In the meantime, mount /proc /proc -t proc\n"

#define MEMINFO_FILE "/proc/meminfo"
static int meminfo_fd = -1;

static char buf[1024];

/* assume no IO-wait stats (default kernel 2.4.x),
   overridden if linux 2.5.x or 2.6.x */
static const char *States_fmts = STATES_line2x4;


/* This macro opens filename only if necessary and seeks to 0 so
 * that successive calls to the functions are more efficient.
 * It also reads the current contents of the file into the global buf.
 */
#define FILE_TO_BUF(filename, fd) do {				\
	static int local_n;					\
	if (fd == -1 && (fd = open(filename, O_RDONLY)) == -1) {\
		fprintf(stderr, BAD_OPEN_MESSAGE);		\
		fflush(NULL);					\
		_exit(102);					\
	}							\
	lseek(fd, 0L, SEEK_SET);				\
	if ((local_n = read(fd, buf, sizeof buf - 1)) < 0) {\
		perror(filename);				\
		fflush(NULL);					\
		_exit(103);					\
	}							\
	buf[local_n] = '\0';					\
} while(0)

#define LINUX_VERSION(x,y,z)   (0x10000*(x) + 0x100*(y) + z)


/***********************************************/
/* get number of CPUs - max. 255 are supported */
/* also, perform some initialisation           */
/* code taken from top and procps              */
/***********************************************/
unsigned int NumCpus_DoInit(void)
{
	long smp_num_cpus;
	int linux_version_code;

	static struct utsname uts;
	int x = 0, y = 0, z = 0;	/* cleared in case sscanf() < 3 */

	if (uname(&uts) == -1)	/* failure implies impending death */
		exit(1);
	if (sscanf(uts.release, "%d.%d.%d", &x, &y, &z) < 3)
		fprintf(stderr,	/* *very* unlikely to happen by accident */
		    "Non-standard uts for running kernel:\n"
		    "release %s=%d.%d.%d gives version code %d\n",
		    uts.release, x, y, z, LINUX_VERSION(x,y,z));
	linux_version_code = LINUX_VERSION(x, y, z);

	if (linux_version_code > LINUX_VERSION(2, 5, 41))
		States_fmts = STATES_line2x5;
	if (linux_version_code >= LINUX_VERSION(2, 6, 0)) {
		// grrr... only some 2.6.0-testX :-(
		States_fmts = STATES_line2x6;
	}

	smp_num_cpus = sysconf(_SC_NPROCESSORS_CONF); // or _SC_NPROCESSORS_ONLN
	if (smp_num_cpus < 1) {
		smp_num_cpus = 1; /* SPARC glibc is buggy */
	}

	if (smp_num_cpus > 255) {
		/* we don't support more than 255 CPUs (well, in fact no more
		   than two ate the moment... */
		smp_num_cpus = 255;
	}

	return (int)smp_num_cpus;
}

/***********************************************************************/
/*
 * Copyright 1999 by Albert Cahalan; all rights reserved.
 * This file may be used subject to the terms and conditions of the
 * GNU Library General Public License Version 2, or any later version
 * at your option, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Library General Public License for more details.
 */
typedef struct mem_table_struct {
	const char *name;     /* memory type name */
	unsigned long *slot;  /* slot in return struct */
} mem_table_struct;

static int compare_mem_table_structs(const void *a, const void *b)
{
    return strcmp(((const mem_table_struct *) a)->name,
	((const mem_table_struct *) b)->name);
}

/* example data, following junk, with comments added:
 *
 * MemTotal:        61768 kB    old
 * MemFree:          1436 kB    old
 * MemShared:           0 kB    old (now always zero; not calculated)
 * Buffers:          1312 kB    old
 * Cached:          20932 kB    old
 * Active:          12464 kB    new
 * Inact_dirty:      7772 kB    new
 * Inact_clean:      2008 kB    new
 * Inact_target:        0 kB    new
 * Inact_laundry:       0 kB    new, and might be missing too
 * HighTotal:           0 kB
 * HighFree:            0 kB
 * LowTotal:        61768 kB
 * LowFree:          1436 kB
 * SwapTotal:      122580 kB    old
 * SwapFree:        60352 kB    old
 * Inactive:        20420 kB    2.5.41+
 * Dirty:               0 kB    2.5.41+
 * Writeback:           0 kB    2.5.41+
 * Mapped:           9792 kB    2.5.41+
 * Slab:             4564 kB    2.5.41+
 * Committed_AS:     8440 kB    2.5.41+
 * PageTables:        304 kB    2.5.41+
 * ReverseMaps:      5738       2.5.41+
 * SwapCached:          0 kB    2.5.??+
 * HugePages_Total:   220       2.5.??+
 * HugePages_Free:    138       2.5.??+
 * Hugepagesize:     4096 kB    2.5.??+
 */

/* obsolete */
unsigned long kb_main_shared;
/* old but still kicking -- the important stuff */
unsigned long kb_main_buffers;
unsigned long kb_main_cached;
unsigned long kb_main_free;
unsigned long kb_main_total;
unsigned long kb_swap_free;
unsigned long kb_swap_total;
/* recently introduced */
unsigned long kb_high_free;
unsigned long kb_high_total;
unsigned long kb_low_free;
unsigned long kb_low_total;
/* 2.4.xx era */
unsigned long kb_active;
unsigned long kb_inact_laundry;
unsigned long kb_inact_dirty;
unsigned long kb_inact_clean;
unsigned long kb_inact_target;
unsigned long kb_swap_cached;  /* late 2.4 only */
/* derived values */
unsigned long kb_swap_used;
unsigned long kb_main_used;
/* 2.5.41+ */
unsigned long kb_writeback;
unsigned long kb_slab;
unsigned long nr_reversemaps;
unsigned long kb_committed_as;
unsigned long kb_dirty;
unsigned long kb_inactive;
unsigned long kb_mapped;
unsigned long kb_pagetables;

static void meminfo(void)
{
	char namebuf[16]; /* big enough to hold any row name */
	mem_table_struct findme = { namebuf, NULL};
	mem_table_struct *found;
	char *head;
	char *tail;
	static const mem_table_struct mem_table[] = {
		{"Active",       &kb_active},       // important
		{"Buffers",      &kb_main_buffers}, // important
		{"Cached",       &kb_main_cached},  // important
		{"Committed_AS", &kb_committed_as},
		{"Dirty",        &kb_dirty},        // kB version of vmstat nr_dirty
		{"HighFree",     &kb_high_free},
		{"HighTotal",    &kb_high_total},
		{"Inact_clean",  &kb_inact_clean},
		{"Inact_dirty",  &kb_inact_dirty},
		{"Inact_laundry",&kb_inact_laundry},
		{"Inact_target", &kb_inact_target},
		{"Inactive",     &kb_inactive},     // important
		{"LowFree",      &kb_low_free},
		{"LowTotal",     &kb_low_total},
		{"Mapped",       &kb_mapped},       // kB version of vmstat nr_mapped
		{"MemFree",      &kb_main_free},    // important
		{"MemShared",    &kb_main_shared},  // important
		{"MemTotal",     &kb_main_total},   // important
		{"PageTables",   &kb_pagetables},   // kB version of vmstat
						    // nr_page_table_pages
		{"ReverseMaps",  &nr_reversemaps},  // same as vmstat
						    // nr_page_table_pages
		{"Slab",         &kb_slab},         // kB version of vmstat nr_slab
		{"SwapCached",   &kb_swap_cached},
		{"SwapFree",     &kb_swap_free},    // important
		{"SwapTotal",    &kb_swap_total},   // important
		{"Writeback",    &kb_writeback},    // kB version of vmstat
						    // nr_writeback
	};
	const int mem_table_count =
	    sizeof(mem_table) / sizeof(mem_table_struct);

	FILE_TO_BUF(MEMINFO_FILE,meminfo_fd);

	kb_inactive = ~0UL;

	head = buf;
	for(;;) {
		tail = strchr(head, ':');
		if (!tail)
			break;
		*tail = '\0';
		if (strlen(head) >= sizeof(namebuf)) {
			head = tail + 1;
			goto nextline;
		}
		strcpy(namebuf, head);
		found = bsearch(&findme, mem_table, mem_table_count,
		    sizeof(mem_table_struct), compare_mem_table_structs);
		head = tail + 1;
		if (!found)
			goto nextline;
		*(found->slot) = strtoul(head, &tail, 10);
nextline:
		tail = strchr(head, '\n');
		if (!tail)
			break;
		head = tail + 1;
	}
	if (!kb_low_total) {  /* low==main except with large-memory support */
		kb_low_total = kb_main_total;
		kb_low_free  = kb_main_free;
	}
	if (kb_inactive == ~0UL) {
		kb_inactive = kb_inact_dirty + kb_inact_clean + kb_inact_laundry;
	}
	kb_swap_used = kb_swap_total - kb_swap_free;
	kb_main_used = kb_main_total - kb_main_free;
}

/*************************************************************************/
/*
 * This guy's modeled on libproc's 'five_cpu_numbers' function except
 * we preserve all cpu data in our CPU_t array which is organized
 * as follows:
 *    cpus[0] thru cpus[n] == tics for each separate cpu
 *    cpus[Cpu_tot]        == tics from the 1st /proc/stat line */
static CPU_t *cpus_refresh (CPU_t *cpus, unsigned int Cpu_tot)
{
	static FILE *fp = NULL;
	int i;

	// enough for a /proc/stat CPU line (not the intr line)
	char buf[SMLBUFSIZ];

	/* by opening this file once, we'll avoid the hit on minor page faults
	   (sorry Linux, but you'll have to close it for us) */
	if (!fp) {
		if (!(fp = fopen("/proc/stat", "r")))
			std_err(fmtmk("Failed /proc/stat open: %s",
			    strerror(errno)));
		/* note: we allocate one more CPU_t than Cpu_tot so that the
		   last slot can hold tics representing the /proc/stat cpu
		   summary (the first line read) -- that slot supports our
		   View_CPUSUM toggle */
		cpus = alloc_c((1 + Cpu_tot) * sizeof(CPU_t));
	}
	rewind(fp);
	fflush(fp);

	// first value the last slot with the cpu summary line
	if (!fgets(buf, sizeof(buf), fp))
		std_err("failed /proc/stat read");

	cpus[Cpu_tot].x = 0;  // FIXME: can't tell by kernel version number
	cpus[Cpu_tot].y = 0;  // FIXME: can't tell by kernel version number
	if (4 > sscanf(buf, CPU_FMTS_JUST1, &cpus[Cpu_tot].u, &cpus[Cpu_tot].n,
	    &cpus[Cpu_tot].s, &cpus[Cpu_tot].i, &cpus[Cpu_tot].w,
	    &cpus[Cpu_tot].x, &cpus[Cpu_tot].y))
		std_err("failed /proc/stat read");
	// and just in case we're 2.2.xx compiled without SMP support...
	if (1 == Cpu_tot) {
		/* do it "manually", otherwise we overwrite charge and total */
		cpus[0].u = cpus[1].u;
		cpus[0].n = cpus[1].n;
		cpus[0].s = cpus[1].s;
		cpus[0].i = cpus[1].i;
		cpus[0].w = cpus[1].w;
		cpus[0].x = cpus[1].x;
		cpus[0].y = cpus[1].y;
	}

	// now value each separate cpu's tics
	for (i = 0; 1 < Cpu_tot && i < Cpu_tot; i++) {
		if (!fgets(buf, sizeof(buf), fp))
			std_err("failed /proc/stat read");
		cpus[i].x = 0;  // FIXME: can't tell by kernel version number
		cpus[i].y = 0;  // FIXME: can't tell by kernel version number
		if (4 > sscanf(buf, CPU_FMTS_MULTI, &cpus[i].u,
		    &cpus[i].n, &cpus[i].s, &cpus[i].i, &cpus[i].w,
		    &cpus[i].x, &cpus[i].y))
			std_err("failed /proc/stat read");
	}
	return cpus;
}

unsigned int *Get_CPU_Load(unsigned int *load, unsigned int Cpu_tot)
{
	static CPU_t  *smpcpu = NULL;
	unsigned int j;
	register unsigned long charge, total = 0;

	smpcpu = cpus_refresh(smpcpu, Cpu_tot);

	for (j = 0; j < Cpu_tot; j ++) {
		charge = smpcpu[j].u + smpcpu[j].s + smpcpu[j].n;
		total = charge + smpcpu[j].i;

		/* scale cpu to a maximum of HAUTEUR */
		load[j] = ((HAUTEUR * (charge - smpcpu[j].charge)) /
		    (total - smpcpu[j].total + 0.001)) + 1 ;
		smpcpu[j].total = total ;
		smpcpu[j].charge = charge ;
	}

	return load;
}

unsigned int Get_Memory(void)
{
	meminfo();

	return ((kb_main_used - kb_main_cached) / (kb_main_total / 100));
	/* should be between 0 and 100 now */
}

unsigned int Get_Swap(void)
{
	/* returns swap usage as value between 0 and 100
	 * OR 999 if no swap present */
	meminfo();

	return (kb_swap_total == 0 ? 999 : kb_swap_used / (kb_swap_total / 100));
}
