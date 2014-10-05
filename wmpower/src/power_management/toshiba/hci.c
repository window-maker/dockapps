/* hci.c -- Hardware Configuration Interface
 *
 * Copyright (c) 1998-2000  Jonathan A. Buzzard (jonathan@buzzard.org.uk)
 *
 * $Log: hci.c,v $
 * Revision 1.2  2003/11/06 12:48:08  noberasco
 * Added a new machine id...
 *
 * Revision 1.1  2003/09/18 16:38:27  noberasco
 * overhaul III
 *
 * Revision 1.1.1.1  2003/06/05 09:34:03  noberasco
 * Initial import.
 *
 * Revision 1.5  2002/01/27 13:22:57  jab
 * updated list of machine ID's
 *
 * Revision 1.4  2001/10/05 13:04:43  jab
 * checked in change to using kernel module
 *
 * Revision 1.3  1999/12/12 11:33:39  jab
 * changed assembler to save registers, should make the programs stabler
 * slightly fudged addition to GetMachineID to get SCTTable ID's
 *
 * Revision 1.2  1999/08/15 10:43:28  jab
 * removed the HciGet and HciSet and replaced with HciFunction
 *
 * Revision 1.1  1999/03/11 20:27:06  jab
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

static const char rcsid[]="$Id: hci.c,v 1.2 2003/11/06 12:48:08 noberasco Exp $";

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/ioctl.h>

#include"hci.h"


int HciFunction(SMMRegisters *regs)
{
	int fd;

	if ((fd=open(TOSH_DEVICE, O_RDWR))<0)
		return HCI_FAILURE;

	if (access(TOSH_PROC, R_OK)) {
		close(fd);
		return HCI_FAILURE;
	}

	if (ioctl(fd, TOSH_SMM, regs)<0) {
		close(fd);
		return HCI_FAILURE;
	}

	close(fd);

	return (int) (regs->eax & 0xff00)>>8;
}


/*
 * Return the BIOS version of the laptop
 */
int HciGetBiosVersion(void)
{
	FILE *str;
	int major,minor;
	char buffer[64];

	if (access(TOSH_PROC, R_OK))
		return -1;

	/* open /proc/toshiba for reading */

	if (!(str = fopen(TOSH_PROC, "r")))
		return -1;

	/* scan in the information */

	fgets(buffer, sizeof(buffer)-1, str);
	fclose(str);
	buffer[sizeof(buffer)-1] = '\0';
	sscanf(buffer, "%*s %*x %*d.%*d %d.%d %*x %*x\n", &major, &minor);

	/* return the information */

	return (major*0x100)+minor;
}


/*
 * Get the Toshiba machine identification number
 */
int HciGetMachineID(int *id)
{
	FILE *str;
	char buffer[64];

	if (access(TOSH_PROC, R_OK))
		return HCI_FAILURE;

	/* open /proc/toshiba for reading */

	if (!(str = fopen(TOSH_PROC, "r")))
		return HCI_FAILURE;

	/* scan in the information */

	fgets(buffer, sizeof(buffer)-1, str);
	fclose(str);
	buffer[sizeof(buffer)-1] = '\0';
	sscanf(buffer, "%*s %x %*d.%*d %*d.%*d %*x %*x\n", (unsigned int *) id);

	return HCI_SUCCESS;
}


/*
 * Return the LCD Panel type
 */
int HciGetLCDPanelType(int *resolution, int *type)
{
	SMMRegisters regs;

	regs.eax = HCI_GET;
	regs.ebx = HCI_FLAT_PANEL;
	HciFunction(&regs);

	*resolution = (regs.ecx & 0xff00)>>8;
	*type = regs.ecx & 0xff;

	return HCI_SUCCESS;
}
