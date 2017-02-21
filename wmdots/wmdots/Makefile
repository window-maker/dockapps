LIBDIR = -L/usr/X11R6/lib
LIBS   = -lXpm -lXext -lX11
OBJS =	wmdots.o ../wmgeneral/wmgeneral.o

.c.o:
	cc -c -g -O2 -Wall $< -o $*.o

wmifs: $(OBJS)
	cc -o wmdots $^ -lXext $(LIBDIR) $(LIBS)

all:: wmdots

clean::
	for i in $(OBJS) ; do \
		rm $$i ; \
	done
	rm wmdots

install::
	cp -f wmdots /usr/local/bin/
	chmod 755 /usr/local/bin/wmdots
	chown root:root /usr/local/bin/wmdots

