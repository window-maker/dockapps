CC	= gcc
#OPT	= -pg -g -DPRO
OPT	= -O3 -fomit-frame-pointer
CFLAGS	= $(OPT) -Wall -I/usr/X11R6/include
LDFLAGS = $(OPT) -L/usr/X11R6/lib -lX11 -lXpm -lXext

SRCS	= wmfemon.c
OBJS	= wmfemon.o

all:	clean wmfemon

wmfemon:	$(OBJS)

clean:
	rm -f *.o *~ wmfemon trace *.out *.bb *.bbg
