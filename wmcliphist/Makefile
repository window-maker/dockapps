srcCC ?= gcc
PKG_CONFIG ?= pkg-config
INSTALL = install
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin
DATADIR = $(PREFIX)/share/wmcliphist
MAN1DIR = $(PREFIX)/share/man/man1
INCLUDES = `$(PKG_CONFIG) --cflags gtk+-3.0 x11`

# for normal use
CFLAGS += -Wall -ansi -pedantic $(INCLUDES) -DDATADIR=\"$(DATADIR)\"
DEBUG =

# for debuggind purposes
# ISO doesn't support macros with variable number of arguments so -pedantic
# must not be used
#CFLAGS += -Wall -g -ansi $(INCLUDES) -DFNCALL_DEBUG -DDATADIR=\"$(DATADIR)\"
#DEBUG = debug.o

LIBS = `$(PKG_CONFIG) --libs gtk+-3.0 x11`

OBJECTS = wmcliphist.o clipboard.o gui.o rcconfig.o history.o hotkeys.o utils.o $(DEBUG)
TARGET = wmcliphist
ICONS = icon/ico_16x16.png icon/ico_30x30_black.png icon/ico_30x30_gray.png \
	icon/ico_30x30_white.png icon/ico_40x40_black.png \
	icon/ico_40x40_gray.png icon/ico_40x40_white.png \
	icon/ico_60x60_black.png icon/ico_60x60_gray.png \
	icon/ico_60x60_white.png


all: $(TARGET)

lclint:
	lclint $(INCLUDES) +posixlib *.c >lclint.log

wmcliphist: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@

wmcliphist.o: wmcliphist.c wmcliphist.h

clipboard.o: clipboard.c wmcliphist.h

rcconfig.o: rcconfig.c wmcliphist.h

gui.o: gui.c wmcliphist.h

history.o: history.c wmcliphist.h

hotkeys.o: hotkeys.c wmcliphist.h

utils.o: utils.c wmcliphist.h

clean:
	rm -rf $(OBJECTS) $(TARGET)
	rm -rf core

install:
	$(INSTALL) -d $(DESTDIR)$(DATADIR)
	$(INSTALL) -m 644 $(ICONS) $(DESTDIR)$(DATADIR)
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL) $(TARGET) $(DESTDIR)$(BINDIR)
	$(INSTALL) -d $(DESTDIR)$(MAN1DIR)
	$(INSTALL) -m 644 wmcliphist.1 $(DESTDIR)$(MAN1DIR)
