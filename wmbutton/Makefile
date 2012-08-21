INCLUDES =-I/usr/include -I/usr/local/include
LIBINC =-L/usr/X11R6/lib -L/usr/include/lib
LIBS = -lX11 -lXpm -lXext

TARGET = wmbutton
OBJECTS = wmbutton.o wmb_libs.o
DESTDIR =
PREFIX = /usr/local
BINDIR = ${PREFIX}/bin
MANDIR = ${PREFIX}/share/man
CONF   = /etc
CONFFL = ${CONF}/wmbutton.conf
INSTALL = /usr/bin/install
INSTALL_PROGRAM = $(INSTALL) -p -o root -g root -m 755
INSTALL_FILE    = $(INSTALL) -p -o root -g root -m 644

CFLAGS += -Wall -O2

.c.o:
	gcc -c ${CFLAGS} ${INCLUDES} $< -o $*.o

${TARGET}: ${OBJECTS}
	gcc -o ${TARGET} ${OBJECTS} ${LIBINC} ${LIBS}

clean::
	for i in ${OBJECTS}; do if [ -e $$i ] ; then rm $$i; fi; done 
	if [ -e ${TARGET} ] ; then rm ${TARGET}; fi
	if [ -e tags ]; then rm tags; fi
	if [ -e core ]; then rm core; fi

install::
	${INSTALL_PROGRAM} wmbutton ${DESTDIR}${BINDIR}
	${INSTALL_FILE} sample.wmbutton ${DESTDIR}${CONFFL}

wmbutton.o: wmbutton.c wmbutton.h Makefile

wmb_libs.o: wmb_libs.c wmbutton.h Makefile

tags:
	ctags *.[ch]
