LIBDIR = -L/usr/X11R6/lib
LIBS   = -lXpm -lXext -lX11 -lm -ldockapp
OBJS =	wmkeys.o \

.c.o:
	cc -c -O2 -Wall $< -o $*.o

wmkeys: $(OBJS)
	cc -o wmkeys $^ $(LIBDIR) $(LIBS)

clean::
	for i in $(OBJS) ; do \
		rm $$i; \
	done
	rm wmkeys
