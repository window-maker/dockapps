LIBS   = -lXpm -lXext -lX11 -lm
CFLAGS = -O2 -Wall
OBJS = wmitime.o wmgeneral/wmgeneral.o
INSTALL = install
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man/man1
DESKTOPDIR = $(PREFIX)/share/applications

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $*.o

wmitime: $(OBJS)
	$(CC) $(LDFLAGS) -o wmitime $^ $(LIBS)

all:: wmtime

clean::
	for i in $(OBJS) ; do \
		rm -f $$i ; \
	done
	rm -f wmitime wmitime.desktop
	rm -f *~

wmitime.desktop:
	sed "s|@BINDIR@|$(BINDIR)|" wmitime.desktop.in > $@

install:: wmitime wmitime.desktop
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL) wmitime $(DESTDIR)$(BINDIR)
	$(INSTALL) -d $(DESTDIR)$(MANDIR)
	$(INSTALL) -m 644 wmitime.1 $(DESTDIR)$(MANDIR)
	$(INSTALL) -d $(DESTDIR)$(DESKTOPDIR)
	$(INSTALL) -m 644 wmitime.desktop $(DESTDIR)$(DESKTOPDIR)
	@echo "wmitime Installation finished..."
