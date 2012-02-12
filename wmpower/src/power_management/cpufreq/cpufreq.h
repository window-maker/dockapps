/***************************************************************************
                          cpufreq.h  -  description
                             -------------------
    begin                : Feb 10 2003
    copyright            : (C) 2003,2004,2005 by Noberasco Michele
    e-mail               : s4t4n@gentoo.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.              *
 *                                                                         *
 ***************************************************************************/


#define CPUFREQ_GOV_PERFORMANCE "performance"
#define CPUFREQ_GOV_ONDEMAND    "ondemand"


/* Checks wether this machine supports CPU frequency scaling */
int check_cpufreq(void);

/* Set cpufreq governor */
int cpufreq_set_governor(char *governor);

/* Get cpufreq governor of CPU #n, where 1<=n<=MAX, where MAX is
 * the number of CPUs you have in your system
 */
char *cpufreq_get_governor(int cpu);
