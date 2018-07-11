/*
 * Routines for Solaris.
 *
 * Note that this might not work on differnet versions of Solaris.  The
 * reason being is that the kstat variables can change from version to
 * version.  Read the kvm and kstat man pages to see what Sun says
 * about it.  Also there is no 'real' way to find out the implemented
 * kernel variables!  There is an undocumented feature to netstat which
 * walks the kstat chain.  The output sort of makes sense for kernel
 * hackers.  Anyways use 'netstat -k'.
 *
 * The code is GPL'ed.  See the asmon COPYING file that comes with the
 * distribution.  The only thing I add is that if you use this code
 * and we meet sometime, then you can buy me a beer.  Happy monitoring.
 *
 *      Eric Davis <ead@pobox.com>
 */

#include <kstat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/cpuvar.h>
#include <sys/swap.h>
#include <sys/param.h>


typedef struct swapent   SWAP_ENT_T;
typedef struct swaptable SWAP_TABLE_T;
typedef unsigned long    ULONG;


static int err
    (
    char *      msg
    )
    {
    perror(msg);
    return -1;
    }


int getLoad
    (
    float *     load
    )
    {
    kstat_t *           ks;
    kstat_ctl_t *       kc;
    kstat_named_t *     kn;

    if ((kc = kstat_open()) == NULL)
        return err("kstat_open");

    if ((ks = kstat_lookup(kc, "unix", 0, "system_misc")) == NULL)
        return err("kstat_lookup");

    if (kstat_read(kc, ks, NULL) == -1)
        return err("kstat_read");

    if ((kn = kstat_data_lookup(ks, "avenrun_1min")) == NULL)
        return err("kstat_data_lookup");

    if (kstat_close(kc) == -1)
        return err("kstat_close");

    *load = ((float)kn->value.ul / FSCALE);
    /* printf("load: %2.2f\n\n", *load); */
    return 0;
    }


int getSwap
    (
    ULONG *     swapMax,
    ULONG *     swapFree
    )
    {
    static char         buf[256];
    int                 i;
    int                 totalSwapEntries;
    SWAP_TABLE_T *      swapTable;
    SWAP_ENT_T *        swapEntries;

    TRY_AGAIN:

    totalSwapEntries = swapctl(SC_GETNSWP, 0);
    swapTable = (SWAP_TABLE_T *)malloc(sizeof(int) +
                                       (totalSwapEntries *
                                        sizeof(SWAP_ENT_T)));

    swapTable->swt_n = totalSwapEntries;
    for (swapEntries = &(swapTable->swt_ent[0]),
         i = 0; i < totalSwapEntries; i++, swapEntries++)
        {
        swapEntries->ste_path = buf;
        }

    if (swapctl(SC_LIST, swapTable) == -1)
        {
        /* Retry if our swap table was too small else bail */
        free(swapTable);
        perror("swapctl");
        if (errno == ENOMEM) goto TRY_AGAIN;
        else return -1;
        }

    for (swapEntries = &(swapTable->swt_ent[0]), *swapMax = *swapFree = 0,
         i = 0; i < totalSwapEntries; i++, swapEntries++)
        {
        if (!(swapEntries->ste_flags & ST_INDEL) &&
            !(swapEntries->ste_flags & ST_DOINGDEL))
            {
            *swapMax += swapEntries->ste_pages;
            *swapFree += swapEntries->ste_free;
            }
        }

    *swapMax *= sysconf(_SC_PAGESIZE);
    *swapFree *= sysconf(_SC_PAGESIZE);
    /*
    printf("swap max: %ld\nswap free: %ld\nswap used: %ld\n\n",
           *swapMax, *swapFree, (*swapMax - *swapFree));
    */
    free(swapTable);
    return 0;
    }


int getMem
    (
    ULONG *     memMax,
    ULONG *     memFree
    )
    {
    kstat_t *           ks;
    kstat_ctl_t *       kc;
    kstat_named_t *     kn;

    if ((kc = kstat_open()) == NULL)
        return err("kstat_open");

    if ((ks = kstat_lookup(kc, "unix", 0, "system_pages")) == NULL)
        return err("kstat_lookup");

    if (kstat_read(kc, ks, NULL) == -1)
        return err("kstat_read");

    if ((kn = kstat_data_lookup(ks, "physmem")) == NULL)
        return err("kstat_data_lookup");

    *memMax = (kn->value.ul * sysconf(_SC_PAGESIZE));

    if ((kn = kstat_data_lookup(ks, "freemem")) == NULL)
        return err("kstat_data_lookup");

    *memFree = (kn->value.ul * sysconf(_SC_PAGESIZE));

    if (kstat_close(kc) == -1)
        return err("kstat_close");

    /*
    printf("mem max: %ld\nmem free: %ld\nmem used: %ld\n\n",
           *memMax, *memFree, (*memMax - *memFree));
    */
    return 0;
    }


int getCPU
    (
    ULONG *     cpuIdle,
    ULONG *     cpuUser,
    ULONG *     cpuKern,
    ULONG *     cpuWait,
    ULONG *     pageIn,
    ULONG *     pageOut,
    ULONG *     swapIn,
    ULONG *     swapOut
    )
    {
    cpu_stat_t *        cpu_stat;

    ULONG               numCPUs;
    int                 i;
    kstat_t *           ks;
    kstat_ctl_t *       kc;
    kstat_named_t *     kn;

    if ((kc = kstat_open()) == NULL)
        return err("kstat_open");

    if ((ks = kstat_lookup(kc, "unix", 0, "system_misc")) == NULL)
        return err("kstat_lookup");

    if (kstat_read(kc, ks, NULL) == -1)
        return err("kstat_read");

    if ((kn = kstat_data_lookup(ks, "ncpus")) == NULL)
        return err("kstat_data_lookup");

    numCPUs = kn->value.ul;

    cpu_stat = (cpu_stat_t *)malloc(numCPUs * sizeof(cpu_stat_t));

    for (i = 0, ks = kc->kc_chain; ks != NULL; ks = ks->ks_next)
        {
        if (strncmp(ks->ks_name, "cpu_stat", 8) == 0)
            {
            if (kstat_read(kc, ks, &cpu_stat[i++]) == -1)
                {
                free(cpu_stat);
                return err("kstat_read");
                }
            if (i > numCPUs)
                {
                fprintf(stderr,
                        "getCPU: invalid number of cpus in kstat chain\n");
                free(cpu_stat);
                return -1;
                }
            }
        }
    if (i != numCPUs)
        {
        fprintf(stderr,
                "getCPU: invalid number of cpus in kstat chain\n");
        free(cpu_stat);
        return -1;
        }

    for (*cpuIdle = *cpuUser = *cpuKern = *cpuWait = 0,
         *pageIn = *pageOut = *swapIn = *swapOut = 0,
         i = 0; i < numCPUs; ++i)
        {
        *cpuIdle += cpu_stat[i].cpu_sysinfo.cpu[CPU_IDLE];
        *cpuUser += cpu_stat[i].cpu_sysinfo.cpu[CPU_USER];
        *cpuKern += cpu_stat[i].cpu_sysinfo.cpu[CPU_KERNEL];
        *cpuWait += cpu_stat[i].cpu_sysinfo.cpu[CPU_WAIT];
        *pageIn  += cpu_stat[i].cpu_vminfo.pgpgin;
        *pageOut += cpu_stat[i].cpu_vminfo.pgpgout;
        *swapIn  += cpu_stat[i].cpu_vminfo.pgswapin;
        *swapOut += cpu_stat[i].cpu_vminfo.pgswapout;
        }

    if (kstat_close(kc) == -1)
        {
        free(cpu_stat);
        return err("kstat_close");
        }

    /*
    printf("num cpus: %ld\ncpu idle: %ld\ncpu user: %ld\ncpu kern: %ld\ncpu wait: %ld\npage in: %ld\npage out: %ld\nswap in: %ld\nswap out: %ld\n\n",
           numCPUs, *cpuIdle, *cpuUser, *cpuKern, *cpuWait,
           *pageIn, *pageOut, *swapIn, *swapOut);
    */
    free(cpu_stat);
    return 0;
    }

