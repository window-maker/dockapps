LIBDIR = -L/usr/X11R6/lib
LIBS   = -lXpm -lXext -lX11
OBJS =	wmifs.o \
		../wmgeneral/wmgeneral.o \
		../wmgeneral/misc.o \
		../wmgeneral/list.o


.c.o:
	cc -c -O2 -Wall $< -o $*.o

wmifs: $(OBJS)
	cc -o wmifs $^ -lXext $(LIBDIR) $(LIBS)

all:: wmifs

clean::
	for i in $(OBJS) ; do \
		rm $$i ; \
	done
	rm wmifs

install::
	cp -f wmifs /usr/local/bin/
	chmod 755 /usr/local/bin/wmifs
	chown root:root /usr/local/bin/wmifs
	cp sample.wmifsrc /etc/wmifsrc
	chown root:root /etc/wmifsrc
	chmod 644 /etc/wmifsrc
	cp -f sample.wmifsrc $(HOME)/.wmifsrc
	echo "WMiFS-1.3beta installation finished..."
	echo " "
	echo "have fun! ;-)"
