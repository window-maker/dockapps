
# Set this to 'y' if you want support for reading the link quality
# of nowiresneeded 1148 PCMCIA or Swallow cards

ENABLE_NWN_SUPPORT=n


##################################################################
# Nothing to configure under here

NAME=wmifinfo
VERSION=0.11

CC = gcc
LD = $(CC)
INSTALL = install
CFLAGS = -Wall -O2
COPTS = -D'VERSION="$(VERSION)"' -D'NAME="$(NAME)"'
LDOPTS = -lX11 -lXpm -lXext
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man

BIN =	wmifinfo
FILES = wmifinfo.o xutils.o

ifeq ("$(ENABLE_NWN_SUPPORT)", "y")
FILES += nwn.o
COPTS += -DENABLE_NWN_SUPPORT
endif

all:	$(BIN)

.c.o:
	$(CC) $(COPTS) $(CPPFLAGS) $(CFLAGS) -c $<

$(BIN):	$(FILES)
	$(LD) $(LDFLAGS) -o $@ $(FILES) $(LDOPTS)

clean:
	rm -f *.o $(BIN) core ./.#* *.orig *.rej

install:
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL) $(BIN) $(DESTDIR)$(BINDIR)
	$(INSTALL) -d $(DESTDIR)$(MANDIR)/man1
	$(INSTALL) -m 644 wmifinfo.1 $(DESTDIR)$(MANDIR)/man1

dist:	clean
	rm -rf /tmp/wmifinfo-$(VERSION)
	cd .. && cp -a wmifinfo /tmp/wmifinfo-$(VERSION)
	cd /tmp && tar --exclude CVS -zcvf wmifinfo-$(VERSION).tgz wmifinfo-$(VERSION)/

