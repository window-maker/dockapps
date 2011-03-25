# $Id: Makefile,v 1.2 2002/08/16 17:22:26 essmann Exp $
#
# Copyright (c) 2001 Bruno Essmann <essmann@users.sourceforge.net>
# All rights reserved.

.SILENT:

XPMINC = /usr/X11R6/include
XPMLIB = /usr/X11R6/lib

INCLUDES = -I/usr/X11R6/include -I/usr/local/include -I/usr/include/X11R6 -I$(XPMINC)
LIBINC = -L/usr/X11R6/lib -L/usr/include/lib -L$(XPMLIB)
LIBS = -lm -lX11 -lXpm -lXext

TARGET = wmpager
OBJECTS = wmpager.o
SOURCE = wmpager.c

all: ${TARGET}

${OBJECTS}: ${SOURCE}
	@echo "*** `pwd`: $@" ; \
	gcc -g -c -Wall ${INCLUDES} ${FLAGS} $< -o $*.o

${TARGET}: ${OBJECTS}
	@echo "*** `pwd`: $@" ; \
	gcc -g -o ${TARGET} ${OBJECTS} ${LIBINC} ${LIBS}

clean:
	@echo "*** `pwd`: $@" ; \
	for i in ${OBJECTS}; do if [ -f $$i ] ; then rm $$i; fi; done ; \
	if [ -f ${TARGET} ] ; then rm ${TARGET}; fi
