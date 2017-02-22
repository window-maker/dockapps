LIBDIR = -L/usr/X11R6/lib
LIBS   = -lXpm -lXext -lX11
OBJS =  wmdonkeymon.o ../wmgeneral/wmgeneral.o

.c.o:
	cc -c -O2 -Wall $< -o $*.o

wmmon: $(OBJS)
	cc -o wmdonkeymon $^ $(LIBDIR) $(LIBS)

install:
	cp wmdonkeymon /usr/local/bin/

uninstall:
	rm /usr/local/bin/wmdonkeymon

clean::
	rm wmdonkeymon
	for i in $(OBJS) ; do \
		rm $$i;\
	done
