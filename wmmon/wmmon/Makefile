LIBDIR = -L/usr/X11R6/lib -L/usr/local/lib
LIBS   = -lXpm -lXext -lX11 -ldockapp
OBJS =  wmmon.o

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
