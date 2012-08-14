CC     = gcc
CFLAGS = -O2 -Wall 
INCDIR = -I/usr/X11R6/include/X11 -I/usr/X11R6/include
DESTDIR= /usr/X11R6
LIBDIR = -L/usr/X11R6/lib

# for Linux
LIBS   = -lXpm -lX11 -lXext

# for Solaris
#LIBS   = -lXpm  -lX11 -lXext -lsocket -lnsl


OBJS   = wmMatrix.o matrix.o xutils.o yarandom.o




.c.o:
	$(CC) $(CFLAGS) -D$(shell echo `uname -s`) -c $< -o $*.o $(INCDIR)


all:	wmMatrix.o wmMatrix 

wmMatrix.o: wmMatrix_master.xpm wmMatrix_mask.xbm
wmMatrix:	$(OBJS) 
	$(CC) $(COPTS) $(SYSTEM) -o wmMatrix $^ $(INCDIR) $(LIBDIR) $(LIBS)


clean:
	for i in $(OBJS) ; do \
		rm -f $$i; \
	done
	rm -f wmMatrix

install:: wmMatrix
	install -c -s -m 0755 wmMatrix $(DESTDIR)/bin

