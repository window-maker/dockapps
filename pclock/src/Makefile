BINDIR = /usr/X11R6/bin

PROG = pclock
OBJS = Main.o Graphics.o getopt.o getopt1.o

DEFAULT_XPM = round-3d.xpm

INCDIR = -I/usr/X11R6/include -I.
LIBDIR = -L/usr/X11R6/lib
CC = gcc
CFLAGS = -O -s -Wall
LIBS = -lXpm -lXext -lX11 -lm

###############################################################################

all: $(PROG)

install: all
	install -s -m 111 -o bin -g bin $(PROG) $(BINDIR)

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

