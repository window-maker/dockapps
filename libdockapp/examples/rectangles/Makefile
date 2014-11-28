CC=gcc
INSTALL=install
PREFIX=/usr/local
CFLAGS+=-Wall `pkg-config --cflags dockapp`
LIBS=`pkg-config --libs dockapp`

rectangles: rectangles.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

rectangles.o: rectangles.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

install:
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) rectangles $(DESTDIR)$(PREFIX)/bin

clean:
	rm -f rectangles rectangles.o
