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

name=xutils

${name}.o:
	$(CC) $(CFLAGS) -D$(shell echo `uname -s`) -c ${name}.c -o ${name}.o \
              $(INCDIR)
	mv ${name}.o ..

clean:
	rm -f *.o
