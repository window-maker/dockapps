CC=gcc
INSTALL=install
PREFIX=/usr/local
CFLAGS+=-Wall `pkg-config --cflags dockapp`
LIBS=`pkg-config --libs dockapp`

basic: basic.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

basic.o: basic.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

install:
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) basic $(DESTDIR)$(PREFIX)/bin

clean:
	rm -f basic basic.o
