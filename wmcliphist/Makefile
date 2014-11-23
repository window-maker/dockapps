CC ?= gcc
PREFIX = /usr/local
INCLUDES = `pkg-config --cflags gtk+-2.0 x11`

# for normal use
CFLAGS += -Wall -ansi -pedantic $(INCLUDES)
DEBUG = 

# for debuggind purposes
# ISO doesn't support macros with variable number of arguments so -pedantic
# must not be used
#CFLAGS += -Wall -g -ansi $(INCLUDES) -DFNCALL_DEBUG
#DEBUG = debug.o

LIBS = `pkg-config --libs gtk+-2.0 x11`

OBJECTS = wmcliphist.o clipboard.o gui.o rcconfig.o history.o hotkeys.o utils.o $(DEBUG)
TARGET = wmcliphist

all: $(TARGET)

lclint:
	lclint $(INCLUDES) +posixlib *.c >lclint.log

wmcliphist: $(OBJECTS) foodock/foodock.o
	$(CC) $(LDFLAGS) $(OBJECTS) foodock/foodock.o $(LIBS) -o $@

wmcliphist.o: wmcliphist.c wmcliphist.h \
	icon/ico_60x60_black.xpm icon/ico_60x60_gray.xpm \
	icon/ico_60x60_white.xpm icon/ico_60x60_mask.xbm \
	icon/ico_40x40_black.xpm icon/ico_40x40_gray.xpm \
	icon/ico_40x40_white.xpm icon/ico_40x40_mask.xbm \
	icon/ico_30x30_black.xpm icon/ico_30x30_gray.xpm \
	icon/ico_30x30_white.xpm icon/ico_30x30_mask.xbm \
	icon/ico_16x16.xpm icon/ico_16x16_mask.xbm \
	foodock/foodock.h

clipboard.o: clipboard.c wmcliphist.h

rcconfig.o: rcconfig.c wmcliphist.h

gui.o: gui.c wmcliphist.h

history.o: history.c wmcliphist.h

hotkeys.o: hotkeys.c wmcliphist.h

utils.o: utils.c wmcliphist.h

clean:
	rm -rf $(OBJECTS) $(TARGET)
	rm -rf core
	@(cd foodock && make clean)

install:
	cp wmcliphist $(PREFIX)/bin
