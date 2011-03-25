/* sci.h -- System Configuration Interface
 *
 * Copyright (c) 1998  Jonathan A. Buzzard (jonathan@buzzard.org.uk)
 *
 * WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
 *
 *   This code is covered by the GNU GPL and you are free to make any
 *   changes you wish to it under the terms of the license. However the
 *   code has the potential to render your computer and/or someone else's
 *   unuseable. Unless you truely understand what is going on, I urge you
 *   not to make any modifications and use it as it stands.
 *
 * $Log: sci.h,v $
 * Revision 1.2  2004/03/27 12:59:15  noberasco
 * see CL
 *
 * Revision 1.1  2003/09/18 16:38:27  noberasco
 * overhaul III
 *
 * Revision 1.1.1.1  2003/06/05 09:34:03  noberasco
 * Initial import.
 *
 * Revision 1.11  2001/10/05 13:08:37  jab
 * checked in changes for kernel module
 *
 * Revision 1.10  1999/12/18 14:33:44  jab
 * removed prototype for SciGetModel
 *
 * Revision 1.9  1999/12/04 13:41:23  jab
 * modified SCI_DATE macro to exclude the year and added SCI_FULLDATE macro
 *
 * Revision 1.8  1999/03/11 20:23:18  jab
 * added macros to manipulate date type
 * updated some of the enumerations and added a few more
 *
 * Revision 1.7  1999/03/06 16:47:12  jab
 * removed declarations for BiosVersion and MachineID functions
 *
 * Revision 1.6  1998/09/07 18:15:50  jab
 * added prototype for new model fuction
 * added a structure for the system configuration interface registers
 *
 * Revision 1.5  1998/08/23 12:17:36  jab
 * added the SCI_BATTERY/SCI_MAINS values
 *
 * Revision 1.4  1998/08/19 08:42:29  jab
 * added extern "C" declaration in case anyone uses this with C++
 * fixed the declaration of SCI_SUCSSES/SCI_FAILURE
 * other miscellaneous tidy ups
 *
 * Revision 1.3  1998/08/06 08:25:36  jab
 * changed defines to enums and prepended everything with sci_
 *
 * Revision 1.2  1998/08/04 08:07:46  jab
 * added some extra information from Japanesse Libretto effort
 *
 * Revision 1.1  1998/05/23 08:08:17  jab
 * Initial revision
 *
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
 *
 */

#ifndef SCI_H
#define SCI_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LINUX_TOSHIBA_H
#define LINUX_TOSHIBA_H
#include<linux/toshiba.h>
#endif 


/*
 * the different modes that can be changed
 */
enum {
	SCI_POWER_UP        = 0x0100,
	SCI_BATTERY_SAVE    = 0x0101,
	SCI_PROCESSING      = 0x0102,
	SCI_SLEEP_MODE      = 0x0104,
	SCI_DISPLAY_AUTO    = 0x0105,
	SCI_HDD_AUTO_OFF    = 0x0106,
	SCI_CPU_CACHE       = 0x0108,
	SCI_SPEAKER_VOLUME  = 0x0109,
	SCI_SYSTEM_BEEP     = 0x010a,
	SCI_BATTERY_ALARM   = 0x010b,
	SCI_PANEL_ALARM     = 0x010c,
	SCI_PANEL_POWER     = 0x010d,
	SCI_ALARM_DATE      = 0x010e,
	SCI_ALARM_TIME      = 0x010f,
	SCI_ALARM_POWER     = 0x010f,
	SCI_SYSTEM_AUTO     = 0x0110,
	SCI_BATTERY_PERCENT = 0x0111,
	SCI_BATTERY_TIME    = 0x0112,
	SCI_LCD_BRIGHTNESS  = 0x0115,
	SCI_LCD_MAXBRIGHT   = 0x011b,
	SCI_BOOT_METHOD     = 0x011d,
	SCI_2ND_BATTERY     = 0x011e,
	SCI_CACHE_POLICY    = 0x011f,
	SCI_COOLING_METHOD  = 0x0122,
	SCI_STANDBY_TIME    = 0x0125,
	SCI_HIBERNATION     = 0x012d,
	SCI_LCD_BACKLIGHT   = 0x0305,
	SCI_DISPLAY_STRETCH = 0x0308,
	SCI_PARALLEL_PORT   = 0x0501,
	SCI_POINTING_DEVICE = 0x0505,
	SCI_INFRARED_PORT   = 0x0508,
	SCI_PASSWORD_MODE   = 0x0600,
	SCI_PASSWORD_CHECK  = 0x0601,
	SCI_PASSWORD        = 0x0602,
	SCI_PASSWORD_VERIFY = 0x0603,
	SCI_PASSWORD_LEVEL  = 0x0604
};


/*
 * the different states the various modes can be set to
 */

enum {	
	SCI_BOOT            = 0x0000,
	SCI_RESUME          = 0x0001,
	SCI_HIBERNATE       = 0x0002,
	SCI_QUICK_HIBERNATE = 0x0003
};

enum {
	SCI_USER_SETTINGS   = 0x0000,
	SCI_LOW_POWER       = 0x0001,
	SCI_FULL_POWER      = 0x0002,
	SCI_LONG_LIFE       = 0x0001,
	SCI_NORMAL_LIFE     = 0x0002,
	SCI_ECONOMY         = 0x0002,
	SCI_FULL_LIFE       = 0x0003
};

enum {
	SCI_LOW             = 0x0000,
	SCI_HIGH            = 0x0001
};

enum {
	SCI_OFF             = 0x0000,
	SCI_ON              = 0x0001
};

enum {
	SCI_DISABLED        = 0x0000,
	SCI_ENABLED         = 0x0001
};

enum {
	SCI_ALARM_ENABLED   = 0x0000,
	SCI_ALARM_DISABLED  = 0x0001
};

enum {
	SCI_TIME_DISABLED   = 0x0001,  /* will disable relevant settings */
	SCI_TIME_00         = 0x0002,
	SCI_TIME_01         = 0x0004,
	SCI_TIME_03         = 0x0008,
	SCI_TIME_05         = 0x0010,
	SCI_TIME_10         = 0x0020,
	SCI_TIME_15         = 0x0040,
	SCI_TIME_20         = 0x0080,
	SCI_TIME_25         = 0x0100,
	SCI_TIME_30         = 0x0200,
	SCI_TIME_35         = 0x0400,
	SCI_TIME_40         = 0x0800,
	SCI_TIME_45         = 0x1000,
	SCI_TIME_50         = 0x2000,
	SCI_TIME_55         = 0x4000,
	SCI_TIME_60         = 0x8000
};
	
enum {
	SCI_FD_HD           = 0x0000,
	SCI_HD_FD           = 0x0001
};

enum {
	SCI_VOLUME_OFF      = 0x0000,
	SCI_VOLUME_LOW      = 0x0001,
	SCI_VOLUME_MEDIUM   = 0x0002,
	SCI_VOLUME_HIGH     = 0x0003
};

enum {
	SCI_BRIGHT          = 0x0000,
	SCI_SEMI_BRIGHT     = 0x0001,
	SCI_SUPER_BRIGHT    = 0x0002
};

enum {
	SCI_BACK_DARK       = 0x0000,
	SCI_BACK_DIM        = 0x0001,
	SCI_BACK_SEMI       = 0x0002,
	SCI_BACK_BRIGHT     = 0x0003
};

enum {
	SCI_PERFORMANCE     = 0x0000,
	SCI_QUIET           = 0x0001
};

enum {
	SCI_PARALLEL_ECP    = 0x0010,
	SCI_PARALLEL_SPP    = 0x0020, /* Libretto 20x/30x only ? */
	SCI_PARALLEL_PS2    = 0x0040
};

enum {
	SCI_AUTO_SELECT     = 0x0000,
	SCI_SIMULATANEOUS   = 0x0001
};

enum {
	SCI_IRDA_1_0        = 0x0001, /* These don't work on IrDA 1.1 laptops */
	SCI_ASK             = 0x0002
};

enum {
	SCI_NOT_REGISTERED  = 0x0000,
	SCI_REGISTERED      = 0x0001
};

enum {
	SCI_USER_PASSWORD   = 0x0000,
	SCI_SUPER_PASSWORD  = 0x0001
};

enum {
	SCI_BATTERY = 0x0003,
	SCI_MAINS   = 0x0004
};


/*
 * SCI error codes
 */
enum {
	SCI_SUCCESS         = 0x00,
	SCI_FAILURE         = 0x01,
	SCI_NOT_SUPPORTED   = 0x80,
	SCI_ALREADY_OPEN    = 0x81,
	SCI_NOT_OPENED      = 0x82,
	SCI_INPUT_ERROR     = 0x83,
	SCI_WRITE_PROTECTED = 0x84,
	SCI_NOT_PRESENT     = 0x86,
	SCI_NOT_READY       = 0x8c,
	SCI_DEVICE_ERROR    = 0x8d,
	SCI_NOT_INSTALLED   = 0x8e
};


/*
 * macro's to manipulate the time and date data types
 */
#define SCI_TIME(h,m) ((m & 0x3f)<<1) | ((h & 0x1f)<<7)
#define SCI_HOUR(t)   (t & 0x7fc0)>>7 
#define SCI_MINUTE(t) (t & 0x7e)>>1
#define SCI_TIME_ON(t) (t & 0x01)
#define SCI_DATE(m,d) ((m & 0xf)<<6) | ((d & 0x1f)<<1)
#define SCI_FULLDATE(y,m,d) (((y-1990) & 0x1f)<<10) | ((m & 0xf)<<6) | ((d & 0x1f)<<1)
#define SCI_YEAR(d) 1990+((d & 0x1fc00)>>10)
#define SCI_MONTH(d) (d & 0x3c0)>>6
#define SCI_DAY(d) (d & 0x3e)>>1
#define SCI_DATE_EVERYDAY(d) (d & 0x01)

/*
 * function prototypes
 */
int SciSupportCheck(int *version);
int SciOpenInterface(void);
int SciCloseInterface(void);
int SciGet(SMMRegisters *reg);
int SciSet(SMMRegisters *reg);
int SciACPower(void);

#ifdef __cplusplus
}
#endif

#endif
