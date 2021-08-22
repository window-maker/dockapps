LIBDIR = -L/usr/X11R6/lib
LIBS   = -lXpm -lXext -lX11
OBJS =  wmisdnmon.o \
		../wmgeneral/wmgeneral.o \
		../wmgeneral/misc.o \
		../wmgeneral/list.o


.c.o:
	cc -c -O2 -Wall $< -o $*.o

wmmon: $(OBJS)
	cc -o wmisdnmon $^ $(LIBDIR) $(LIBS)

install:
	cp wmisdnmon /usr/local/bin/
	cp ring.wav /usr/share/

uninstall:
	rm /usr/local/bin/wmisdnmon
	rm /usr/share/ring.wav

clean::
	for i in $(OBJS) ; do \
		rm $$i;\
	done
	rm wmisdnmon
