# Makefile for ${name}
# <mfischer@umr.edu>
# based on wmWeather - see ../Credits for more information

CC     = gcc
CFLAGS = #-O2 -g #-Wall # I think this option is idiotic
INCDIR = -I/usr/X11R6/include/X11 -I/usr/X11R6/include
DESTDIR= /usr/X11R6
LIBDIR = -L/usr/X11R6/lib
# for linux
LIBS   = -lXpm -lX11 -lXext
# for Solaris
# LIBS   = -lXpm -lX11 -lXext -lsocket

OBJS   = jDockApp.o xutils.o jprintf.o

name=wmjmailapp

all:	${name}
	${name}

jDockApp.o: jDockApp/jDockApp_main.xpm jDockApp/jDockApp.c
	cd $*; make $*.o

xutils.o: xutils/xutils.c
	cd $*; make $*.o

jprintf.o: jDockApp/jprintf.c
	cd jDockApp; make jprintf.o


${name}: $(OBJS) app.c
	$(CC) $(CFLAGS) $(SYSTEM) -o ${name} $^ $(INCDIR) $(LIBDIR) $(LIBS)

clean:
	rm -f *.o ${name}

pack: clean
	cd ../..; tar -zcf ${name}.tgz ${name}
