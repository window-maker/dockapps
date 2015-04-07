OBJS  = wmload.o
LIBS = -lX11 -lXpm -lXext -lm
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
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
