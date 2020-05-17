CC = gcc
OS = -DLINUX

LIBDIR = -L/usr/X11R6/lib
LIBDIR = -L/usr/X11R6/lib
INCDIR = -I/usr/X11R6/share/include

PREFIX = /usr
BINDIR = $(PREFIX)/bin
DOCDIR = $(PREFIX)/doc/wmcube-1.0.0
OBJDIR = $(PREFIX)/share/wmcube

# Edit OSLIBS as appropriate to include OS specific libraries.

OSLIBS =
LIBS   = -lXpm -lXext -lX11 -lm $(OSLIBS)
CFLAGS = -Wall -O2 $(OS)

OBJS =	wmcube.o \
	../wmgeneral/wmgeneral.o \
	../wmgeneral/misc.o \
	../wmgeneral/list.o

.c.o:
	$(CC) $(CFLAGS) $(INCDIR) -c $< -o $*.o

wmcube: $(OBJS)
	$(CC) -o wmcube $^ -lXext $(LIBDIR) $(LIBS)

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
	chown root:root $(BINDIR)/wmcube
	mkdir -p $(DOCDIR)
	cp -p ../CHANGES ../COPYING ../INSTALL ../README $(DOCDIR)
	cp -pr ../3D-objects $(DOCDIR)
	mkdir -p $(OBJDIR)
	cp -p ../3D-objects/* $(OBJDIR)
	@echo "wmcube installation finished..."

uninstall::
	rm -f $(BINDIR)/wmcube
	rm -f $(DOCDIR)/3D-objects/*
	if [ -d $(DOCDIR)/3D-objects ]; then rmdir $(DOCDIR)/3D-objects; fi
	rm -f $(DOCDIR)/*
	if [ -d $(DOCDIR) ]; then rmdir $(DOCDIR); fi
	rm -f $(OBJDIR)/*
	if [ -d $(OBJDIR) ]; then rmdir $(OBJDIR); fi
