LIBDIR = -L/usr/X11R6/lib
LIBS   = -lXpm -lXext -lX11 -lm
OBJS =	wmtime.o \
		../wmgeneral/wmgeneral.o \
		../wmgeneral/misc.o \
		../wmgeneral/list.o
XPMS = wmtime-master.xpm wmtime-mask.xbm

CC = gcc
CFLAGS = -O2

.c.o:
	$(CC) $(CFLAGS) -c -Wall $< -o $*.o

wmtime: $(OBJS) $(XPMS)
	$(CC) $(CFLAGS) -o wmtime $(OBJS) $(LIBDIR) $(LIBS)

clean::
	for i in $(OBJS) ; do \
		rm $$i; \
	done
	rm wmtime
