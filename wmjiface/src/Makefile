prefix=/home/jettero/bin
#owner=-o root
#group=-g root

# Makefile for ${name}
# <mfischer@umr.edu>
# based on wmWeather - see ../Credits for more information

CC     = gcc
CFLAGS = #-O2 -g #-Wall # I think this option is idiotic
INCDIR = -I/usr/X11R6/include/X11 -I/usr/X11R6/include #-I/usr/i386-glibc20-linux/include
DESTDIR= /usr/X11R6
LIBDIR = -L/usr/X11R6/lib
# for linux
LIBS   = -lXpm -lX11 -lXext
# for Solaris
# LIBS   = -lXpm -lX11 -lXext -lsocket

OBJS   = jDockApp.o xutils.o jprintf.o

name=wmjiface
version=1.7cb

CFLAGS=-DVERSION="\"${version}\"" -DBDATE="\"${DATESTR}\""

all:
	make real_all "DATESTR=`date '+%B %d, %Y'`"

real_all:  ${name}

jDockApp.o: jDockApp/jDockApp_main.xpm jDockApp/jDockApp.c
	cd $*; make $*.o

xutils.o: xutils/xutils.c
	cd $*; make $*.o

jprintf.o: jDockApp/jprintf.c
	cd jDockApp; make jprintf.o


${name}: $(OBJS) ${name}.c
	$(CC) $(CFLAGS) $(SYSTEM) -o ${name} $^ $(INCDIR) $(LIBDIR) $(LIBS)

install: all
	install ${owner} ${group} -s -m 755 ./${name}  ${prefix}/${name}
	install ${owner} ${group}    -m 755 ./ifacechk ${prefix}/ifacechk
	make clean;

clean:
	rm -f *.o ${name}

wpack: pack
	cd ../../; mv ${name}-${version}.tgz /home/jettero/www/dockapps

pack: clean
	cd ../..; tar -zcf ${name}-${version}.tgz ${name}
