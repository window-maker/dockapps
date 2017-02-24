LIBDIR = -L/usr/X11R6/lib
LIBS   = -lXpm -lXext -lX11 -ldockapp
FLAGS  = -O2
OBJS   = wmpop3.o Pop3Client.o

.c.o:
	cc -I/usr/X11R6/share/include $(FLAGS) -c -Wall $< -o $*.o

wmpop3: $(OBJS)
	cc $(FLAGS) -o wmpop3 $^ -lXext $(LIBDIR) $(LIBS)

all:: wmpop3

clean::
	for i in $(OBJS) ; do \
		rm -f $$i ; \
	done
	rm -f wmpop3
	rm -f *~

install:: wmpop3
	@echo "Use 'make install1' for system install."
	@echo "Use 'make install2' for single user install."
	@echo

install1:: wmpop3
	cp -f wmpop3 /usr/local/bin/
	chmod 755 /usr/local/bin/wmpop3
	chown root:root /usr/local/bin/wmpop3
	@echo
	@echo "WMPop3 Installation finished..."

install2:: wmpop3
	mkdir $(HOME)/wmpop3
	cp -f wmpop3 $(HOME)/wmpop3
	chmod 755 $(HOME)/wmpop3
	cp -f wmpop3rc $(HOME)/.wmpop3rc
	chmod 600 $(HOME)/.wmpop3rc
	@echo "WMPop3 Installation finished..."
	@echo
	@echo "Make sure ~/wmpop3 is in your path"
	@echo
	@echo "Please edit ~/.wmpop3rc before running WMPop3."
