/*
 * longrun_linux.c - module to get LongRun(TM) status, for GNU/Linux
 *
 * Copyright(C) 2001 Transmeta Corporation
 * Copyright(C) 2001 Seiichi SATO <ssato@sh.rim.or.jp>
 *
 * licensed under the GPL
 */
#undef USE_PREAD

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#ifdef USE_PREAD
#define __USE_UNIX98	     /* for pread/pwrite */
#endif
#define __USE_FILE_OFFSET64 /* msr/cpuid needs 64bit address with newer linuxes */
#include <unistd.h>
#include "common.h"

#define MSR_DEVICE "/dev/cpu/0/msr"
#define MSR_TMx86_LONGRUN	0x80868010
#define MSR_TMx86_LONGRUN_FLAGS	0x80868011

#define LONGRUN_MASK(x)		((x) & 0x0000007f)
#define LONGRUN_RESERVED(x)	((x) & 0xffffff80)
#define LONGRUN_WRITE(x, y)	(LONGRUN_RESERVED(x) | LONGRUN_MASK(y))

#define CPUID_DEVICE "/dev/cpu/0/cpuid"
#define CPUID_TMx86_VENDOR_ID		0x80860000
#define CPUID_TMx86_PROCESSOR_INFO	0x80860001
#define CPUID_TMx86_LONGRUN_STATUS	0x80860007
#define CPUID_TMx86_FEATURE_LONGRUN(x)	((x) & 0x02)

static int  cpuid_fd;			/* CPUID file descriptor */
static char *cpuid_device;		/* CPUID device name */
static int  msr_fd;			/* MSR file descriptor */
static char *msr_device;		/* MSR device name */

static void
read_cpuid(loff_t address, int *eax, int *ebx, int *ecx, int *edx)
{
    uint32_t data[4];

#ifdef USE_PREAD
    if (pread(cpuid_fd, &data, 16, address) != 16) {
	perror("pread");
#else
    if (lseek(cpuid_fd, address, SEEK_SET) != address) {
	perror("lseek");
	exit(1);
    }
    if (read(cpuid_fd, &data, 16) != 16) {
	perror("read");
#endif
	exit(1);
    }

    if (eax)
	*eax = data[0];
    if (ebx)
	*ebx = data[1];
    if (ecx)
	*ecx = data[2];
    if (edx)
	*edx = data[3];
}

/* note: if an output is NULL, then don't set it */
static void
read_msr(loff_t address, int *lower, int *upper)
{
    uint32_t data[2];

#ifdef USE_PREAD
    if (pread(msr_fd, &data, 8, address) != 8) {
	perror("pread");
#else
    if (lseek(msr_fd, address, SEEK_SET) != address) {
	perror("lseek");
	exit(1);
    }
    if (read(msr_fd, &data, 8) != 8) {
	perror("read");
#endif
	exit(1);
    }

    if (lower)
	*lower = data[0];
    if (upper)
	*upper = data[1];
}

void
longrun_init(char *cpuid_dev, char *msr_dev)
{
    int eax, ebx, ecx, edx;

    /* set CPUID device */
    cpuid_device = CPUID_DEVICE;
    if (cpuid_dev)
	cpuid_device = cpuid_dev;

    /* set MSR device */
    msr_device = MSR_DEVICE;
    if (msr_dev)
	msr_device = msr_dev;

    /* open CPUID device */
    if ((cpuid_fd = open(cpuid_device, O_RDONLY)) < 0) {
	fprintf(stderr, "error opening %s\n", cpuid_device);
	if (errno == ENODEV)
	    fprintf(stderr,
		    "make sure your kernel was compiled with CONFIG_X86_CPUID=y\n");
	exit(1);
    }

    /* open MSR device */
    if ((msr_fd = open(msr_device, O_RDONLY)) < 0) {
	fprintf(stderr, "error opening %s\n", msr_device);
	if (errno == ENODEV)
	    fprintf(stderr,
		    "make sure your kernel was compiled with CONFIG_X86_MSR=y\n");
	exit(1);
    }

    /* test for "TransmetaCPU" */
    read_cpuid(CPUID_TMx86_VENDOR_ID, &eax, &ebx, &ecx, &edx);
    if (ebx != 0x6e617254 || ecx != 0x55504361 || edx != 0x74656d73) {
	fprintf(stderr, "not a Transmeta x86 CPU\n");
	exit(1);
    }

    /* test for LongRun feature flag */
    read_cpuid(CPUID_TMx86_PROCESSOR_INFO, &eax, &ebx, &ecx, &edx);
    if (!CPUID_TMx86_FEATURE_LONGRUN(edx)) {
	printf("LongRun: unsupported\n");
	exit(0);
    }

    /* close device */
    close(cpuid_fd);
    close(msr_fd);
}



/*
 * percent:  performance level (0 to 100)
 * flags:    performance/economy/unknown
 * mhz:      frequency
 * volts:    voltage
 */
void
longrun_get_stat(int *percent, int *flags, int *mhz, int *voltz)
{
    int eax, ebx, ecx;

    /* open CPUID device */
    if ((cpuid_fd = open(cpuid_device, O_RDWR)) < 0) {
	fprintf(stderr, "error opening %s\n", cpuid_device);
	if (errno == ENODEV)
	    fprintf(stderr,
		    "make sure your kernel was compiled with CONFIG_X86_CPUID=y\n");
	exit(1);
    }

    /* open MSR device */
    if ((msr_fd = open(msr_device, O_RDWR)) < 0) {
	fprintf(stderr, "error opening %s\n", msr_device);
	if (errno == ENODEV)
	    fprintf(stderr,
		    "make sure your kernel was compiled with CONFIG_X86_MSR=y\n");
	exit(1);
    }

    /* frequency, voltage, performance level, */
    read_cpuid(CPUID_TMx86_LONGRUN_STATUS, &eax, &ebx, &ecx, 0);
    *mhz = eax;
    *voltz = ebx;
    *percent = ecx;

    /* flags */
    read_msr(MSR_TMx86_LONGRUN_FLAGS, flags, NULL);
    *flags =
	(*flags & 1) ? LONGRUN_FLAGS_PEFORMANCE : LONGRUN_FLAGS_ECONOMY;

    /* close device */
    close(cpuid_fd);
    close(msr_fd);
}
