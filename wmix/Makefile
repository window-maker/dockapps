CC		= gcc
CFLAGS		= -O3 -W -Wall
LDFLAGS		= -L/usr/X11R6/lib
OBJECTS		= misc.o mixer-oss.o ui_x.o wmix.o

# where to install this program (also for packaging stuff)
DESTDIR		=
PREFIX		= $(DESTDIR)/usr/X11R6
INSTALL_BIN	= -m 755
INSTALL_DATA	= -m 644

wmix: $(OBJECTS)
	$(CC) -o $@ $(LDFLAGS) $(OBJECTS) -lXpm -lXext -lX11 -lm

clean:
	rm -rf *.o wmix *~

install: wmix
	install $(INSTALL_BIN)	wmix	$(PREFIX)/bin
	install $(INSTALL_DATA)	wmix.1x	$(PREFIX)/man/man1
