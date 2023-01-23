CC=gcc
INSTALL=install
PREFIX=/usr/local
CFLAGS+=-Wall -Wextra -O3 `pkg-config --cflags dockapp`
LIBS=`pkg-config --libs dockapp x11`

wmarchup: wmarchup.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

wmarchup.o: wmarchup.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

install:
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) wmarchup $(DESTDIR)$(PREFIX)/bin
	$(INSTALL) arch_update.sh $(DESTDIR)$(PREFIX)/bin

clean:
	rm -f wmarchup wmarchup.o
