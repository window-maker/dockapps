LIBS   = -lXpm -lXext -lX11 -lm
CFLAGS = -O2 -Wall
OBJS =	wmitime.o \
		wmgeneral/wmgeneral.o \
		wmgeneral/misc.o \
		wmgeneral/list.o
INSTALL = install
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $*.o

wmitime: $(OBJS)
	$(CC) $(LDFLAGS) -o wmitime $^ $(LIBS)

all:: wmtime

clean::
	for i in $(OBJS) ; do \
		rm -f $$i ; \
	done
	rm -f wmitime
	rm -f *~

install:: wmitime
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL) wmitime $(DESTDIR)$(BINDIR)
	@echo "wmitime Installation finished..."
