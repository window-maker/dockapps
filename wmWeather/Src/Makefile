CC     = gcc
CFLAGS = -O2 -Wall
INCDIR = -I/usr/X11R6/include/X11 -I/usr/X11R6/include
DESTDIR= /usr/X11R6
LIBDIR = -L/usr/X11R6/lib
# for linux
LIBS   = -lXpm -lX11 -lXext
# for Solaris
# LIBS   = -lXpm -lX11 -lXext -lsocket
OBJS   = wmWeather.o \
         xutils.o


.c.o:
	$(CC) $(CFLAGS) -D$(shell echo `uname -s`) -c $< -o $*.o $(INCDIR)


all:	wmWeather.o wmWeather

wmWeather.o: wmWeather_master.xpm wmWeather_mask.xbm
wmWeather:	$(OBJS) 
	$(CC) $(CFLAGS) $(SYSTEM) -o wmWeather $^ $(INCDIR) $(LIBDIR) $(LIBS)

clean:
	for i in $(OBJS) ; do \
		rm -f $$i; \
	done
	rm -f wmWeather

install:: wmWeather
	install -c -s -m 0755 wmWeather $(DESTDIR)/bin
	install -c    -m 0755 GrabWeather $(DESTDIR)/bin
	install -c    -m 0644 wmWeather.1 $(DESTDIR)/man/man1 

