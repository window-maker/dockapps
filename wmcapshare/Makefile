CC = gcc

bindir = /usr/local/bin

OPENOBEX_CONFIG = /usr/local/bin/openobex-config
GLIB_CONFIG = pkg-config glib-2.0

all: wmcapshare

wmcapshare: wmcapshare.o obex_io.o obex_put_common.o
	$(CC) `$(OPENOBEX_CONFIG) --libs` `$(GLIB_CONFIG) --libs` -o wmcapshare -ldockapp *.o

wmcapshare.o: wmcapshare.h wmcapshare.c version.h
	$(CC) `$(GLIB_CONFIG) --cflags` -c wmcapshare.c

obex_io.o: obex_io.c obex_io.h
	$(CC) `$(GLIB_CONFIG) --cflags` -c obex_io.c

obex_put_common.o: obex_put_common.c obex_put_common.h
	$(CC) `$(GLIB_CONFIG) --cflags` -c obex_put_common.c

clean:
	rm -f *.o wmcapshare

install: wmcapshare
	cp wmcapshare $(bindir)/wmcapshare
