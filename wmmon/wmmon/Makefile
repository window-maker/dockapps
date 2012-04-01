LIBDIR = -L/usr/X11R6/lib
LIBS   = -lXpm -lXext -lX11
OBJS =  wmmon.o \
		../wmgeneral/wmgeneral.o \
		../wmgeneral/misc.o \
		../wmgeneral/list.o

CFLAGS = -O2
CC = cc


.c.o:
	$(CC) $(CFLAGS) -c -Wall $< -o $*.o

wmmon: $(OBJS)
	$(CC) $(LDFLAGS) -o wmmon $^ $(LIBDIR) $(LIBS)

debug: CFLAGS = -g3 
debug: wmmon

clean::
	for i in $(OBJS) ; do \
		rm -f $$i;\
	done
	rm -f wmmon
