MAKE=make
CC=gcc
CFLAGS=-Wall -g
RM=rm -f
X=/usr/X11R6
INCLUDES=-I$(X)/include
LIBPATHS=-L${X}/lib
LIBS=-lXext
PROG=wmswallow
OBJS=wmswallow.o
SH=/bin/sh

default:
	@echo "Type make <platform> to compile."
	@echo "Currently supported platforms are:"
	@echo "	  xfree solaris solaris-gcc"

xfree:
	${MAKE} main

solaris:
	${MAKE} CC=cc CFLAGS="-g -Xa" X=/usr/openwin \
		LIBPATHS="-L/usr/openwin/lib -R/usr/openwin/lib" \
		LIBS="-lXext -lX" main

solaris-gcc:
	${MAKE} X=/usr/openwin \
		LIBPATHS="-L/usr/openwin/lib -R/usr/openwin/lib" \
		LIBS="-lXext -lX" main

main:	${OBJS}
	${CC} ${CFLAGS} $(INCLUDES) ${OBJS} -o ${PROG} ${LIBPATHS} ${LIBS}

tarball:
	${SH} -c "cd ..;tar cpO wmswallow/*.c wmswallow/Makefile \
	 wmswallow/todo wmswallow/LICENCE wmswallow/README \
	wmswallow/README.solaris wmswallow/version.h \
	wmswallow/CHANGELOG wmswallow/INSTALL wmswallow/wmswallow.spec\
	 |compress > wmswallow.tar.Z"

clean:
	${RM} ${PROG} *~ *.o

.c.o:
	${CC} ${CFLAGS} $(INCLUDES) -c $<
