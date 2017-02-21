

INCLUDES =-I/usr/X11R6/include/X11 -I/usr/local/include -I/usr/include/X11R6/X11
LIBINC =-L/usr/X11R6/lib -L/usr/include/lib
LIBS = -lm -lX11 -lXpm -lXext

TARGET  = wmcp
OBJECTS = wmcp.o
SOURCE  = wmcp.c

scalpel:
	if [ -e buttons.xpm ];             then rm buttons.xpm; fi
	if [ -e pushed_buttons.xpm ];      then rm pushed_buttons.xpm; fi
	if [ -e backdrop.xpm ];            then rm backdrop.xpm; fi
	ln -s buttons-scalpel.xpm          buttons.xpm
	ln -s pushed_buttons-scalpel.xpm   pushed_buttons.xpm
	ln -s backdrop-scalpel.xpm         backdrop.xpm
	make clean
	make ${TARGET}

gv:
	if [ -e buttons.xpm ];             then rm buttons.xpm; fi
	if [ -e pushed_buttons.xpm ];      then rm pushed_buttons.xpm; fi
	if [ -e backdrop.xpm ];            then rm backdrop.xpm; fi
	ln -s buttons-gv.xpm               buttons.xpm
	ln -s pushed_buttons-gv.xpm        pushed_buttons.xpm
	ln -s backdrop-gv.xpm              backdrop.xpm
	make clean
	make ${TARGET}

###################################################################

${OBJECTS}: ${SOURCE}
	gcc -g -c -Wall ${INCLUDES} ${FLAGS} $< -o $*.o

${TARGET}: ${OBJECTS}
	gcc -g -o ${TARGET} ${OBJECTS} ${LIBINC} ${LIBS}

clean::
	for i in ${OBJECTS}; do if [ -e $$i ] ; then rm $$i; fi; done
	if [ -e ${TARGET} ] ; then rm ${TARGET}; fi
