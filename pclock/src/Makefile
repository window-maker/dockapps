PROG = pclock
OBJS = Main.o Graphics.o getopt.o getopt1.o

DEFAULT_XPM = round-3d.xpm

INCDIR = -I/usr/X11R6/include -I.
LIBDIR = -L/usr/X11R6/lib
CC = gcc
CFLAGS = -O -s -Wall
LIBS = -lXpm -lXext -lX11 -lm

prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
MKDIR_P = mkdir -p
INSTALL = install
INSTALL_PROGRAM = $(INSTALL)

###############################################################################

all: $(PROG)

install: all
	$(MKDIR_P) $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) $(PROG) $(DESTDIR)$(bindir)/$(PROG)

clean:
	rm -f *.o Makefile.bak Default.xpm *~ #*

clobber: clean
	rm -f $(PROG)

###############################################################################

Default.xpm:
	ln -s ../XPM/$(DEFAULT_XPM) $@

Graphics.o: PClock.h Default.xpm

Main.o: PClock.h Defaults.h Version.h

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBDIR) $(LIBS)

###############################################################################

.c.o:
	$(CC) $(CFLAGS) $(INCDIR) -c $< -o $@

