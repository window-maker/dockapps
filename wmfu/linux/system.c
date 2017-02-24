/*
 * Copyright (c) 2007 Daniel Borca  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include <arpa/inet.h>
#include <ctype.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/wireless.h>

#include "../list.h"
#include "../sensors.h"
#include "../system.h"


int
system_get_uptime (int *days, int *hours, int *mins)
{
    char buf[BUFSIZ];

    unsigned long ut;

    if (sensors_read_line("/proc/uptime", BUFSIZ, buf) <= 0) {
	return 0;
    }

    if (sscanf(buf, "%lu", &ut) != 1) {
	return 0;
    }

    ut /= 60;
    *mins = ut % 60;
    ut /= 60;
    *hours = ut % 24;
    ut /= 24;
    *days = ut;

    return 1;
}


int
system_get_cpu_load (int num, CPUSTAT *avg, CPUSTAT load[])
{
    FILE *f;
    char buf[BUFSIZ];

    int i;
    int ok = 0;

    f = fopen("/proc/stat", "rt");
    if (f == NULL) {
	return 0;
    }

    if (avg != NULL) {
	avg->used = -1;
    }
    for (i = 0; i < num; i++) {
	load[i].used = -1;
    }

    while (fgets(buf, BUFSIZ, f)) {
	CPUSTAT *p;
	int array[10], total;
	int n;

	if (buf[0] == 'c' && buf[1] == 'p' && buf[2] == 'u' && buf[3] == ' ') {
	    if (avg == NULL) {
		continue;
	    }
	    n = sscanf(buf + 4, "%d %d %d %d %d %d %d %d %d %d",
		&array[0], &array[1], &array[2], &array[3],
		&array[4], &array[5], &array[6], &array[7],
		&array[8], &array[9]);
	    if (n < 4 || avg == NULL) {
		continue;
	    }
	    p = avg;
	    ok |= 1;
	} else {
	    if (!num) {
		continue;
	    }
	    n = sscanf(buf, "cpu%d %d %d %d %d %d %d %d %d %d %d", &i,
		&array[0], &array[1], &array[2], &array[3],
		&array[4], &array[5], &array[6], &array[7],
		&array[8], &array[9]);
	    if (--n < 4 || i >= num) {
		continue;
	    }
	    p = &load[i];
	    ok |= 2;
	}

	for (total = 0; n--; total += array[n]) {
	}

	p->used = 0;
	if (total != p->old_total) {
	    p->used = 100 - 100 * (array[3] - p->old_idle) / (total - p->old_total);
	}

	p->old_idle = array[3];
	p->old_total = total;
    }
    fclose(f);

    return ok;
}


int
system_get_cpu_speed (int num, int speed[])
{
    FILE *f;
    char buf[BUFSIZ];

    int i;
    int ok = 0;

    f = fopen("/proc/cpuinfo", "rt");
    if (f == NULL) {
	return 0;
    }

    for (i = 0; i < num; i++) {
	speed[i] = -1;
    }

    while (fgets(buf, BUFSIZ, f)) {
	sscanf(buf, "processor : %d", &i);
	if (i < num) {
	    ok += sscanf(buf, "cpu MHz : %d", &speed[i]);
	}
    }
    fclose(f);

    return ok;
}


int
system_get_cpu_gov (int cpu, int max, char *out)
{
    char filename[128];

    sprintf(filename, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor", cpu);

    if (sensors_read_line(filename, max, out) <= 0) {
	return 0;
    }

    return 1;
}


int
system_get_mem_stat (int *mem_free, int *mem_total, int *swp_free, int *swp_total)
{
    FILE *f;
    char buf[BUFSIZ];

    f = fopen("/proc/meminfo", "rt");
    if (f == NULL) {
	return 0;
    }

    *mem_free = 0;
    *mem_total = 0;
    *swp_free = 0;
    *swp_total = 0;

    while (fgets(buf, BUFSIZ, f)) {
	sscanf(buf, "MemTotal: %d", mem_total);
	sscanf(buf, "MemFree: %d", mem_free);
	sscanf(buf, "SwapTotal: %d", swp_total);
	sscanf(buf, "SwapFree: %d", swp_free);
    }
    fclose(f);

    return 1;
}


int
system_get_max_temp (SENSOR *list, int *temp, int *crit)
{
    SENSOR *s;
    char buf[BUFSIZ];

    int tmp, max = -1;

    *crit = 0;

    list_foreach (s, list) {
	if (s->type == S_ACPI_THERMAL_ZONE &&
	    sensors_read_line(s->filename, BUFSIZ, buf) > 0 &&
	    sscanf(buf, "temperature: %d", &tmp) == 1) {
	    if (100 * tmp > 90 * s->idata) {
		*crit = 1;
	    }
	    if (max < tmp) {
		max = tmp;
	    }
	}
	if (s->type == S_HWMON_CORETEMP &&
	    sensors_read_line(s->filename, BUFSIZ, buf) > 0 &&
	    sscanf(buf, "%d", &tmp) == 1) {
	    if (100 * tmp > 90 * s->idata) {
		*crit = 1;
	    }
	    if (max * 1000 < tmp) {
		max = tmp / 1000;
	    }
	}
    }

    if (max < 0) {
	return 0;
    }

    *temp = max;
    return 1;
}


int
system_get_temperature (SENSOR *list, int num, TEMPSTAT temp[])
{
    SENSOR *s;
    char buf[BUFSIZ];
    int i = 0;

    list_foreach (s, list) {
	if (i >= num) {
	    break;
	}
	if (s->type == S_ACPI_THERMAL_ZONE &&
	    sensors_read_line(s->filename, BUFSIZ, buf) > 0 &&
	    sscanf(buf, "temperature: %d", &temp[i].temp) == 1) {
	    temp[i].max = s->idata;
	    strncpy(temp[i].name, s->name, 7);
	    temp[i].name[7] = '\0';
	    i++;
	}
	if (s->type == S_HWMON_CORETEMP &&
	    sensors_read_line(s->filename, BUFSIZ, buf) > 0 &&
	    sscanf(buf, "%d", &temp[i].temp) == 1) {
	    temp[i].temp /= 1000;
	    temp[i].max = s->idata / 1000;
	    strncpy(temp[i].name, s->name, 7);
	    temp[i].name[7] = '\0';
	    i++;
	}
	if (s->type == S_NVIDIA_SETTINGS_GPUCORETEMP &&
	    sensors_nvidia(s->name, &temp[i].temp) == 0) {
	    temp[i].max = s->idata;
	    strncpy(temp[i].name, s->name, 7);
	    temp[i].name[7] = '\0';
	    i++;
	}
    }

    return i;
}


int
system_get_best_wifi (int *wifi)
{
    FILE *f;
    char buf[BUFSIZ];

    int tmp, max = -1;

    f = fopen("/proc/net/wireless", "rt");
    if (f == NULL) {
	return 0;
    }

    while (fgets(buf, BUFSIZ, f)) {
	if (sscanf(buf, "%*s %*d %d", &tmp) == 1 &&
	    max < tmp) {
	    max = tmp;
	}
    }
    fclose(f);

    if (max < 0) {
	return 0;
    }

    *wifi = max;
    return 1;
}


int
system_get_ac_adapter (SENSOR *list)
{
    SENSOR *s;
    char buf[BUFSIZ];

    list_foreach (s, list) {
	if (s->type == S_ACPI_AC_ADAPTER &&
	    sensors_read_line(s->filename, BUFSIZ, buf) > 0 &&
	    strstr(buf, "on-line")) {
	    return 1;
	}
    }

    return 0;
}


int
system_get_battery (SENSOR *list, int num, BATSTAT *total, BATSTAT batt[])
{
    SENSOR *s;
    char buf[BUFSIZ];

    int n = 0;

    int all_state = 0;
    total->name[0] = '\0';
    total->full_cap = 0;
    total->curr_cap = 0;
    total->rate = 0;

    list_foreach (s, list) {
	if (s->type == S_ACPI_BATTERY) {
	    FILE *f;
	    int present = 0;
	    int full_cap = 0;
	    int curr_cap = 0;
	    int charging = 0;
	    int rate = 0;

	    strcpy(strrchr(s->filename, '/'), "/state");
	    f = fopen(s->filename, "rt");
	    if (f == NULL) {
		continue;
	    }
	    while (fgets(buf, sizeof(buf), f)) {
		if (strstr(buf, "present:") && strstr(buf, "yes")) {
		    present = 1;
		}
		/*if (strstr(buf, "charging state:")) {
		    if (strstr(buf + 15, "charging")) {
			charging = 1;
		    }
		    if (strstr(buf + 15, "discharging")) {
			charging = -1;
		    }
		    if (all_state * charging < 0) {
			printf("cannot charge and discharge at the same time\n");
		    }
		}*/
		sscanf(buf, "remaining capacity: %d", &curr_cap);
		sscanf(buf, "present rate: %d", &rate);
	    }
	    fclose(f);

	    if (!present) {
		continue;
	    }

	    strcpy(strrchr(s->filename, '/'), "/info");
	    f = fopen(s->filename, "rt");
	    if (f == NULL) {
		continue;
	    }
	    while (fgets(buf, sizeof(buf), f)) {
		sscanf(buf, "design capacity: %d", &full_cap);
		sscanf(buf, "last full capacity: %d", &full_cap);
	    }
	    fclose(f);

	    if (!full_cap) {
		continue;
	    }

	    total->full_cap += full_cap;
	    total->curr_cap += curr_cap;
	    total->rate += rate;
	    if (n < num) {
		strncpy(batt[n].name, s->name, sizeof(total->name) - 1);
		batt[n].name[sizeof(total->name) - 1] = '\0';
		batt[n].full_cap = full_cap;
		batt[n].curr_cap = curr_cap;
		batt[n].rate = rate;
	    }
	    all_state |= charging;
	    n++;
	}
    }

    return n;
}


int
system_get_netif (int num, IFSTAT *ifaces)
{
    FILE *f;
    int sock;
    char buf[BUFSIZ];

    IFSTAT *q = ifaces;
    int i = 0;

    /* proc does not expose ethX:Y aliases */
    /* http://www-128.ibm.com/developerworks/aix/library/au-ioctl-socket.html */
    f = fopen("/proc/net/dev", "rt");
    if (f == NULL) {
	return 0;
    }

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
	fclose(f);
	return 0;
    }

    while (fgets(buf, BUFSIZ, f)) {
	char *p;
	struct ifreq ifr;
	struct iwreq iwr;
	struct iw_statistics iws;

	if (i++ < 2) {
	    continue;
	}
	if (q - ifaces >= num) {
	    break;
	}
	p = strrchr(buf, ':');
	if (p == NULL) {
	    continue;
	}
	*p = '\0';
	for (p = buf; isspace(*p); p++) {
	}

	strcpy(ifr.ifr_name, p);
	if (ioctl(sock, SIOCGIFFLAGS, (caddr_t)&ifr) < 0) {
	    continue;
	}
	if (ifr.ifr_flags & IFF_LOOPBACK) {
	    continue;
	}
	if (!(ifr.ifr_flags & IFF_UP)) {
	    continue;
	}

	strncpy(q->name, p, sizeof(q->name));
	q->name[sizeof(q->name) - 1] = '\0';
	q->essid[0] = '\0';
	q->ipv4 = -1;
	q->wlink = -1;

	if (ifr.ifr_flags & IFF_UP) {
	    if (ioctl(sock, SIOCGIFADDR, (caddr_t)&ifr) >= 0) {
		q->ipv4 = ntohl(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr);
	    }
	}

	strncpy(iwr.ifr_name, p, IFNAMSIZ);
	iwr.u.essid.pointer = (caddr_t)&q->essid;
	iwr.u.essid.length = sizeof(q->essid);
	iwr.u.essid.flags = 0;
	if (ioctl(sock, SIOCGIWESSID, &iwr) >= 0) {
	    q->essid[iwr.u.essid.length] = '\0';
	}

	iwr.u.essid.pointer = (caddr_t)&iws;
	iwr.u.essid.length = sizeof(iws);
	iwr.u.essid.flags = 0;
	if (ioctl(sock, SIOCGIWSTATS, &iwr) >= 0) {
	    q->wlink = iws.qual.qual;
	}

	q++;
    }
    close(sock);
    fclose(f);

    return q - ifaces;
}
