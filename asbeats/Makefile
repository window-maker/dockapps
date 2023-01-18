GCC=gcc
RM=rm -f
LIBPATHS=-L/usr/X11R6/lib
LIBS=-lXpm -lXext -lX11
PROG=asbeats
OBJS=asbeats.o
prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
MKDIR_P = mkdir -p
INSTALL = install
INSTALL_PROGRAM = $(INSTALL)

all:	${OBJS}
	${GCC} ${OBJS} -o ${PROG} ${LIBPATHS} ${LIBS}

clean:
	${RM} ${PROG} *~ *.o

install:
	$(MKDIR_P) $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) asbeats $(DESTDIR)$(bindir)/asbeats
