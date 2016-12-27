
CFLAGS			= -Wall -Os
INCS			= -I/usr/X11R6/include/X11
LIBS			= -L/usr/X11R6/lib -lXpm -lX11 -lXext -lm
INSTALL_PREFIX	= /usr/local


wmtunlo: docklib.o wmtunlo.c mask.h master.h
	gcc $(CFLAGS) $(INCS) docklib.o wmtunlo.c -o wmtunlo $(LIBS)

docklib.o: docklib.c docklib.h
	gcc $(CFLAGS) $(INCS) -c docklib.c

install:
	cp ./wmtunlo $(INSTALL_PREFIX)/bin

uninstall:
	rm -f $(INSTALL_PREFIX)/bin/wmtunlo

clean:
	rm -f *~ *.o wmtunlo


