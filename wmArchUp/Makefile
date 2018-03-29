CC=gcc
INSTALL=install
PREFIX=/usr/local
CFLAGS+=-Wall `pkg-config --cflags dockapp`
LIBS=`pkg-config --libs dockapp`

wmarchup: wmarchup.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

wmarchup.o: wmarchup.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

install:
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) wmarchup $(DESTDIR)$(PREFIX)/bin

clean:
	rm -f wmarchup wmarchup.o
