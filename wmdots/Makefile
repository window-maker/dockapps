LIBDIR = -L/usr/X11R6/lib
LIBS   = -lXpm -lXext -lX11 -ldockapp
OBJS =	wmdots.o
prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
MKDIR_P = mkdir -p
INSTALL = install
INSTALL_PROGRAM = $(INSTALL)

.c.o:
	cc -c -g -O2 -Wall $< -o $*.o

wmdots: $(OBJS)
	cc -o wmdots $^ -lXext $(LIBDIR) $(LIBS)

all:: wmdots

clean::
	for i in $(OBJS) ; do \
		rm -f $$i ; \
	done
	rm -f wmdots

install::
	$(MKDIR_P) $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) wmdots $(DESTDIR)$(bindir)/wmdots
