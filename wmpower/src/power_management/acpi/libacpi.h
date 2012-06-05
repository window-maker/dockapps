/***************************************************************************
                          libacpi.h  -  description
                             -------------------
    begin                : Feb 10 2003
    copyright            : (C) 2003 by Noberasco Michele
    e-mail               : 2001s098@educ.disi.unige.it
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.              *
 *                                                                         *
 ***************************************************************************/

 /***************************************************************************
        Originally written by Costantino Pistagna for his wmacpimon
 ***************************************************************************/

#define MAXBATT 8
#define MAXFANS 8

typedef enum
{
  POWER,       /* on AC, Battery charged  */
  DISCHARGING, /* on Battery, Discharging */
  CHARGING,    /* on AC, Charging         */
  UNKNOW       /* unknown                 */
}
Charging;

typedef struct
{
	/* /proc stuff                                           */
  int present;    /* 1 if present, 0 if no battery         */
  Charging state; /* charging state enum                   */
  int prate;      /* present rate                          */
  int rcapacity;  /* rameining capacity                    */
  int pvoltage;   /* present voltage                       */
  /* not present in /proc                                  */
  int rtime;			/* remaining time                        */
  int percentage; /* battery percentage (-1 if no battery) */
}
ACPIstate;

typedef struct
{
  int present;                 /* 1 if present, 0 if no battery      */
  int design_capacity;         /* design capacity                    */
  int last_full_capacity;      /* last_full_capacity                 */
  int battery_technology;      /* 1 non-rechargeable, 0 rechargeable */
  int design_voltage;          /* design voltage                     */
  int design_capacity_warning; /* design capacity warning (critical) */
  int design_capacity_low;     /* design capacity low (low level)    */
}
ACPIinfo;

char battery_type;

typedef struct
{
  int state;	/* 1 if online, 0 if offline */
}
ACADstate;

/* number of batteries detected */
int batt_count;

int  check_acpi (void);
void read_acad_state (ACADstate *acadstate);
void read_acpi_info (ACPIinfo *acpiinfo, int battery);
void read_acpi_state (ACPIstate *acpistate, ACPIinfo *acpiinfo, int battery);
void acpi_get_temperature(int *temperature, int *temp_is_celsius);
int  acpi_get_fan_status(void);
