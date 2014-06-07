CC		= gcc
CFLAGS		= -O3 -W -Wall
LDFLAGS		= -L/usr/X11R6/lib
LIBS		= -lXpm -lXext -lX11 -lm
OBJECTS		= misc.o config.o mixer-oss.o ui_x.o mmkeys.o wmix.o

# where to install this program (also for packaging stuff)
PREFIX		= /usr/local
INSTALL_BIN	= -m 755
INSTALL_DATA	= -m 644

wmix: $(OBJECTS)
	$(CC) -o $@ $(LDFLAGS) $(OBJECTS) $(LIBS)

clean:
	rm -rf *.o wmix *~

install: wmix
	install $(INSTALL_BIN)	wmix	$(PREFIX)/bin
	install $(INSTALL_DATA)	wmix.1x	$(PREFIX)/man/man1
