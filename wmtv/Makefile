#
#	Makefile for wmtv
#
INCDIR	=	-I/usr/X11R6/include
LIBDIR	=	-L/usr/X11R6/lib
LIBS	=	-lXpm -lXext -lX11 -lXxf86dga -lXxf86vm -ldockapp
OBJS	=	src/wmtv.o

.c.o:
	cc -c -O2 -Wall -D$(shell echo `uname -s`) $< -o $*.o $(INCDIR)

all: wmtv

wmtv: $(OBJS)
	cc -o wmtv $^ $(LIBDIR) $(LIBS)

install:: all
	install -s -m 4755 -o root wmtv /usr/X11R6/bin
	install -m 0644 -o root -g root wmtvrc.sample /etc/wmtvrc

clean:
	for i in $(OBJS) ; do \
		rm -f $$i; \
	done
	rm -f wmtv
