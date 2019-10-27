include makeinfo

all: wmbattery

clean:
	rm -f wmbattery *.o

distclean: clean
	rm -f config.status config.cache config.log makeinfo config.h \
		config.h.in configure

install: all
	$(INSTALL) -d $(DESTDIR)$(bindir) $(DESTDIR)$(man1dir) $(DESTDIR)$(icondir)
	$(INSTALL_PROGRAM) wmbattery $(DESTDIR)$(bindir)
	$(INSTALL_DATA) $(srcdir)/wmbattery.1 $(DESTDIR)$(man1dir)/wmbattery.1
	$(INSTALL_DATA) $(srcdir)/*.xpm $(DESTDIR)$(icondir)

uninstall:
	rm -rf $(bindir)/wmbattery $(man1dir)/wmbattery.1 $(icondir)

OBJS=wmbattery.o acpi.o sonypi.o

ifdef USE_HAL
LIBS+=$(shell pkg-config --libs hal)
OBJS+=simplehal.o
CFLAGS+=-DHAL
simplehal.o: simplehal.c
	$(CC) $(CFLAGS) $(shell pkg-config --cflags hal) -c simplehal.c -o simplehal.o
endif

ifdef USE_UPOWER
LIBS+=$(shell pkg-config --libs upower-glib)
OBJS+=upower.o
CFLAGS+=-DUPOWER
upower.o: upower.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(shell pkg-config --cflags upower-glib) -c upower.c -o upower.o
endif

wmbattery: $(OBJS)
	$(CC) -o wmbattery $(LDFLAGS) $(OBJS) $(LIBS)

wmbattery.o: wmbattery.c wmbattery.h
