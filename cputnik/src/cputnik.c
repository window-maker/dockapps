
/*
 * Cputnik - a simple cpu and memory monitor
 *
 * Copyright (C) 2002-2005 pasp and sill
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


#include "cputnik.h"


FILE *fp_stat, *fp_loadavg, *fp_memory;
int cmd_lmb, cmd_rmb, update_period;
int show_memory, free_memory;
char command_lmb[PATH_MAX], command_rmb[PATH_MAX], *ProgName;
stat_dev cpu_device;

/*----------------------------------------------------------------------*/

void get_mem_statistics(int *free)
{
long long int m_total, m_free, m_cached;
char temp[BUFFER_SIZE];

    fp_memory = freopen("/proc/meminfo", "r", fp_memory);

    while(fscanf(fp_memory, "%s", temp)!=EOF) {

        if(!strncmp(temp,"MemTotal:", 9))
           fscanf(fp_memory, "%Ld", &m_total);

        if(!strncmp(temp,"MemFree:", 8))
           fscanf(fp_memory, "%Ld", &m_free);

        if(!strncmp(temp,"Cached:", 7))
           fscanf(fp_memory, "%Ld", &m_cached);
    }

    *free = (int)(((float)(m_total - m_free - m_cached) / m_total) * 100.0);
}

/*----------------------------------------------------------------------*/

void draw_memory_meter(void)
{
    free_memory = free_memory * (METER_WIDTH / 100.0);
    if (free_memory > METER_WIDTH) free_memory = METER_WIDTH;
    dcl_copy_xpm_area(0, 64, METER_WIDTH, 5, 26, 12);
    dcl_copy_xpm_area(32, 64, free_memory, 5, 26, 12);
}

/*----------------------------------------------------------------------*/

void get_cpu_statistics(char *devname, long *is, long *ds, long *idle)
{
char temp[BUFFER_SIZE], *p, *tokens = " \t\n";
int i;
float f;

    *is = *ds = *idle = 0;

    if (!strncmp(devname, "cpu", 3)) {

        fseek(fp_stat, 0, SEEK_SET);

        while (fgets(temp, 128, fp_stat)) {

            if (strstr(temp, "cpu")) {
                p = strtok(temp, tokens);
                /* 1..3, 4 == idle, we don't want idle! */
                for (i=0; i<3; i++) {
                    p = strtok(NULL, tokens);
                    *ds += atol(p);
                }

                p = strtok(NULL, tokens);
                *idle = atol(p);
            }

        }

        fp_loadavg = freopen("/proc/loadavg", "r", fp_loadavg);
        fscanf(fp_loadavg, "%f", &f);
        *is = (long) (100 * f);

    }
}

/*----------------------------------------------------------------------*/

void draw_stats(int *his, int num, int size, int x_left, int y_bottom)
{
int pixels_per_byte, j, k, *p, d;

    pixels_per_byte = 100;
    p = his;

    for (j=0; j<num; j++) {
        if (p[0] > pixels_per_byte)
            pixels_per_byte += 100;
        p++;
    }

    p = his;

    for (k=0; k<num; k++) {
        d = (1.0 * p[0] / pixels_per_byte) * size;

        for (j=0; j<size; j++) {

            if (j < d - 3)
                dcl_copy_xpm_area(0, 71, 1, 1, k+x_left, y_bottom-j);
            else if (j < d)
                dcl_copy_xpm_area(1, 71, 1, 1, k+x_left, y_bottom-j);
            else
                dcl_copy_xpm_area(2, 71, 1, 1, k+x_left, y_bottom-j);
        }
        p++;
    }

    /* horizontal line */
    for (j = pixels_per_byte-100; j > 0; j-=100) {
        for (k=0; k<num; k++) {
            d = (40.0 / pixels_per_byte) * j;
            dcl_copy_xpm_area(3, 71, 1, 1, k+x_left, y_bottom-d);
        }
    }

}

/*----------------------------------------------------------------------*/

void update_stat_cpu(stat_dev *st)
{
long k, istat, idle;

    get_cpu_statistics(st->name, &k, &istat, &idle);

    st->rt_idle = idle - st->idlelast;
    st->idlelast = idle;

    st->rt_stat = istat - st->statlast;
    st->statlast = istat;

    st->his[V_WIDTH] += k;
    st->hisaddcnt++;
}

/*----------------------------------------------------------------------*/

void cputnik_routine(int argc, char **argv)
{
XEvent Event;
int but_stat = -1, xfd = 0;
long start_time, current_time, next_time, istat, idle, k;
unsigned long i, j;
fd_set inputs;
struct timeval timeout;

    fp_memory = fopen("/proc/meminfo", "r");
    fp_loadavg = fopen("/proc/loadavg", "r");
    fp_stat = fopen("/proc/stat", "r");

    for (j=0; j<V_WIDTH+1; j++)
        cpu_device.his[j] = 0;

    cpu_device.hisaddcnt = 0;
    dcl_strcpy(cpu_device.name, CPU_NAME, CPU_NAME_LEN);

    timeout.tv_sec = 0;
    timeout.tv_usec = 250000;

    dcl_open_x_window(argc, argv, cputnik_master_xpm, cputnik_mask_bits, MASK_WIDTH, MASK_HEIGHT);

    /* add mouse region */
    dcl_add_mouse_region(0, 4, 4, 59, 59);

    start_time = time(0);
    next_time = start_time + update_period;

    get_cpu_statistics(cpu_device.name, &k, &istat, &idle);
    cpu_device.statlast = istat;
    cpu_device.idlelast = idle;

    if (show_memory) {

        dcl_draw_string(6,  5, "cpu", FONT_NORMAL, 3);
        dcl_draw_string(6, 12, "mem", FONT_NORMAL, 3);
        dcl_copy_xpm_area(5, 57, 54, 1, 5, 18);
        get_mem_statistics(&free_memory);
        draw_memory_meter();
        draw_stats(cpu_device.his, V_WIDTH, V_HEIGHT_MEM, 5, 55);

    } else {

        dcl_draw_string(6, 5, "cpu", FONT_LARGE, 3);
        dcl_copy_xpm_area(5, 57, 54, 1, 5, 13);
        draw_stats(cpu_device.his, V_WIDTH, V_HEIGHT, 5, 55);

    }

    while(1) {

        current_time = time(0);

        waitpid(0, NULL, WNOHANG);

        update_stat_cpu(&cpu_device);

        /* cpu meter */
        dcl_copy_xpm_area(0, 64, 32, 7-(show_memory*2), 26, 5);

        j = (cpu_device.rt_stat + cpu_device.rt_idle);
        if (j != 0) j = (cpu_device.rt_stat * 100) / j;
        j = j * (METER_WIDTH / 100.0);
        if (j > METER_WIDTH) j = METER_WIDTH;

        dcl_copy_xpm_area(32, 64, j, 7-(show_memory*2), 26, 5);

        if (current_time >= next_time) {
            next_time += update_period;

            if (cpu_device.his[V_WIDTH])
                cpu_device.his[V_WIDTH] /= cpu_device.hisaddcnt;

            for (j=1; j<V_WIDTH+1; j++)
                cpu_device.his[j-1] = cpu_device.his[j];

        if (show_memory) {

            get_mem_statistics(&free_memory);
            draw_memory_meter();
            draw_stats(cpu_device.his, V_WIDTH, V_HEIGHT_MEM, 5, 55);

        } else
            draw_stats(cpu_device.his, V_WIDTH, V_HEIGHT, 5, 55);

            cpu_device.his[V_WIDTH] = 0;
            cpu_device.hisaddcnt = 0;

        }

        xfd = ConnectionNumber(display);

        FD_ZERO(&inputs);
        FD_SET(xfd, &inputs);

        switch(select(FD_SETSIZE, &inputs, NULL, NULL, &timeout)) {

            case 0:
                timeout.tv_sec = 0;
                timeout.tv_usec = 250000;
                dcl_redraw_window();
                break;

            case -1:
                break;

            default:
                break;
        }

        while(FD_ISSET(xfd, &inputs) && XPending(display)) {
            XNextEvent(display, &Event);
            switch (Event.type) {
            case Expose:
                dcl_redraw_window();
                break;
            case DestroyNotify:
                XCloseDisplay(display);
                exit(0);
                break;
            case ButtonPress:
                but_stat = dcl_check_mouse_region(Event.xbutton.x, Event.xbutton.y);
                break;
            case ButtonRelease:
                i = dcl_check_mouse_region(Event.xbutton.x, Event.xbutton.y);

                if(!i && Event.xbutton.button == LMB && cmd_lmb)
                    dcl_execute_command(command_lmb, 0);
                else if(!i && Event.xbutton.button == RMB && cmd_rmb)
                    dcl_execute_command(command_rmb, 0);
                break;
            }
        }

    }
}

/*----------------------------------------------------------------------*/

void cputnik_write_prefs(void)
{

    if (dcl_prefs_openfile (dcl_getfilename_config (".clay", "cputnik.rc"), P_WRITE)) {

        dcl_prefs_put_int ("update_period", update_period);
        dcl_prefs_put_int ("show_memory", show_memory);
        dcl_prefs_put_string ("command_lmb", command_lmb);
        dcl_prefs_put_string ("command_rmb", command_rmb);

    }

    dcl_prefs_closefile ();
}

/*----------------------------------------------------------------------*/

void cputnik_read_prefs(void)
{

    if (dcl_prefs_openfile (dcl_getfilename_config(".clay", "cputnik.rc"), P_READ)) {

        update_period = dcl_prefs_get_int("update_period");
        show_memory = dcl_prefs_get_int("show_memory") % 2;
        dcl_strcpy(command_lmb, dcl_prefs_get_string("command_lmb"), BUFFER_SIZE);
        dcl_strcpy(command_rmb, dcl_prefs_get_string("command_rmb"), BUFFER_SIZE);

        cmd_lmb = cmd_rmb = 0;

        if(strlen(command_lmb)) cmd_lmb = 1;
        if(strlen(command_rmb)) cmd_rmb = 1;

        dcl_prefs_closefile ();

    } else {

        update_period = 2;
        show_memory = 1;
        strcpy(command_lmb, "gnome-system-monitor");
        strcpy(command_rmb, "xkill");

        cputnik_write_prefs ();

    }
}

/*----------------------------------------------------------------------*/

int main(int argc, char **argv)
{
    cputnik_read_prefs();
    cputnik_routine(argc, argv);

    return 0;
}

