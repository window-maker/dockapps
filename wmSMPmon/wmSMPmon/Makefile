#(c)1999-2003 redseb
# Adapted for wmSMPmon 2.9.x and higher: Thomas Ribbrock

# Change the following to determine installation location
MANDIR=/usr/local/share/man

OS := $(shell uname -s)

SRC = general.c wmSMPmon.c
EXE = wmSMPmon
MAN = wmSMPmon.1
OBJ = $(SRC:.c=.o)
prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
MKDIR_P = mkdir -p
INSTALL = install
INSTALL_PROGRAM = $(INSTALL)
INSTALLEXEFLAGS = -m 755 -s
INSTALLMANFLAGS = -m 644
CC = gcc
CFLAGS = -Wall -O2 -g
LIB = -L/usr/X11R6/lib -lXpm -lXext -lX11 -ldockapp

ifeq ($(OS),Linux)
SRC += sysinfo-linux.c
endif
ifeq ($(OS),SunOS)
SRC += sysinfo-solaris.c
LIB += -lkstat
endif

all: $(OBJ)
	$(CC) -o $(EXE) $(OBJ) $(LIB)

$(OBJ): %.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(EXE)
	rm -rf *.o

install:
	$(MKDIR_P) $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) $(INSTALLEXEFLAGS) $(EXE) $(DESTDIR)$(bindir)
	$(INSTALL) -d $(MANDIR)/man1
	$(INSTALL) $(INSTALLMANFLAGS) $(MAN) $(MANDIR)/man1
