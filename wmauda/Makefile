CC	?= gcc
CFLAGS	?= -g -pipe

PREFIX	?= /usr/local

INSTALL_DIR	:= $(PREFIX)/bin
PIXMAP_DIR	:= $(PREFIX)/share/pixmaps
MANPAGE_DIR	:= $(PREFIX)/share/man/man1

CFLAGS 	+= $(shell pkg-config audclient --cflags) $(shell pkg-config dbus-glib-1 --cflags) $(shell pkg-config gtk+-2.0 --cflags) -DPIXMAP_DIR="\"$(PIXMAP_DIR)\""

LIBS 	:= $(shell pkg-config audclient --libs) $(shell pkg-config dbus-glib-1 --libs) $(shell pkg-config gtk+-2.0 --libs) -lX11

OBJS 	= wmauda.o
HEADERS = dock-master.xpm

wmauda:	$(OBJS) $(HEADERS)
	$(CC) -o wmauda $(OBJS) $(CFLAGS) $(LIBS)

all:	wmauda

clean:	
	rm -f *.o wmauda

install: all
	install -d $(DESTDIR)$(INSTALL_DIR)
	install -d $(DESTDIR)$(PIXMAP_DIR)
	install -d $(DESTDIR)$(MANPAGE_DIR)
	install -m 0755 wmauda $(DESTDIR)$(INSTALL_DIR)
	install -m 0644 wmauda.xpm $(DESTDIR)$(PIXMAP_DIR)
	install -m 0644 wmauda.1 $(DESTDIR)$(MANPAGE_DIR)
