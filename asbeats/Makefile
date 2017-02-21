GCC=gcc
RM=rm -f
LIBPATHS=-L/usr/X11R6/lib
LIBS=-lXpm -lXext -lX11
PROG=asbeats
OBJS=asbeats.o

all:	${OBJS}
	${GCC} ${OBJS} -o ${PROG} ${LIBPATHS} ${LIBS}

clean:
	${RM} ${PROG} *~ *.o
