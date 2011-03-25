# Copyright (c) 2007 Daniel Borca  All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA


.PHONY: all install clean realclean

prefix ?= /usr/local

CC = gcc
CFLAGS = -Wall -W -pedantic
CFLAGS += -O2

LD = gcc
LDFLAGS = -L/usr/X11R6/lib
LDFLAGS += -s

LDLIBS = -lXpm -lXext -lX11 

SOURCES = \
	dockapp.c \
	linux/sensors.c \
	linux/system.c \
	util.c \
	wmfu.c

OBJECTS = $(SOURCES:.c=.o)

.c.o:
	$(CC) -o $@ $(CFLAGS) -c $<

all: wmfu

wmfu: $(OBJECTS)
	$(LD) -o $@ $(LDFLAGS) $^ $(LDLIBS)

install: all
	install -D wmfu ${prefix}/bin/wmfu
	install -D --mode=644 wmfu.1 ${prefix}/man/man1/wmfu.1
	install -d ${prefix}/share/doc/wmfu
	install --mode=644 README COPYING ${prefix}/share/doc/wmfu

clean:
	-$(RM) $(OBJECTS)

realclean: clean
	-$(RM) wmfu

-include depend
