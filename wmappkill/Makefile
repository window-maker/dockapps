GTOP_LIBS = -lgtop -lgtop_sysdeps -lgtop_common -lXau
GCCOPTS = -O2 -Wall -g
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



