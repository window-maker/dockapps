CC     = gcc
CFLAGS = -O2 -Wall
INCDIR = -I/usr/X11R6/include/X11
DESTDIR= /usr/X11R6
LIBDIR = -L/usr/X11R6/lib
# for linux
LIBS   = -lXpm -lX11 -lXext
# for Solaris
# LIBS   = -lXpm -lX11 -lXext -lsocket
OBJS   = wmGrabImage.o \
         ../wmgeneral/wmgeneral.o


.c.o:
	$(CC) $(CFLAGS) -D$(shell echo `uname -s`) -c $< -o $*.o $(INCDIR)


all:	wmGrabImage.o wmGrabImage

wmGrabImage.o: wmGrabImage_master.xpm wmGrabImage_mask.xbm
wmGrabImage:	$(OBJS) 
	$(CC) $(CFLAGS) $(SYSTEM) -o wmGrabImage $^ $(INCDIR) $(LIBDIR) $(LIBS)

clean:
	for i in $(OBJS) ; do \
		rm -f $$i; \
	done
	rm -f wmGrabImage

install:: wmGrabImage
	install -s -m 0755 wmGrabImage $(DESTDIR)/bin
	install    -m 0755 GrabImage $(DESTDIR)/bin
	install    -m 0644 wmGrabImage.1 $(DESTDIR)/man/man1 

