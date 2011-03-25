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


#ifndef SYSTEM_GET_H_included
#define SYSTEM_GET_H_included

typedef struct {
    int used;
    int old_idle;
    int old_total;
} CPUSTAT;

typedef struct {
    char name[32];	/* IFNAMSIZ */
    char essid[32];	/* IW_ESSID_MAX_SIZE */
    int ipv4;
    int wlink;
} IFSTAT;

typedef struct {
    char name[8];
    int temp;
    int max;
} TEMPSTAT;

typedef struct {
    char name[8];
    int full_cap;
    int curr_cap;
    int rate;
} BATSTAT;

int system_get_uptime (int *days, int *hours, int *mins);
int system_get_cpu_load (int num, CPUSTAT *avg, CPUSTAT load[]);
int system_get_cpu_speed (int num, int speed[]);
int system_get_cpu_gov (int cpu, int max, char *out);
int system_get_mem_stat (int *mem_free, int *mem_total, int *swp_free, int *swp_total);
int system_get_max_temp (SENSOR *list, int *temp, int *crit);
int system_get_temperature (SENSOR *list, int num, TEMPSTAT temp[]);
int system_get_best_wifi (int *wifi);
int system_get_ac_adapter (SENSOR *list);
int system_get_battery (SENSOR *list, int num, BATSTAT *total, BATSTAT batt[]);
int system_get_netif (int num, IFSTAT *ifaces);

#endif
