LIBS = -lX11 -lXpm -lXext -lm
CC = gcc
TARGET = wmcalc
OBJECTS = wmcalc.o readln.o wmcalcswitch.o wmcalcfunc.o
CFLAGS = -O2 -Wall
DESTDIR =
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man/man1
CONF   = /etc
INSTALL = /usr/bin/install
INSTALL_DIR	= $(INSTALL) -d
INSTALL_PROGRAM = $(INSTALL)
INSTALL_FILE    = $(INSTALL) -m 644

.c.o:
	$(CC) -DCONF=\"$(CONF)\" $(CPPFLAGS) $(CFLAGS) -c $< -o $*.o

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

clean::
	for i in $(OBJECTS); do if [ -e $$i ] ; then rm $$i; fi; done
	if [ -e $(TARGET) ] ; then rm $(TARGET); fi

install::
	$(INSTALL_DIR) $(DESTDIR)$(BINDIR)
	$(INSTALL_PROGRAM) wmcalc $(DESTDIR)$(BINDIR)
	$(INSTALL_DIR) $(DESTDIR)$(CONF)
	$(INSTALL_FILE) wmcalc.conf $(DESTDIR)$(CONF)
	$(INSTALL_DIR) $(DESTDIR)$(MANDIR)/man1
	$(INSTALL_FILE) wmcalc.1 $(DESTDIR)$(MANDIR)
