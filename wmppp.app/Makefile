LIBDIR = -L/usr/X11R6/lib
LIBS   = -lXpm -lXext -lX11
CFLAGS += -O2
OBJS  = wmppp.o \
		wmgeneral/wmgeneral.o \
		wmgeneral/misc.o \
		wmgeneral/list.o

.c.o:
	cc -g -c $(CPPFLAGS) $(CFLAGS) -Wall $< -o $*.o

wmppp: $(OBJS)
	cc -o wmppp $(LDFLAGS) $^ -lXext $(LIBDIR) $(LIBS)

all:: wmppp getmodemspeed

clean::
	for i in $(OBJS) ; do \
		rm -f $$i; \
	done
	rm -f wmppp getmodemspeed

install::

	cp -f wmppp /usr/local/bin/
	chmod 755 /usr/local/bin/wmppp
	chown root:root /usr/local/bin/wmppp
	cp getmodemspeed /etc/ppp/
	chmod 755 /etc/ppp/getmodemspeed
	chown root.root /etc/ppp/getmodemspeed
	cp -f user.wmppprc /etc/wmppprc
	chmod 644 /etc/wmppprc
	chown root.root /etc/wmppprc
	cp -f user.wmppprc $(HOME)/.wmppprc
	cp -f wmppp.1 /usr/local/share/man/man1
	echo "WMPPP installation finished."
