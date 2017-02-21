CC     = gcc
CFLAGS = -O2 -Wall
INCDIR = -I/usr/X11R6/include/X11
DESTDIR= /usr/X11R6
LIBDIR = -L/usr/X11R6/lib
# for linux
LIBS   = -lXpm -lX11 -lXext -ldockapp
# for Solaris
# LIBS   = -lXpm -lX11 -lXext -lsocket
OBJS   = wmSpaceWeather.o


.c.o:
	$(CC) $(COPTS) -D$(shell echo `uname -s`) -c $< -o $*.o $(INCDIR)


all:	wmSpaceWeather.o wmSpaceWeather

wmSpaceWeather.o: wmSpaceWeather_master.xpm wmSpaceWeather_mask.xbm
wmSpaceWeather:	$(OBJS)
	$(CC) $(COPTS) $(SYSTEM) -o wmSpaceWeather $^ $(INCDIR) $(LIBDIR) $(LIBS)

clean:
	for i in $(OBJS) ; do \
		rm -f $$i; \
	done
	rm -f wmSpaceWeather

install:: wmSpaceWeather
	install -s -m 0755 wmSpaceWeather $(DESTDIR)/bin
	install    -m 0755 GetKp $(DESTDIR)/bin
	install    -m 0644 wmSpaceWeather.1 $(DESTDIR)/man/man1

