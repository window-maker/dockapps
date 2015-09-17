CC		= gcc
CFLAGS		= -std=gnu99 -O3 -W -Wall `pkg-config --cflags alsa`
LDFLAGS		= -L/usr/X11R6/lib
LIBS		= -lXpm -lXext -lX11 -lm `pkg-config --libs alsa`
OBJECTS		= misc.o config.o mixer-alsa.o mixer-oss.o ui_x.o mmkeys.o wmix.o

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
