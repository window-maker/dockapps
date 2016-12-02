#
# Makefile for wmymail
#


PROGRAM=wmymail

VERSION=$(shell cat VERSION)
DISTDIR=$(PROGRAM)-$(VERSION)
TARBALL=$(PROGRAM)-$(VERSION).tar.gz
LSM=$(PROGRAM)-$(VERSION).lsm

CC      = cc
LIBS    = -L/usr/X11R6/lib -lX11 -lXpm -lXext -ldockapp
CFLAGS  = -O2 -I/usr/X11R6/include -Wall
XPMS = xpm/main.xpm xpm/mbox_1.xpm xpm/mbox_2.xpm xpm/mbox_3.xpm \
  xpm/numbers.xpm xpm/unumbers.xpm
DISTFILES = Makefile README README.old ChangeLog wmymail.c \
  raw-envelope.jpg lsm.in VERSION

default all target: wmymail

wmymail: wmymail.o
	$(CC) $< $(CFLAGS) $(LIBS) -o $@

wmymail.o: wmymail.c $(XPMS)
	$(CC) $(CFLAGS) $(MY_INCS) -o $@ -c $<

clean:
	rm -f wmymail wmymail.o $(LSM) $(TARBALL)
	rm -fr $(DISTDIR)

dist: $(TARBALL)

lsm: $(LSM)

$(LSM):
	make-lsm

$(TARBALL): $(DISTFILES) $(LSM)
	rm -fr $(DISTDIR)
	mkdir -p $(DISTDIR)/xpm
	cp $(DISTFILES) $(DISTDIR)
	cp $(XPMS) wmymail-$(VERSION)/xpm
	rm -f wmymail-$(VERSION).tar.gz
	tar -c $(DISTDIR) | gzip > $(TARBALL)
	make-lsm
	cp $(LSM) $(DISTDIR)
	tar -c $(DISTDIR) | gzip > $(TARBALL)

