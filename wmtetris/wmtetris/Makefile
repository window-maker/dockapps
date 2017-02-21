LIBDIR = -L/usr/X11R6/lib
LIBS   = -lXpm -lXext -lX11 -ldockapp
OBJS   =  wmtetris.o

wmtetris: $(OBJS)
	cc -O -o wmtetris $^ $(LIBDIR) $(LIBS)

%.o:	%.c
	cc -O -c -o $@ $<

clean:
	for i in $(OBJS) ; do \
		rm -f $$i;\
	done
	rm -f wmtetris
