CC     = gcc
CFLAGS = -O2 -Wall
INCDIR = -I/usr/X11R6/include
DESTDIR= /usr/X11R6
LIBDIR = -L/usr/X11R6/lib
LIBS   = -lXpm -lX11 -lXext
OBJS   = wmapm.o \
         ../wmgeneral/wmgeneral.o

.c.o:
	$(CC) $(COPTS) -D$(shell echo `uname -s`) -c $< -o $*.o $(INCDIR)

all:	wmapm.o wmapm

wmapm.o: wmapm_master.xpm wmapm_mask.xbm wmapm.h
wmapm: $(OBJS)
	$(CC) $(COPTS) $(SYSTEM) -o wmapm $^ $(INCDIR) $(LIBDIR) $(LIBS)

clean:
	for i in $(OBJS) ; do \
		rm -f $$i; \
	done
	rm -f wmapm

install:: wmapm
	install -s -m 0755 wmapm $(DESTDIR)/bin
	install    -m 0644 wmapm.1 $(DESTDIR)/man/man1 
	chmod +s $(DESTDIR)/bin/wmapm

