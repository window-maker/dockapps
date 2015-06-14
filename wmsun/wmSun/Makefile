CC     = gcc
CFLAGS = -O2 -Wall
INCDIR = -I/usr/X11R6/include/X11
DESTDIR= /usr/X11R6
LIBDIR = -L/usr/X11R6/lib



# If you need to compile on a Linux, dont change anything. If
# compiling for Solaris, swap the LIBS below...
#
# For Linux (and other?) Machines...
#
LIBS   = -lXpm -lX11 -lXext
#
# For Sun Solaris Machines (I know it compiles on 2.6)...
#
# LIBS   = -lXpm -lX11 -lXext -lsocket


OBJS   = wmSun.o SunRise.o ../wmgeneral/wmgeneral.o


.c.o:
	$(CC) $(COPTS) -D$(shell echo `uname -s`) -c $< -o $*.o $(INCDIR)


all:	wmSun.o wmSun

wmSun.o: wmSun_master.xpm wmSun_mask.xbm 
wmSun:	$(OBJS) 
	$(CC) $(COPTS) $(SYSTEM) -lm -o wmSun $^ $(INCDIR) $(LIBDIR) $(LIBS)

clean:
	for i in $(OBJS) ; do \
		rm -f $$i; \
	done
	rm -f wmSun

install:: wmSun
	install -s -m 0755 wmSun $(DESTDIR)/bin
	install    -m 0644 wmSun.1 $(DESTDIR)/man/man1 

