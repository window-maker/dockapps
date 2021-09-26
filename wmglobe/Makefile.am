#bin_PROGRAMS = wmglobe
SUBDIRS = src
#wmglobe_SOURCES = mycontext.c rend.c wmglobe.c wmgoption.h zapnum.h myconvert.c sunpos.c wmglobe.h wmgutil.c
dist_doc_DATA = README

INC	=	-I/usr/X11R6/include
LIBS	=	-lXext -lX11 -lm -L/usr/X11R6/lib \
		-ltiff  -lXpm -lpng -lz -ljpeg @LIBGIF@ -lwraster
OBJS	=	src/wmglobe.o src/rend.o src/wmgutil.o \
		src/sunpos.o src/myconvert.o src/mycontext.o

.c.o :
	gcc -c -O2 -Wall $(INC) $< -o $*.o

all:  wmglobe

#clean:
#	rm -f src/*.o wmglobe

wmglobe :	$(OBJS)
		gcc -O2 -Wall $(OBJS) -o wmglobe $(LIBS)
		strip wmglobe

#install:
#		install wmglobe /usr/local/bin
#		install wmglobe.1 /usr/local/man/man1
