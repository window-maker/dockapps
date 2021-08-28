CC     = gcc
CFLAGS = -O2 -Wall
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man/man1
INSTALL = install
WMSUN_VERSION = 1.05

# If you need to compile on a Linux, dont change anything. If
# compiling for Solaris, swap the LIBS below...
#
# For Linux (and other?) Machines...
#
LIBS   = -lXpm -lX11 -lXext -lm -ldockapp
#
# For Sun Solaris Machines (I know it compiles on 2.6)...
#
# LIBS   = -lXpm -lX11 -lXext -lsocket

OBJS   = wmsun.o SunRise.o

.c.o:
	$(CC) $(CPPFLAGS) -DWMSUN_VERSION=\"$(WMSUN_VERSION)\" $(CFLAGS) -c $< \
		-o $*.o

all:	wmsun.o wmsun

wmsun.o: wmSun_master.xpm wmSun_mask.xbm

wmsun:	$(OBJS)
	$(CC) $(LDFLAGS) -o wmsun $^ $(LIBS)

clean:
	for i in $(OBJS) ; do \
		rm -f $$i; \
	done
	rm -f wmsun

install:: wmsun
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL) wmsun $(DESTDIR)$(BINDIR)
	$(INSTALL) -d $(DESTDIR)$(MANDIR)
	$(INSTALL) -m 644 wmsun.1 $(DESTDIR)$(MANDIR)
