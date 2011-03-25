#(c)1999-2003 redseb 
# Adapted for wmSMPmon 2.9.x and higher: Thomas Ribbrock

# Change the following two to determine installation location
BINDIR=/usr/local/bin/
MANDIR=/usr/local/share/man/

SRC = general.c sysinfo-linux.c ../wmgeneral/wmgeneral.c wmSMPmon.c 
EXE = wmSMPmon
MAN = wmSMPmon.1
OBJ = $(SRC:.c=.o)
INSTALL = /usr/bin/install
INSTALLEXEFLAGS = -m 755 -s
INSTALLMANFLAGS = -m 644
CC = gcc
CFLAGS = -Wall -O2 -g
LIB = -L/usr/X11R6/lib -lXpm -lXext -lX11

all: $(OBJ)
	$(CC) -o $(EXE) $(OBJ) $(LIB)

$(OBJ): %.o : %.c 
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(EXE) 
	rm -rf *.o
	rm -rf ../wmgeneral/*.o

install:
	$(INSTALL) $(INSTALLEXEFLAGS) $(EXE) $(BINDIR)
	$(INSTALL) -d $(MANDIR)/man1
	$(INSTALL) $(INSTALLMANFLAGS) $(MAN) $(MANDIR)/man1
