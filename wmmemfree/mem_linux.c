/*
 *  mem_linux.c - get memory status
 *
 *  Copyright (C) 2003 Draghicioiu Mihai <misuceldestept@go.ro>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>

long long int mem_total, mem_used, mem_free, mem_shared, mem_buffers, mem_cached;
long long int swp_total, swp_used, swp_free;

void mem_getfree()
{
 FILE *file;

 file = fopen("/proc/meminfo", "r");
 if(!file)
 {
  perror("/proc/meminfo");
  exit(1);
 }
 fscanf(file, "MemTotal: %lld kB MemFree: %lld kB MemAvailable: %*d kB "
	"Buffers: %lld kB Cached: %lld kB",
	&mem_total, &mem_free, &mem_buffers, &mem_cached);
 for (int i = 0; i < 10; i++) {
	 while (fgetc(file) != '\n') {}
 }
 fscanf(file, "SwapTotal: %lld kB SwapFree: %lld kB", &swp_total, &swp_free);
 fclose(file);
 mem_used = mem_total - mem_free;
 swp_used = swp_total - swp_free;
}
