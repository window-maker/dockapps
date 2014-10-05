/* hci.h -- Hardware Configuration Interface
 *
 * Copyright (c) 1998,1999  Jonathan A. Buzzard (jonathan@buzzard.org.uk)
 *
 * $Log: hci.h,v $
 * Revision 1.2  2004/03/27 12:59:15  noberasco
 * see CL
 *
 * Revision 1.1  2003/09/18 16:38:27  noberasco
 * overhaul III
 *
 * Revision 1.1.1.1  2003/06/05 09:34:03  noberasco
 * Initial import.
 *
 * Revision 1.4  2002/01/27 13:23:25  jab
 * added some more HCI function calls to the list
 *
 * Revision 1.3  2001/10/05 13:06:26  jab
 * updates for kernel module checked in
 *
 * Revision 1.2  1999/12/17 12:19:46  jab
 * added function prototypes for HciFunction and HciFnStatus
 * added lots of enum types for various settings
 *
 * Revision 1.1  1999/03/11 20:29:11  jab
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef HCI_H
#define HCI_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LINUX_TOSHIBA_H
#define LINUX_TOSHIBA_H
#include<linux/toshiba.h>
#endif


enum {
	HCI_GET             = 0xfe00,
	HCI_SET             = 0xff00
};


enum {
	HCI_BACKLIGHT        = 0x0002,
	HCI_AC_ADAPTOR       = 0x0003,
	HCI_FAN              = 0x0004,
	HCI_SOFTWARE_SUSPEND = 0x0010,
	HCI_FLAT_PANEL       = 0x0011,
	HCI_SELECT_STATUS    = 0x0014,
	HCI_SYSTEM_EVENT     = 0x0016,
	HCI_FIR_STATUS       = 0x001b,
	HCI_VIDEO_OUT        = 0x001c,
	HCI_HOTKEY_EVENT     = 0x001e,
	HCI_UNUSED_MEMORY    = 0x0021,
	HCI_LOCK_STATUS      = 0x0022,
	HCI_BOOT_DEVICE      = 0x0026,
	HCI_OWNERSTRING      = 0x0029,
	HCI_BRIGHTNESS_LEVEL = 0x002a,
	HCI_HIBERNATION_INFO = 0x002d,
	HCI_HIBERNATION_LBA  = 0x002e,
	HCI_CPU_SPEED        = 0x0032
};


/*
 * the different states the various modes can be set to
 */

enum {
	HCI_DISABLE         = 0x0000,
	HCI_ENABLE          = 0x0001
};

enum {
	HCI_640_480         = 0x00,
	HCI_800_600         = 0x01,
	HCI_1024_768        = 0x02,
	HCI_1024_600        = 0x03,
	HCI_800_480         = 0x04
};

enum {
	HCI_STN_MONO        = 0x00,
	HCI_STN_COLOUR      = 0x01,
	HCI_9BIT_TFT        = 0x02,
	HCI_12BIT_TFT       = 0x03,
	HCI_18BIT_TFT       = 0x04,
	HCI_24BIT_TFT       = 0x05
};

enum {
	HCI_INTERNAL        = 0x0000,
	HCI_EXTERNAL        = 0x0001,
	HCI_SIMULTANEOUS    = 0x0002
};

enum {
	HCI_BIOS_SIZE       = 0x0000,
	HCI_MEMORY_SIZE     = 0x0001,
	HCI_VRAM_SIZE       = 0x0002
};

enum {
	HCI_BUILT_IN        = 0x0000,
	HCI_SELECT_INT      = 0x0001,
	HCI_SELECT_DOCK     = 0x0002,
	HCI_5INCH_DOCK      = 0x0003
};

enum {
	HCI_LOCKED          = 0x0000,
	HCI_UNLOCKED        = 0x0001
};

enum {
	HCI_NOTHING         = 0x0000,
	HCI_FLOPPY          = 0x0001,
	HCI_ATAPI           = 0x0002,
	HCI_IDE             = 0x0003,
	HCI_BATTERY         = 0x0004
};


/*
 * HCI error codes
 */
enum {
	HCI_SUCCESS         = 0x00,
	HCI_FAILURE         = 0x01,
	HCI_NOT_SUPPORTED   = 0x80,
	HCI_INPUT_ERROR     = 0x83,
	HCI_WRITE_PROTECTED = 0x84,
	HCI_FIFO_EMPTY      = 0x8c
};


/*
 * function prototypes
 */
int HciFunction(SMMRegisters *reg);
int HciGetBiosVersion(void);
int HciGetMachineID(int *id);
int HciGetLCDPanelType(int *resolution, int *type);

#ifdef __cplusplus
}
#endif

#endif
