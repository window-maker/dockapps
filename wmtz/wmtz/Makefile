CC = gcc
LIBDIR = -L/usr/X11R6/lib
LIBS   = -lXpm -lXext -lX11 -lm -ldockapp
FLAGS = -O2
DESTDIR = /usr/local
OBJS =	wmtz.o

.c.o:
	$(CC) -I/usr/X11R6/include $(FLAGS) -c -Wall $< -o $*.o

wmtz: $(OBJS)
	$(CC) $(FLAGS) -o wmtz $^ -lXext $(LIBDIR) $(LIBS)

all:: wmtz

clean::
	for i in $(OBJS) ; do \
		rm -f $$i ; \
	done
	rm -f wmtz
	rm -f *~

install:: wmtz
	cp -f wmtz $(DESTDIR)/bin/
	chmod 755 $(DESTDIR)/bin/wmtz
	chown root:root $(DESTDIR)/bin/wmtz
	cp -f wmtz.1 $(DESTDIR)/man/man1
	chmod 644 $(DESTDIR)/man/man1/wmtz.1
	chown root:root $(DESTDIR)/man/man1/wmtz.1
	cp -f wmtzrc $(HOME)/.wmtzrc
	chmod 600 $(HOME)/.wmtzrc
	cp -f wmtzrc /etc/wmtzrc
	chmod 644 /etc/wmtzrc
	chown root:root /etc/wmtzrc
	@echo "wmtz: Installation finished..."

uninstall::
	rm -i /usr/local/bin/wmtz
	rm -i /etc/wmtzrc
	chmod 666 $(HOME)/.wmtzrc
	rm -i $(HOME)/.wmtzrc
	@echo "wmtz: Uninstall completed..."
