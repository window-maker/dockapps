INSTALL = /usr/bin/install -m 755
PREFIX = /usr/local

VERSION = 1.02
CFLAGS = -O3 -Wall -DVERSION=\"$(VERSION)\" ${EXTRA}
CC = gcc
OBJS = cnslock.o kleds.o

LIBS = -lX11 -ldockapp

all: cnslock

cnslock.o: cnslock.c
	$(CC) $(CFLAGS) -c cnslock.c -o cnslock.o

kleds.o: kleds.c include/kleds.h
	$(CC) -Wall -I/usr/X11R6/include -c kleds.c -o kleds.o

cnslock: $(OBJS)
	$(CC) -o cnslock $(OBJS) $(LIBS)

clean:
	rm -rf cnslock *.o *~ .xvpics

install : cnslock
	$(INSTALL) cnslock $(PREFIX)/bin
