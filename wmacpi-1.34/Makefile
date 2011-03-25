# set options.  pick one, acpi or apm.  comment out the other one.  don't
# uncomment both, bad things will happen :)

OPT	= -O3 -DACPI
#OPT	= -O3 -DAPM

# uncomment this to make wmacpi use less system colors (looks uglier too)
#OPT	+= -DLOW_COLOR

# debugging options (don't bother with these)
#OPT	= -pg -g -DPRO -DACPI

CC	= gcc
CFLAGS	= $(OPT) -Wall -ansi -I/usr/X11R6/include
LDFLAGS = $(OPT) -L/usr/X11R6/lib -lX11 -lXpm -lXext

SRCS	= wmacpi.c libapm.c libacpi.c
OBJS	= wmacpi.o libapm.o libacpi.o

all:	wmacpi

wmacpi:	$(OBJS)

clean:
	rm -f *.o *~ wmacpi trace *.out *.bb *.bbg
