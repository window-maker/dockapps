/*
 * cpu_cygwin.c - module to get cpu usage, for Cygwin
 *
 * Copyright (c) 2001 Seiichi SATO <ssato@sh.rim.or.jp>
 *
 * licensed under the GPL
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"

#include <w32api/windows.h>
#include <w32api/windef.h>
#include <w32api/winreg.h>
#include <w32api/winbase.h>

#define WIN32_9x 1		/* 95, 98, Me   */
#define WIN32_NT 2		/* NT, 2000, XP */

/* NT, 2000, XP */
#define LONGINT2DOUBLE(x) ((double)((x).HighPart) * 4.294967296E9 + (double)((x).LowPart))
/*
 * The following both data structures aren't defined anywhere in the Microsoft
 * header files. Taken from DNA's libraly licensed under GPL.
 * (http://estu.nit.ac.jp/~e982457/freesoft/freesoft.html)
 */
typedef struct _SYSTEM_BASIC_INFORMATION {		/* Info Class 0 */
    DWORD   unused1;
    ULONG   unused2[6];
    PVOID   unused3[2];
    ULONG   unused4;
    BYTE    bKeNumberProcessors;
    BYTE    unused5;
    WORD    unused6;
} SYSTEM_BASIC_INFORMATION;

typedef struct _SYSTEM_PERFORMANCE_INFORMATION {	/* Info Class 2 */
    LARGE_INTEGER IdleTime;
    DWORD unused[76];
} SYSTEM_PERFORMANCE_INFORMATION;

typedef struct _SYSTEM_TIME_INFORMATION {		/* Info Class 3 */
    LARGE_INTEGER liKeBootTime;
    LARGE_INTEGER liKeSystemTime;
    LARGE_INTEGER liExpTimeZoneBias;
    ULONG uCurrentTimeZoneId;
    DWORD dwReserved;
} SYSTEM_TIME_INFORMATION;

#define SystemBasicInformation		0
#define SystemPerformanceInformation	2
#define SystemTimeInformation		3
/* end of NT, 2000, XP */

static int platform = 0;

void
cpu_init(void)
{
    OSVERSIONINFO os_ver_info;

    /* which version? */
    os_ver_info.dwOSVersionInfoSize = sizeof(os_ver_info);
    GetVersionEx(&os_ver_info);
    platform = os_ver_info.dwPlatformId;

    if ((platform != WIN32_9x) && (platform != WIN32_NT)) {
	fprintf(stderr, "%s: unknown platform\n", PACKAGE);
	exit (1);
    }
}

/*
 * cpu_get_usage_9x(): get cpu usage in percent via registry
 *
 * How to get:
 *  1. query 'PerfStats.StartStat.KERNEL.CPUUsage' to start monitoring
 *  2. get usage from 'PerfStats.StatData.KERNEL.CPUUsage'
 *  3. query 'PerfStats.StopStat.KERNEL.CPUUsage' to stop monitoring
 *
 * If cpu usage is 100% evry time, please reboot.;(
 */
static int
cpu_get_usage_9x(cpu_options *opts)
{
    int usage = 0;

    HKEY hkeys; /* for monitoring (start, stop) */
    HKEY hkeyr; /* for reading usage            */
    DWORD dummy;
    DWORD dwsize = sizeof(DWORD);

    if (RegOpenKeyEx(HKEY_DYN_DATA, "PerfStats\\StatData",
		     0, KEY_READ, &hkeyr) != ERROR_SUCCESS) {
	fprintf(stderr, "%s: could not open registry 'PerfStats\\StatData'\n", PACKAGE);
	return 0;
    }

    /* start monitoring */
    RegOpenKeyEx(HKEY_DYN_DATA, "PerfStats\\StartStat", 0, KEY_READ, &hkeys);
    RegQueryValueEx(hkeys, "KERNEL\\CPUUsage", 0, NULL, (LPBYTE)&dummy,
		    &dwsize);
    RegCloseKey(hkeys);

    /* get usage */
    RegQueryValueEx(hkeyr, "KERNEL\\CPUUsage", 0, NULL, (LPBYTE)&usage,
		    &dwsize);
    RegCloseKey(hkeyr);

    /* stop monitoring */
    RegOpenKeyEx(HKEY_DYN_DATA, "PerfStats\\StopStat", 0, KEY_READ, &hkeys);
    RegQueryValueEx(hkeys, "KERNEL\\CPUUsage", 0, NULL, (LPBYTE)&dummy,
		    &dwsize);
    RegCloseKey(hkeys);

    return usage;
}

/*
 * cpu_get_usage_NT:
 *
 * How to get:
 *  1. Load NTDLL.DLL (should use dlopen?)
 *  2. Get addresses of NtQuerySystemInformation (should use dlsym?)
 *  3. Get system time and idle time
 *  4. Calculate cpu usage
 *  5. Unload NTDLL.DLL (should use dlclose?)
 *
 * I do not test this function with SMP system, since I do not have SMP system.
 */
static int
cpu_get_usage_NT(cpu_options *opts)
{
    int usage;
    double total, used;
    static double pre_total = 0, pre_used = 0;

    HINSTANCE h_ntdll;
    FARPROC NtQuerySystemInformation = NULL;

    SYSTEM_BASIC_INFORMATION	   sbi;
    SYSTEM_TIME_INFORMATION	   sti;
    SYSTEM_PERFORMANCE_INFORMATION spi;

    if ((h_ntdll = LoadLibraryEx("NTDLL.DLL", NULL, 0)) == NULL) {
	fprintf(stderr, "%s: could not load NTDLL.DLL\n", PACKAGE);
	exit (1);
    }

    NtQuerySystemInformation = GetProcAddress(h_ntdll,
					      "NtQuerySystemInformation");
    if (!NtQuerySystemInformation) {
	fprintf(stderr, "%s: could not find NtQuerySystemInformation()\n", PACKAGE);
	FreeLibrary(h_ntdll);
	return 0;
    }

    if ((NtQuerySystemInformation(SystemBasicInformation,
				  &sbi,
		                  sizeof(SYSTEM_BASIC_INFORMATION),
				  NULL)) != NO_ERROR)
	return 0;

    if ((NtQuerySystemInformation(SystemTimeInformation,
				  &sti,
				  sizeof(SYSTEM_TIME_INFORMATION),
				  0)) != NO_ERROR)
	return 0;

    if ((NtQuerySystemInformation(SystemPerformanceInformation,
				  &spi,
				  sizeof(SYSTEM_PERFORMANCE_INFORMATION),
				  0)) != NO_ERROR)
	return 0;

    total = LONGINT2DOUBLE(sti.liKeSystemTime);
    used = total - LONGINT2DOUBLE(spi.IdleTime);

    if ((pre_total == 0) || !(total - pre_total > 0))  {
	usage = 0;
    } else {
	usage = (100 * (used - pre_used)) / (total - pre_total);
    }

    if (sbi.bKeNumberProcessors > 1) {
	usage = usage / sbi.bKeNumberProcessors;
    }

    pre_used = used;
    pre_total = total;

    FreeLibrary(h_ntdll);

    return usage;
}

/* return current cpu usage in percent */
int
cpu_get_usage(cpu_options *opts)
{
    switch (platform) {
	case WIN32_9x:
	    return cpu_get_usage_9x(opts);
	case WIN32_NT:
	    return cpu_get_usage_NT(opts);
	default: /* make gcc happy */
	    break;
    }
    return 0;
}
