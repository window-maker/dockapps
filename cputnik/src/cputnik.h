
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


#include "docklib.h"

#include "master.h"
#include "mask.h"

#define		BUFFER_SIZE		512
#define		CPU_NAME		"cpu0"
#define		CPU_NAME_LEN	5

#define		V_WIDTH			54
#define		V_HEIGHT		41
#define		V_HEIGHT_MEM	36
#define     METER_WIDTH     32

typedef struct {
	char	name[CPU_NAME_LEN];
	int		his[V_WIDTH+1];
	int		hisaddcnt;
	long	rt_stat;
	long	statlast;
	long	rt_idle;
	long	idlelast;
} stat_dev;

