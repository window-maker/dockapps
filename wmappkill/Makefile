GTOP_LIBS = -lgtop-2.0 `pkg-config glib-2.0 --libs`
GCCOPTS = -O2 -Wall -g -I/usr/include/libgtop-2.0 \
	`pkg-config glib-2.0 --cflags`
CFLAGS=  -Wall
LDLIBS = -lX11 -lXpm -lXext
LDFLAGS= -L/usr/X/lib -L/usr/X11R6/lib -L/usr/X11/lib -L/usr/local/lib/X11


all : wmAppKill

wmAppKill : wmAppKill.c
	gcc ${GCCOPTS}  wmAppKill.c -o wmAppKill  ${GTOP_LIBS} ${LDFLAGS} ${LDLIBS}

install: wmAppKill
	 install -m 755 wmAppKill /usr/bin/wmAppKill

clean:
	$(RM) *.o core tst *~ \#*



