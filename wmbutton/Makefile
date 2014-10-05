INCLUDES =-I/usr/include -I/usr/local/include
LIBINC =-L/usr/X11R6/lib -L/usr/include/lib
LIBS = -lX11 -lXpm -lXext

TARGET = wmbutton
OBJECTS = wmbutton.o wmb_libs.o
DESTDIR = /usr/local
PREFIX = /usr
BINDIR = /bin
CONF   = /etc
CONFFL = ${CONF}/wmbutton.conf
INSTALL = /usr/bin/install

CFLAGS += -Wall -O2
LDFLAGS +=

.c.o:
	gcc -c ${CFLAGS} ${INCLUDES} $< -o $*.o

${TARGET}: ${OBJECTS}
	gcc -o ${TARGET} ${OBJECTS}  ${LDFLAGS} ${LIBINC} ${LIBS}

clean::
	for i in ${OBJECTS}; do if [ -e $$i ] ; then rm $$i; fi; done
	if [ -e ${TARGET} ] ; then rm ${TARGET}; fi
	if [ -e tags ]; then rm tags; fi
	if [ -e core ]; then rm core; fi

install::
	$(INSTALL) -d -m 0755 ${DESTDIR}${PREFIX}${BINDIR} ${DESTDIR}${CONF}
	$(INSTALL) -p -o root -g root -m 755 wmbutton ${DESTDIR}${PREFIX}${BINDIR}
	$(INSTALL) -p -o root -g root -m 644 sample.wmbutton ${DESTDIR}${CONFFL}

wmbutton.o: wmbutton.c wmbutton.h Makefile

wmb_libs.o: wmb_libs.c wmbutton.h Makefile

tags:
	ctags *.[ch]
