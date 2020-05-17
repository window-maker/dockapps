INSTALL = /usr/bin/install
LIBDIR = -L/usr/X11R6/lib
LIBS   = -lXpm -lXext -lX11
OBJS =  wmcore.o \
		wmgeneral.o \
		misc.o \
		list.o


.c.o:
	cc $(CFLAGS) -Wall -c -O3 $< -o $*.o

wmlmon: $(OBJS)
	cc $(CFLAGS) -Wall -o wmcore $^ $(LIBDIR) $(LIBS)

clean::
	for i in $(OBJS) ; do \
		rm -f $$i;\
	done
	rm -f wmcore
install::
	$(INSTALL) -c wmcore $(DESTDIR)/usr/bin/wmcore
uninstall:
	rm -f /usr/bin/wmcore
