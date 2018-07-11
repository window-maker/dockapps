
# uncomment the following lines if your using Solaris
#SOLARIS = -D__solaris__
#SOLARIS_LIBS = -lsocket -lkstat
#SOLARIS_OBJS = solaris26.o

LIBDIR = -L/usr/X11R6/lib
LIBS   = -lX11 $(SOLARIS_LIBS) -ldockapp
OBJS =  asmon.o $(SOLARIS_OBJS)

.c.o:
	gcc -c -Wall $(SOLARIS) $< -o $*.o

asmon: $(OBJS)
	gcc -o asmon $(OBJS) $(LIBDIR) $(LIBS)

clean::
	for i in $(OBJS) ; do \
		rm $$i;\
	done
	rm asmon

install::
	cp -f asmon /usr/local/bin
