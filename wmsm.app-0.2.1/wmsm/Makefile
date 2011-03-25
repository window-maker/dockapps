CC	= gcc
CFLAGS	= -Wall -O3
LIBDIR 	= -L/usr/X11R6/lib
LIBS   	= -lXpm -lXext -lX11
OBJS 	= wmsm.o ../wmgeneral/wmgeneral.o ../wmgeneral/misc.o ../wmgeneral/list.o
PREFIX	= /usr/X11R6/bin
INSTALL	= /usr/bin/install

wmsm: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBDIR) $(LIBS)

install: wmsm
	$(INSTALL) -m 755 $< $(PREFIX)

clean:
	rm -f $(OBJS)
	rm -f wmsm
