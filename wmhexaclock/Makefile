# wmHexaClock Makefile by Bjoern Witt <ng@bjoernw.de>
#
# Copyright (C) 2006 Bjoern Witt
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program (see the file COPYING); if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA  02111-1307, USA
#
# As a base for this program I used wmDockApp0.1 from Todd Goyen.
# Thanks a lot!


CC = gcc
CFLAGS = -O2 -Wall -g
BIN = wmHexaClock
SRC = ./src/main.c ./src/wmgeneral.c
INSPATH = /usr/local/bin/
SOURCEPATH = ./
X_LIBS =  -L/usr/X11R6/lib
wmHexaClock_LDADD = -lX11 -lXpm -lXext

all : $(MAINSRC)
	$(CC) $(CFLAGS) -o $(BIN) $(SRC) $(X_LIBS) $(wmHexaClock_LDADD)
	strip $(SOURCEPATH)$(BIN)

install :
	install -c -s -m 755 $(BIN) $(INSPATH)

clean :
	rm -f $(SOURCEPATH)$(BIN)

uninstall :
	rm -f $(INSPATH)wmHexaClock
