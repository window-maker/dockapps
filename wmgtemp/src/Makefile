CC      = gcc
INSTALL = /usr/bin/install
LIB     = lib
LDFLAGS = -L/usr/X11R6/$(LIB) -lXpm -lXext -lX11 -lsensors
BINARY  = wmgtemp
CCFLAGS = -Wall -g
INSTDIR = /usr/local/bin

OFILES  = wmgtemp.o \
	./wmgeneral/wmgeneral.o \
	./wmgeneral/misc.o \
	./wmgeneral/list.o

%.o:
	$(CC) $(CCFLAGS) -c $*.c -o $*.o

$(BINARY): $(OFILES)
	$(CC) $(OFILES) $(LDFLAGS) -o $(BINARY)

clean:
	@ $(RM) $(OFILES) *~ "#"* $(BINARY)

install::
	$(INSTALL) -m 755 -c $(BINARY) $(INSTDIR)/$(BINARY)

depend:
	@ $(CC) -MM *.c > .deps

count:
	@ wc -l *.c *.h | sort

include .deps
