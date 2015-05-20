LIBS   = -lXpm -lXext -lX11
OBJS  = wmppp.o \
		wmgeneral/wmgeneral.o \
		wmgeneral/misc.o \
		wmgeneral/list.o

INSTALL = install
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL) -m 644

PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
LIBDIR = $(PREFIX)/lib/wmppp.app
SYSCONFDIR = /etc
MANDIR = $(PREFIX)/share/man/man1

all:: wmppp getmodemspeed

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

wmppp: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

getmodemspeed: getmodemspeed.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean::
	for i in $(OBJS) ; do \
		rm -f $$i; \
	done
	rm -f getmodemspeed.o
	rm -f wmppp getmodemspeed

install::
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL_PROGRAM) wmppp $(DESTDIR)$(BINDIR)
	$(INSTALL) -d $(DESTDIR)$(LIBDIR)
	$(INSTALL_PROGRAM) getmodemspeed $(DESTDIR)$(LIBDIR)
	$(INSTALL) -d $(DESTDIR)$(SYSCONFDIR)
	$(INSTALL_DATA) user.wmppprc $(DESTDIR)$(SYSCONFDIR)/wmppprc
	$(INSTALL) -d $(DESTDIR)$(MANDIR)
	$(INSTALL_DATA) wmppp.1 $(DESTDIR)$(MANDIR)
	echo "WMPPP installation finished."
