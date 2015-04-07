OBJS  = wmload.o
LIBS = -lX11 -lXpm -lXext -lm
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man/man1
INSTALL = install

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

wmload: $(OBJS)
	$(CC) $(LDFLAGS) -o wmload $(OBJS) $(LIBS)

clean:
	rm -f $(OBJS) wmload

install: wmload
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL) wmload $(DESTDIR)$(BINDIR)
	$(INSTALL) -d $(DESTDIR)$(MANDIR)
	$(INSTALL) -m 644 wmload.1 $(DESTDIR)$(MANDIR)
