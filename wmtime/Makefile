LIBS   = -lXpm -lXext -lX11 -lm
OBJS =	wmtime.o \
		wmgeneral/wmgeneral.o \
		wmgeneral/misc.o \
		wmgeneral/list.o
XPMS = wmtime-master.xpm wmtime-mask.xbm
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man/man1
INSTALL = install

CC = gcc
CFLAGS = -O2 -Wall

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $*.o

wmtime: $(OBJS) $(XPMS)
	$(CC) $(LDFLAGS) -o wmtime $(OBJS) $(LIBS)

clean::
	for i in $(OBJS) ; do \
		rm -f $$i; \
	done
	rm -f wmtime

install:: wmtime
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL) wmtime $(DESTDIR)$(BINDIR)
	$(INSTALL) -d $(DESTDIR)$(MANDIR)
	$(INSTALL) -m 644 wmtime.1 $(DESTDIR)$(MANDIR)
