CC     = gcc
CFLAGS = -O2 -Wall
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man/man1
INSTALL = install

# If you need to compile on a Linux, dont change anything. If
# compiling for Solaris, swap the LIBS below...
#
# For Linux (and other?) Machines...
#
LIBS   = -lXpm -lX11 -lXext -lm
#
# For Sun Solaris Machines (I know it compiles on 2.6)...
#
# LIBS   = -lXpm -lX11 -lXext -lsocket

OBJS   = wmSun.o SunRise.o wmgeneral/wmgeneral.o

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $*.o

all:	wmSun.o wmSun

wmSun.o: wmSun_master.xpm wmSun_mask.xbm

wmSun:	$(OBJS)
	$(CC) $(LDFLAGS) -o wmSun $^ $(LIBS)

clean:
	for i in $(OBJS) ; do \
		rm -f $$i; \
	done
	rm -f wmSun

install:: wmSun
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL) wmSun $(DESTDIR)$(BINDIR)
	$(INSTALL) -d $(DESTDIR)$(MANDIR)
	$(INSTALL) -m 644 wmSun.1 $(DESTDIR)$(MANDIR)
