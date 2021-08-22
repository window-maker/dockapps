LIBDIR = -L/usr/X11R6/lib
LIBS   = -lXpm -lXext -lX11
COMPILLER_FLAGS = -O2
LINKER_DEBUG_FLAGS = -ggdb -lefence
LINKER_FLAGS=


OBJS =	wmsvn/wmsvn.o \
	wmgeneral/wmgeneral.o \
	wmgeneral/misc.o \
	wmgeneral/list.o\

.c.o:
	cc -I/usr/X11R6/share/include $(COMPILLER_FLAGS) -c -Wall $< -o $*.o

wmsvn: $(OBJS)
	cc $(DEBUG_FLAGS) -o wmsvn.out $^  -lXext $(LIBDIR) $(LIBS)

all:: wmsvn

clean::
	for i in $(OBJS) ; do \
		rm -f $$i ; \
	done
	rm -f wmsvn.out

