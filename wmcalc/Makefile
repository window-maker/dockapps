INCLUDES =-I/usr/X11R6/include/X11 -I/usr/local/include
LIBINC =-L/usr/X11R6/lib
LIBS = -lX11 -lXpm -lXext -lm

TARGET = wmcalc
OBJECTS = wmcalc.o readln.o wmcalcswitch.o wmcalcfunc.o
DESTDIR =
PREFIX = /usr/local
BINDIR = ${PREFIX}/bin
MANDIR = ${PREFIX}/share/man/man1
CONF   = /etc
INSTALL = /usr/bin/install
INSTALL_PROGRAM = $(INSTALL) -p -o root -g root -m 755
INSTALL_FILE    = $(INSTALL) -p -o root -g root -m 644

.c.o:
	gcc -O2 -c -Wall ${INCLUDES} ${FLAGS} $< -o $*.o

${TARGET}: ${OBJECTS}
	gcc -O2 -o ${TARGET} ${OBJECTS} ${LIBINC} ${LIBS}

clean::
	for i in ${OBJECTS}; do if [ -e $$i ] ; then rm $$i; fi; done 
	if [ -e ${TARGET} ] ; then rm ${TARGET}; fi

install::
	${INSTALL_PROGRAM} wmcalc ${DESTDIR}${BINDIR}
	${INSTALL_FILE} wmcalc.conf ${DESTDIR}${CONF}
	${INSTALL_FILE} wmcalc.1 ${DESTDIR}${MANDIR}
