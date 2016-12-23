LIBDIR = -L/usr/X11R6/lib -L/usr/lib -L/opt/gnome/lib
LIBS   = -lXpm -lXext -lX11 -lm  -lgtk -lgdk -rdynamic -lgmodule -lglib -ldl
CFLAGS = -I/usr/X11R6/include -I/usr/lib/glib/include -I/opt/gnome/include/gtk-1.2 -I/opt/gnome/include/glib-1.2 -I/opt/gnome/lib/glib/include
OBJS =	washerDryer.o \
		../wmgeneral/wmgeneral.o \
		../wmgeneral/misc.o \
		../wmgeneral/list.o

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
	cp -f washerDryer /usr/local/bin/
	chmod 755 /usr/local/bin/washerDryer
	chown root:root /usr/local/bin/washerDryer
	@echo
	@echo "Installing man page..."
	cp -f man/washerDryer.1.gz /usr/man/man1/
	chmod 644 /usr/man/man1/washerDryer.1.gz
	@echo
	@echo "washerDryer installation finished."

uninstall::
	@echo
	@echo "Removing executable..."
	rm -f /usr/local/bin/washerDryer
	@echo
	@echo "Removing config file..."
	rm -f $(HOME)/.wdryerrc
	@echo
	@echo "Removing man page..."
	rm -f /usr/man/man1/washerDryer.1.gz
	@echo
	@echo "washerDryer uninstall finished."

