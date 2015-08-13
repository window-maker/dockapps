PREFIX = /usr/local
DESTDIR = $(PREFIX)/lib
INCDIR = $(PREFIX)/include/wmgeneral
LIBDIR = -L/usr/X11R6/lib
LIBS   = -lXpm -lXext -lX11
TARGET = libwmgeneral.so
OBJECTS =  wmgeneral.o \
	misc.o \
	list.o
HEADERS = wmgeneral.h \
	list.h \
	misc.h

INSTALL = install
INSTALL_LIB = $(INSTALL)

CFLAGS = -O2 -c -Wall -Werror -fpic
LDFLAGS = -shared
CC = cc

.c.o::
	$(CC) $(CFLAGS) $< -o $*.o

$(TARGET):: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJECTS)

clean::
	for i in $(OBJECTS) ; do \
		rm -f $$i;\
	done
	rm -f $(TARGET)

install:: $(TARGET)
	$(INSTALL) -d $(DESTDIR)
	$(INSTALL) -m 644 $(TARGET) $(DESTDIR)
	$(INSTALL) -d $(INCDIR)
	$(INSTALL) -m 644 $(HEADERS) $(INCDIR)
