CC      = gcc 
INSTDIR = /usr/X11R6/bin/
MANDIR  = /usr/X11R6/man/

INCLUDES = \
	tile.xpm \
	wmisdncid.xpm \
	wmisdncidicons.xpm

OBJS    = \
	wmisdncid.o

SRC_DIR = ./

CFLAGS  = -c -ggdb -I./ -Wall

all: $(OBJS) wmisdncid

%.o: $(SRC_DIR)%.c $(INCLUDES)
	$(CC) $(CFLAGS) $(SRC_DIR)$*.c

wmisdncid: $(OBJS)
	$(CC) $(LFLAGS) -o wmisdncid $(OBJS) -L/usr/X11R6/lib -lXext -lXpm

install: wmisdncid
	strip wmisdncid
	install wmisdncid $(INSTDIR)
	install -m 444 wmisdncid.1x $(MANDIR)man1/

clean:
	-rm -f *.o
	-rm *~
	-rm core
	-rm wmisdncid

