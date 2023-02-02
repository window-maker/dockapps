LIBDIR = -L/usr/X11R6/lib -L/usr/lib -L/opt/gnome/lib
LIBS   = -lXpm -lXext -lX11 `pkg-config gtk+-2.0 --libs` -ldockapp
CFLAGS = `pkg-config gtk+-2.0 --cflags`
OBJS =	washerDryer.o
prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
MKDIR_P = mkdir -p
INSTALL = install
INSTALL_PROGRAM = $(INSTALL)

.c.o:
	cc -c -O2 -Wall $< -o $*.o $(CFLAGS)

washerDryer: $(OBJS)
	cc -Wall -pedantic -o washerDryer $^ $(LIBDIR) $(LIBS)

all:: washerDryer

clean::
	for i in $(OBJS) ; do \
		rm -f $$i; \
	done
	rm -f washerDryer

install::   washerDryer
	@echo
	@echo "Installing config file..."
	cp wdryerrc $(HOME)/.wdryerrc
	chmod 600 $(HOME)/.wdryerrc
	@echo
	@echo "Installing executable..."
	$(MKDIR_P) $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) washerDryer $(DESTDIR)$(bindir)/washerDryer
	@echo
	@echo "Installing man page..."
	cp -f man/washerDryer.1.gz /usr/man/man1/
	chmod 644 /usr/man/man1/washerDryer.1.gz
	@echo
	@echo "washerDryer installation finished."

uninstall::
	@echo
	@echo "Removing executable..."
	rm -f $(DESTDIR)$(bindir)/washerDryer
	@echo
	@echo "Removing config file..."
	rm -f $(HOME)/.wdryerrc
	@echo
	@echo "Removing man page..."
	rm -f /usr/man/man1/washerDryer.1.gz
	@echo
	@echo "washerDryer uninstall finished."

