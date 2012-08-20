INCLUDES =-I/usr/X11R6/include/X11 -I/usr/local/include -I/usr/include/X11R6/X11
LIBINC =-L/usr/X11R6/lib -L/usr/include/lib
LIBS = -lX11 -lXpm -lXext

TARGET = wmbutton
OBJECTS = wmbutton.o wmb_libs.o

CFLAGS += -c -Wall -O2

all: ${TARGET} tags

.c.o:
	gcc ${CFLAGS} ${INCLUDES} $< -o $*.o

${TARGET}: ${OBJECTS}
	gcc -o ${TARGET} ${OBJECTS} ${LIBINC} ${LIBS}

clean::
	for i in ${OBJECTS}; do if [ -e $$i ] ; then rm $$i; fi; done 
	if [ -e ${TARGET} ] ; then rm ${TARGET}; fi
	if [ -e tags ]; then rm tags; fi
	if [ -e core ]; then rm core; fi

wmbutton.o: wmbutton.c wmbutton.h Makefile

wmb_libs.o: wmb_libs.c wmbutton.h Makefile

tags:
	ctags *.[ch]
