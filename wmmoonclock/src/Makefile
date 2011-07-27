CC     = gcc
CFLAGS = -O2
INCDIR = -I/usr/X11R6/include/X11
DESTDIR= /usr/X11R6
LIBDIR = -L/usr/X11R6/lib

# for Linux
LIBS   = -lXpm -lX11 -lXext

# for Solaris
# LIBS   = -lXpm -lX11 -lXext -lsocket

OBJS   = wmMoonClock.o CalcEphem.o Moon.o MoonRise.o \
         xutils.o


.c.o:
	$(CC) $(CFLAGS) -D$(shell echo `uname -s`) -c $< -o $*.o $(INCDIR)


all:	wmMoonClock.o wmMoonClock

wmMoonClock.o: wmMoonClock_master.xpm wmMoonClock_mask.xbm CalcEphem.h
wmMoonClock:	$(OBJS) 
	$(CC) $(COPTS) $(SYSTEM) -lm -o wmMoonClock $^ $(INCDIR) $(LIBDIR) $(LIBS)

clean:
	for i in $(OBJS) ; do \
		rm -f $$i; \
	done
	rm -f wmMoonClock

install:: wmMoonClock
	install -s -m 0755 wmMoonClock $(DESTDIR)/bin
	install    -m 0644 wmMoonClock.1 $(DESTDIR)/man/man1 

