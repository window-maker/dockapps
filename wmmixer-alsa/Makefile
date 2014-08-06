CC=gcc
CFLAGS= -I/usr/X11R6/include -g
OBJS=wmmixer-alsa.o mixer.o

all:	${OBJS}
	${CC} -L/usr/X11R6/lib -g -lm -lXpm -lXext -lX11 -lasound ${OBJS} -o wmmixer-alsa

clean:
	rm -rf *.o *~ wmmixer-alsa
