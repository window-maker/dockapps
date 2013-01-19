/*
 * sysinfo-solaris.c
 *
 * System information gathering for Solaris
 */

#include "general.h"
#include "standards.h"
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <stdint.h>
#include <kstat.h>
#include <sys/sysinfo.h>
#include <sys/swap.h>

/*
 * The index field is for a fast lookup on the string.
 * A -1 tells stat_data_lookup() that we need to locate
 * the string.  Once stat_data_lookup() has located the
 * string, it will set the index of were we found it.
 */
typedef struct cpu_states_info {
	char *field_name;
	int index;
} cpu_states_info_t;

#define	CPU_STATES 4
static struct cpu_states_info cpu_states[CPU_STATES] = {
	{"cpu_ticks_idle", -1},
	{"cpu_ticks_user", -1},
	{"cpu_ticks_kernel", -1},
	{"cpu_ticks_wait", -1}
};

static kstat_ctl_t *kcp = NULL;
static kstat_t *ksp_old;

static uint64_t physmem = 0;

/* NumCPUs_DoInit returns the number of CPUs present in the system and
   performs any initialization necessary for the sysinfo-XXX module */
unsigned int NumCpus_DoInit(void)
{
	int smp_num_cpus;
	int i;

	kcp = kstat_open();
	if (kcp == NULL)
		exit(1);

	physmem = sysconf(_SC_PHYS_PAGES);
	smp_num_cpus = sysconf(_SC_NPROCESSORS_CONF);
	if (smp_num_cpus < 1) {
		smp_num_cpus = 1; /* SPARC glibc is buggy */
	}

	if (smp_num_cpus > 255) {
		/* we don't support more than 255 CPUs (well, in fact no more
		   than two ate the moment... */
		smp_num_cpus = 255;
	}

	ksp_old = malloc(smp_num_cpus * sizeof (kstat_t));
	if (ksp_old == NULL) {
		kstat_close(kcp);
		fprintf(stderr, "ERROR: Can't allocate cpu load history.\n");
		exit(1);
	}

	for (i = 0; i < smp_num_cpus; i++) {
		ksp_old[i].ks_data = NULL;
		ksp_old[i].ks_data_size = 0;
	}

	return smp_num_cpus;
}

/*
 * If index_ptr integer value is > -1 then the index points to the
 * string entry in the ks_data that we are interested in. Otherwise
 * we will need to walk the array.
 */
void *stat_data_lookup(kstat_t *ksp, char *name, int *index_ptr)
{
	int i;
	int size;
	int index;
	char *namep, *datap;

	switch (ksp->ks_type) {
		case KSTAT_TYPE_NAMED:
			size = sizeof (kstat_named_t);
			namep = KSTAT_NAMED_PTR(ksp)->name;
			break;
		case KSTAT_TYPE_TIMER:
			size = sizeof (kstat_timer_t);
			namep = KSTAT_TIMER_PTR(ksp)->name;
			break;
		default:
			errno = EINVAL;
			return (NULL);
	}

	index = *index_ptr;
	if (index >= 0) {
		/* Short cut to the information. */
		datap = ksp->ks_data;
		datap = &datap[size*index];
		return (datap);
	}

	/* Need to go find the string. */
	datap = ksp->ks_data;
	for (i = 0; i < ksp->ks_ndata; i++) {
		if (strcmp(name, namep) == 0) {
			*index_ptr = i;
			return (datap);
		}
		namep += size;
		datap += size;
	}
	errno = ENOENT;
	return (NULL);
}

uint64_t kstat_delta(kstat_t *old, kstat_t *new, char *name, int *index)
{
	kstat_named_t *knew = stat_data_lookup(new, name, index);

	if (old && old->ks_data) {
		kstat_named_t *kold = stat_data_lookup(old, name, index);
		return (knew->value.ui64 - kold->value.ui64);
	}
	return (knew->value.ui64);
}

uint64_t cpu_ticks_delta(kstat_t *old, kstat_t *new)
{
	uint64_t ticks = 0;
	size_t i;

	for (i = 0; i < CPU_STATES; i++) {
		ticks += kstat_delta(old, new, cpu_states[i].field_name,
		    &cpu_states[i].index);
	}
	return ((ticks == 0) ? 1 : ticks);
}

int kstat_copy(const kstat_t *src, kstat_t *dst)
{
	void *dst_data = NULL;

	if (dst->ks_data && dst->ks_data_size < src->ks_data_size)
		free((void *)dst->ks_data);
	else
		dst_data = dst->ks_data;

	*dst = *src;

	if (src->ks_data != NULL) {
		if (dst_data)
			dst->ks_data = dst_data;
		else if ((dst->ks_data = malloc(src->ks_data_size)) == NULL)
			return (-1);
		bcopy(src->ks_data, dst->ks_data, src->ks_data_size);
	} else {
		if (dst_data)
			free((void *)dst_data);
		dst->ks_data = NULL;
		dst->ks_data_size = 0;
	}
	return (0);
}

/* Get_CPU_Load returns an array of CPU loads, one for each CPU, scaled
   to HAUTEUR. The array is defined and allocated by the main program
   and passed to the function as '*load'. The number of CPUs present
   is given in 'Cpu_tot' */
unsigned int *Get_CPU_Load(unsigned int *load, unsigned int Cpu_tot)
{
	kstat_t *ksp_new;
	double factor;
	uint64_t cur_load;
	int i;

	if (kcp == NULL || ksp_old == NULL)
		return (load);

	for (i = 0; i < Cpu_tot; i++) {
		if ((ksp_new = kstat_lookup(kcp, "cpu", i, "sys")) == NULL) {
			load[i] = 0;
			continue;
		}

		if (kstat_read(kcp, ksp_new, NULL) == -1) {
			load[i] = 0;
			continue;
		}

		cur_load = cpu_ticks_delta(&ksp_old[i], ksp_new);
		factor = HAUTEUR / (double)cur_load;

		cur_load = kstat_delta(&ksp_old[i], ksp_new,
		    cpu_states[1].field_name, &cpu_states[1].index) +
		    kstat_delta(&ksp_old[i], ksp_new, cpu_states[2].field_name,
		    &cpu_states[2].index);
		if (ksp_old[i].ks_data) {
			load[i] = factor * cur_load;
		}
		kstat_copy(ksp_new, &ksp_old[i]);
	}

	return (load);
}

/* return current memory/swap usage on a scale from 0-100 */
unsigned int Get_Memory(void)
{
	kstat_t *ksp_new;
	static vminfo_t *vm_new = NULL;
	static vminfo_t *vm_old = NULL;
	vminfo_t *vm_swap;
	static uint64_t freemem = 0;

	if ((ksp_new = kstat_lookup(kcp, "unix", 0, "vminfo")) == NULL) {
		return (0);
	}

	if (vm_new == NULL && (vm_new = malloc(sizeof (vminfo_t))) == NULL) {
		return (0);
	}

	if (kstat_read(kcp, ksp_new, vm_new) == -1) {
		return (0);
	}

	if (vm_old != NULL) {
		uint64_t step = vm_new->updates - vm_old->updates;

		if (step > 0) {
			freemem = (vm_new->freemem - vm_old->freemem) / step;
		}
	}

	vm_swap = vm_new;
	vm_new = vm_old;
	vm_old = vm_swap;

	if (vm_new == NULL)
		return (0);

	return (100 * (physmem - freemem) / physmem);
}

unsigned int Get_Swap(void)
{
	struct anoninfo ai;

	if (swapctl(SC_AINFO, &ai) == -1) {
		return (0);
	}

	return (100 * (ai.ani_max - ai.ani_free) / ai.ani_max);
}
