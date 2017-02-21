CC     = gcc
CFLAGS = -Wall -O2
INCDIR = -I/usr/X11R6/include/X11 -I/usr/X11R6/include
PREFIX = $(shell cat ../PREFIX)
DESTDIR= $(PREFIX)
LIBDIR = -L/usr/X11R6/lib

# for Linux
LIBS   = -lXpm -lX11 -lXext

# for Solaris
# LIBS   = -lXpm -lX11 -lXext -lsocket -lnsl

OBJS   = wmCalClock.o \
         xutils.o


.c.o:
	$(CC) $(CFLAGS) -D$(shell echo `uname -s`) -DKB_PIXMAP_PREFIX="\"$(PREFIX)/share/wmCalClockKbd/\"" -c $< -o $*.o $(INCDIR)


all:	wmCalClock.o wmCalClockKbd

wmCalClock.o: wmCalClock_master.xpm wmCalClock_mask.xbm
wmCalClockKbd:	$(OBJS)
	$(CC) $(COPTS) $(SYSTEM) -o wmCalClockKbd $^ $(INCDIR) $(LIBDIR) $(LIBS)

clean:
	for i in $(OBJS) ; do \
		rm -f $$i; \
	done
	rm -f wmCalClockKbd

install: wmCalClockKbd
	install -c -s -m 0755 wmCalClockKbd $(DESTDIR)/bin
	install -c -m 0644 wmCalClockKbd.1 $(DESTDIR)/share/man/man1

