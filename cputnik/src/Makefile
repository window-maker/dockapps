
CFLAGS			= -Wall -Os
INCS			= -I/usr/X11R6/include/X11
LIBS			= -L/usr/X11R6/lib -lXpm -lXext -lX11
prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
MKDIR_P = mkdir -p
INSTALL = install
INSTALL_PROGRAM = $(INSTALL)


all: cputnik.c docklib.o master.h mask.h
	gcc $(CFLAGS) $(INCS) docklib.o cputnik.c -o cputnik $(LIBS)

docklib.o: docklib.c docklib.h
	gcc $(CFLAGS) $(INCS) -c docklib.c

install:
	$(MKDIR_P) $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) cputnik $(DESTDIR)$(bindir)

uninstall:
	rm -f $(DESTDIR)$(bindir)/cputnik

clean:
	rm -f *.o cputnik *~


