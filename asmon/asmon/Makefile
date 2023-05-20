
# uncomment the following lines if your using Solaris
#SOLARIS = -D__solaris__
#SOLARIS_LIBS = -lsocket -lkstat
#SOLARIS_OBJS = solaris26.o

LIBDIR = -L/usr/X11R6/lib
LIBS   = -lX11 $(SOLARIS_LIBS) -ldockapp
OBJS =  asmon.o $(SOLARIS_OBJS)
prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
MKDIR_P = mkdir -p
INSTALL = install
INSTALL_PROGRAM = $(INSTALL)

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
	$(MKDIR_P) $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) asmon $(DESTDIR)$(bindir)/asmon
