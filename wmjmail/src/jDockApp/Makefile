# Makefile for ${name}
# <mfischer@umr.edu>
# based on wmWeather - see ../Credits for more information

CC     = gcc
CFLAGS = #-O2 -g #-Wall # I think this option is idiotic
INCDIR = -I/usr/X11R6/include/X11 -I/usr/X11R6/include -I/usr/i386-glibc20-linux/include
DESTDIR= /usr/X11R6
LIBDIR = -L/usr/X11R6/lib
# for linux
LIBS   = -lXpm -lX11 -lXext
# for Solaris
# LIBS   = -lXpm -lX11 -lXext -lsocket

.c.o:
	$(CC) $(CFLAGS) -D$(shell echo `uname -s`) -c $< -o $*.o \
              $(INCDIR)
	mv $*.o ..

all: jprintf.o jDockApp.o

jprintf.o:  jprintf.c jDockApp.h
jDockApp.o: jDockApp.c jDockApp.h jDockApp_main.xpm jDockApp_mask.xbm

clean:
	rm -f ${name}.o
