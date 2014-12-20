LIBS   = -lXpm -lXext -lX11 -lm
CFLAGS = -O2 -Wall
OBJS =	wmitime.o \
		wmgeneral/wmgeneral.o \
		wmgeneral/misc.o \
		wmgeneral/list.o
INSTALL = install
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man/man1

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
	$(INSTALL) -d $(DESTDIR)$(MANDIR)
	$(INSTALL) -m 644 wmitime.1 $(DESTDIR)$(MANDIR)
	@echo "wmitime Installation finished..."
