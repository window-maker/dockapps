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


.PHONY: all clean realclean

CC = gcc
CFLAGS = -Wall -W -pedantic
CFLAGS += -O2

LD = gcc
LDFLAGS = -L/usr/X11R6/lib
LDFLAGS += -s

LDLIBS = -lXpm -lXext -lX11 

SOURCES = \
	xhookey.c \
	xhooktst.c

OBJECTS = $(SOURCES:.c=.o)

.c.o:
	$(CC) -o $@ $(CFLAGS) -c $<

all: xhookey

xhookey: $(OBJECTS)
	$(LD) -o $@ $(LDFLAGS) $^ $(LDLIBS)

clean:
	-$(RM) $(OBJECTS)

realclean: clean
	-$(RM) xhookey

-include depend
