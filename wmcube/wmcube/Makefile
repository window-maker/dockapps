CC = gcc
OS = -DLINUX

VERSION = 1.0.2

LIBDIR = -L/usr/X11R6/lib
LIBDIR = -L/usr/X11R6/lib
INCDIR = -I/usr/X11R6/share/include

PREFIX = /usr
BINDIR = $(PREFIX)/bin
DOCDIR = $(PREFIX)/doc/wmcube-$(VERSION)
MANDIR = $(PREFIX)/man/man1
OBJDIR = $(PREFIX)/share/wmcube

# Edit OSLIBS as appropriate to include OS specific libraries.

OSLIBS =
LIBS   = -lXpm -lXext -lX11 -ldockapp -lm $(OSLIBS)
CFLAGS = -Wall -O2

OBJS =	wmcube.o

.c.o:
	$(CC) $(OS) $(CPPFLAGS) $(CFLAGS) $(INCDIR) -c $< -o $*.o

wmcube: $(OBJS)
	$(CC) $(LDFLAGS) -o wmcube $^ -lXext $(LIBDIR) $(LIBS)

all:: wmcube

clean::
	for i in $(OBJS) ; do \
		rm -f $$i ; \
	done
	rm -f wmcube
	rm -f *~

install:: wmcube
	cp -f wmcube $(BINDIR)
	chmod 755 $(BINDIR)/wmcube
	mkdir -p $(DOCDIR)
	cp -p ../CHANGES ../COPYING ../INSTALL ../README ../THANKS $(DOCDIR)
	cp -pr ../3D-objects $(DOCDIR)
	cp -p wmcube.1 $(MANDIR)
	mkdir -p $(OBJDIR)
	cp -p ../3D-objects/* $(OBJDIR)
	@echo "wmcube installation finished..."

uninstall::
	rm -f $(BINDIR)/wmcube
	rm -f $(DOCDIR)/3D-objects/*
	if [ -d $(DOCDIR)/3D-objects ]; then rmdir $(DOCDIR)/3D-objects; fi
	rm -f $(DOCDIR)/*
	if [ -d $(DOCDIR) ]; then rmdir $(DOCDIR); fi
	rm -f $(MANDIR)/wmcube.1
	rm -f $(OBJDIR)/*
	if [ -d $(OBJDIR) ]; then rmdir $(OBJDIR); fi
