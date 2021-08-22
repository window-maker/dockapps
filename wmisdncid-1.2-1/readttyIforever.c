
/***********************************************************************\

readttyIforever - workaround to get all incoming calls displayed
                  part of wmisdncid distribution

Jun 25th 2000  Release 1.1
Copyright (C) 1999  Carl Eike Hofmeister <wmisdncid@carl-eike-hofmeister.de>
This software comes with ABSOLUTELY NO WARRANTY
This software is free software, and you are welcome to redistribute it
under certain conditions
See the COPYING file for details.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

\***********************************************************************/

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <term.h>

int
main (int argc, char *argv[])
{
  int isdndf = -1;

  if (argc != 3)
  {
    printf ("Usage:\n%s <isdn-device, e.g. /dev/ttyI4> <MSN>\n", argv[0]);
    return (2);
  }

  isdndf = open (argv[1], O_RDWR | O_NDELAY, 0);

  if (isdndf >= 0)
  {
    write (isdndf, "AT\rAT&e", 7);
    write (isdndf, argv[2], strlen (argv[2]));
    write (isdndf, "\rATS18=7\r", 9);
  }

  while (isdndf >= 0)
  {
    int numchar;

    ioctl (isdndf, FIONREAD, &numchar);
    if (numchar > 0)
      tcflush (isdndf, TCIFLUSH);

    sleep (1);
  }

  return (1);
}
