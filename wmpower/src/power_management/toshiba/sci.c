/* sci.c -- System Configuration Interface
 *
 * Copyright (c) 1998-2000  Jonathan A. Buzzard (jonathan@buzzard.org.uk)
 *   
 * $Log: sci.c,v $
 * Revision 1.1  2003/09/18 16:38:27  noberasco
 * overhaul III
 *
 * Revision 1.1.1.1  2003/06/05 09:34:03  noberasco
 * Initial import.
 *
 * Revision 1.14  2001/10/05 13:07:40  jab
 * checked in changes for kernel module
 *
 * Revision 1.13  1999/12/18 14:32:24  jab
 * fixed some bugs in the assembler
 * removed the SciGetModel function, not needed now with working MachineID
 *
 * Revision 1.12  1999/11/17 16:00:42  jab
 * changed assembler to manually save registers, hopefully this should
 * make the programs more stable
 *
 * Revision 1.11  1999/07/25 14:39:49  jab
 * fixed bugs in SciOpenInterface and SciSetPassword
 * updated email address
 *
 * Revision 1.10  1999/03/11 20:22:14  jab
 * changed get and set routines to use SciRegisters
 *
 * Revision 1.9  1999/03/06 16:46:30  jab
 * moved the BiosVersion and MachineID functions to hci.c
 *
 * Revision 1.8  1998/09/07 18:23:36  jab
 * removed SciGetMachineID2 as no longer required
 * added a routine to extract the model string from the BIOS
 *
 * Revision 1.7  1998/09/04 18:14:31  jab
 * fixed the compile warning about rcsid
 *
 * Revision 1.6  1998/08/24 18:05:04  jab
 * implemented the SciGetBiosVersion function
 *
 * Revision 1.5  1998/08/23 12:16:45  jab
 * fixed the SciACPower function so it now works
 *
 * Revision 1.4  1998/08/19 08:45:10  jab
 * changed GetMachineId to return SCI_SUCCESS/SCI_FAILURE
 *
 * Revision 1.3  1998/08/18 16:54:54  jab
 * fixes to SciGetMachineId curtesy of Patrick Reyonlds <patrick@cs.virgina.edu>
 *
 * Revision 1.2  1998/08/06 08:27:14  jab
 * prepended all functions with Sci
 *
 * Revision 1.1  1998/05/23 08:08:53  jab
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

static const char rcsid[]="$Id: sci.c,v 1.1 2003/09/18 16:38:27 noberasco Exp $";

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/ioctl.h>

#include"sci.h"


/*
 * Is this a supported Machine? (ie. is it a Toshiba)
 */
int SciSupportCheck(int *version)
{
	SMMRegisters regs;
	int fd;

	if ((fd=open(TOSH_DEVICE, O_RDWR))<0)
		return SCI_FAILURE;

	if (access(TOSH_PROC, R_OK)) {
		close(fd);
		return SCI_FAILURE;
	}

	regs.eax = 0xf0f0;
	regs.ebx = 0x0000;
	regs.ecx = 0x0000;
	regs.edx = 0x0000;

	if (ioctl(fd, TOSH_SMM, &regs)<0) {
		close(fd);
		return SCI_FAILURE;
	}
	close(fd);

	*version = (int) regs.edx;

	return (int) (regs.eax & 0xff00)>>8;
}


/*
 * Open an interface to the Toshiba hardware.
 *
 *   Note: Set and Get will not work unless an interface has been opened.
 */
int SciOpenInterface(void)
{
	SMMRegisters regs;
	int fd;

	if ((fd=open(TOSH_DEVICE, O_RDWR ))<0)
		return SCI_FAILURE;

	regs.eax = 0xf1f1;
	regs.ebx = 0x0000;
	regs.ecx = 0x0000;

	if (ioctl(fd, TOSH_SMM, &regs)<0) {
		close(fd);
		return SCI_FAILURE;
	}
	close(fd);

	return (int) (regs.eax & 0xff00)>>8;
}


/*
 * Close any open interface to the hardware
 */
int SciCloseInterface(void)
{
	SMMRegisters regs;
	int fd;

	if ((fd=open(TOSH_DEVICE, O_RDWR ))<0)
		return SCI_FAILURE;

	regs.eax = 0xf2f2;
	regs.ebx = 0x0000;
	regs.ecx = 0x0000;

	if (ioctl(fd, TOSH_SMM, &regs)<0) {
		close(fd);
		return SCI_FAILURE;
	}
	close(fd);

	return (int) (regs.eax & 0xff00)>>8;
}


/*
 * Get the setting of a given mode of the laptop
 */
int SciGet(SMMRegisters *regs)
{
	int fd;

	if ((fd=open(TOSH_DEVICE, O_RDWR ))<0)
		return SCI_FAILURE;

	regs->eax = 0xf3f3;

	if (ioctl(fd, TOSH_SMM, regs)<0) {
		close(fd);
		return SCI_FAILURE;
	}
	close(fd);

	return (int) (regs->eax & 0xff00)>>8;
}


/*
 * Set the setting of a given mode of the laptop
 */
int SciSet(SMMRegisters *regs)
{
	int fd;

	if ((fd=open(TOSH_DEVICE, O_RDWR ))<0)
		return SCI_FAILURE;

	regs->eax = 0xf4f4;

	if (ioctl(fd, TOSH_SMM, regs)<0) {
		close(fd);
		return SCI_FAILURE;
	}
	close(fd);

	return (int) (regs->eax & 0xff00)>>8;
}


/*
 * Get the status of the AC Power on a Toshiba laptop.
 */
int SciACPower(void)
{
	SMMRegisters regs;
	int fd;

	if (access(TOSH_PROC, R_OK))
		return SCI_FAILURE;

	if ((fd=open(TOSH_DEVICE, O_RDWR))<0)
		return SCI_FAILURE;

	regs.eax = 0xfefe;
	regs.ebx = 0x0003;
	regs.ecx = 0x0000;
	regs.edx = 0x0000;

	if (ioctl(fd, TOSH_SMM, &regs)<0) {
		close(fd);
		return 0;
	}
	close(fd);

	return (int) regs.ecx;
}
