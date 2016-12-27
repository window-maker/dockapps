INSTALL = /usr/bin/install -m 755
PREFIX = /usr/local

VERSION = 1.02
CFLAGS = -O3 -Wall `gtk-config --cflags` -DVERSION=$(VERSION) ${EXTRA}
CC = gcc
OBJS = cnslock.o kleds.o applet.o

LIBS = `gtk-config --libs | sed "s/-lgtk//g"`

all: cnslock

cnslock.o: cnslock.c include/cnslock.h include/defines.h
	$(CC) $(CFLAGS) -c cnslock.c -o cnslock.o

applet.o: applet.c include/applet.h include/defines.h
	gcc $(CFLAGS) -c applet.c -o applet.o

kleds.o: kleds.c include/kleds.h include/defines.h
	$(CC) -Wall -I/usr/X11R6/include -c kleds.c -o kleds.o

cnslock: $(OBJS)
	$(CC) -o cnslock $(OBJS) $(LIBS)

clean:
	rm -rf cnslock *.o *~ .xvpics

install : cnslock
	$(INSTALL) cnslock $(PREFIX)/bin
