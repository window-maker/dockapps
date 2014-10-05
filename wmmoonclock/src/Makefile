CC     = gcc
# CFLAGS = -O2
INCDIR = -I/usr/X11R6/include/X11
DESTDIR= /usr/X11R6
LIBDIR = -L/usr/X11R6/lib

# for Linux
LIBS   = -lXpm -lX11 -lXext -lm

# for Solaris
# LIBS   = -lXpm -lX11 -lXext -lm -lsocket

OBJS   = wmMoonClock.o CalcEphem.o Moon.o MoonRise.o \
         xutils.o


.c.o:
	$(CC) $(CFLAGS) -D$(shell echo `uname -s`) -c $< -o $*.o $(INCDIR)


all:	wmMoonClock.o wmMoonClock

wmMoonClock.o: wmMoonClock_master.xpm wmMoonClock_mask.xbm CalcEphem.h
wmMoonClock:	$(OBJS)
	$(CC) $(CFLAGS) $(SYSTEM) -o wmMoonClock $^ $(INCDIR) $(LIBDIR) $(LIBS)

clean:
	for i in $(OBJS) ; do \
		rm -f $$i; \
	done
	rm -f wmMoonClock

install:: wmMoonClock
	install -d -m 0755 $(DESTDIR)/usr/bin $(DESTDIR)/usr/share/man/man1
	install    -m 0755 wmMoonClock $(DESTDIR)/usr/bin
	install    -m 0644 wmMoonClock.1 $(DESTDIR)/usr/share/man/man1

