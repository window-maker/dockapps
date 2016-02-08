DEBUG  = -DPARANOID
LIBDIR = -L/usr/X11R6/lib
LIBS   = -lXpm -lXext -lX11
INCS   = -I/usr/X11R6/share/include -I/usr/X11R6/include
OBJS   = wmtop.o wmgeneral/wmgeneral.o wmgeneral/misc.o wmgeneral/list.o
OPTS   = -O3 -g -Wall
FLAGS  = $(DEBUG) $(OPTS) -D$(OS)
MAKE   = make
CC     = cc
PREFIX ?= /usr/local

default:
	@echo Use '"make os"' where '"os"' is one of:
	@echo "    freebsd"
	@echo "    linux"
	@echo Then '"make install"'

freebsd:
	$(MAKE) OS=FREEBSD all

linux:
	$(MAKE) OS=LINUX all

.c.o:
	$(CC) $(FLAGS) $(INCS) -c $< -o $*.o

wmtop: $(OBJS) wmtop.o
	$(CC) $(FLAGS) -o wmtop $(OBJS) $(LIBDIR) $(LIBS)
	@echo Now you can do a '"make install"'

all: wmtop

clean:
	for i in $(OBJS) ; do \
		rm -f $$i ; \
	done
	rm -f wmtop 
	rm -f *~

install: wmtop
	strip wmtop
	cp -f wmtop $(PREFIX)/bin/
	chmod 755 $(PREFIX)/bin/wmtop
	cp -f wmtop.1 $(PREFIX)/man/man1
	chmod 644 $(PREFIX)/man/man1/wmtop.1
	@echo "wmtop Installation finished..."
