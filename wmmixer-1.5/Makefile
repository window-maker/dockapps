# $Id: Makefile,v 1.5 2002/06/25 22:13:09 gordon Exp $

prefix      = /usr/local
exec_prefix = ${prefix}
bindir      = ${exec_prefix}/bin
mandir      = ${prefix}/share/man

DESTDIR     =

CXX	    = g++
CXXFLAGS    = -O -Wall
EXTRA_LIBS  = -L/usr/X11R6/lib -lX11 -lXpm -lXext


LD 	    = g++
LDFLAGS     = -o $(EXECUTABLE) $(EXTRA_LIBDIRS) $(EXTRA_LIBS) $(CXXFLAGS)

EXECUTABLE  = wmmixer
OBJS	    = xhandler.o mixctl.o wmmixer.o exception.o

INSTALL = install
INSTALL_FILE    = $(INSTALL) -D -p    -o root -g root  -m  644
INSTALL_PROGRAM = $(INSTALL) -D -p    -o root -g root  -m  755

.cc.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(EXECUTABLE): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS)

all: $(EXECUTABLE)

clean:
	rm -f $(OBJS) $(EXECUTABLE)

install: install-bin install-doc

install-bin: wmmixer
	$(INSTALL_PROGRAM) $< $(DESTDIR)$(bindir)/wmmixer

install-doc: wmmixer.1
	$(INSTALL_FILE) $< $(DESTDIR)$(mandir)/man1/wmmixer.1

uninstall: uninstall-bin uninstall-doc

uninstall-bin:
	rm -f $(DESTDIR)$(bindir)/wmmixer

uninstall-doc:
	rm -f $(DESTDIR)$(mandir)/man1/wmmixer.1

.PHONY: all clean dist-clean install install-bin install-doc \
	uninstall uninstall-bin uninstall-doc

