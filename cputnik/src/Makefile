
CFLAGS			= -Wall -Os
INCS			= -I/usr/X11R6/include/X11
LIBS			= -L/usr/X11R6/lib -lXpm -lXext -lX11
INSTALL_PREFIX	= /usr/local


all: cputnik.c docklib.o master.h mask.h
	gcc $(CFLAGS) $(INCS) docklib.o cputnik.c -o cputnik $(LIBS)

docklib.o: docklib.c docklib.h
	gcc $(CFLAGS) $(INCS) -c docklib.c

install:
	cp ./cputnik $(INSTALL_PREFIX)/bin

uninstall:
	rm -f $(INSTALL_PREFIX)/bin/cputnik

clean:
	rm -f *.o cputnik *~


