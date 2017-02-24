LIBDIR = -L/usr/X11R6/lib
LIBS   = -lXpm -lXext -lX11 -ldockapp
FLAGS  = -g2 -D_DEBUG
OBJS   = wmpop3.o Pop3Client.o

.c.o:
	cc -I/usr/X11R6/share/include $(FLAGS) -c -Wall $< -o $*.o

wmpop3lb: $(OBJS)
	cc $(FLAGS) -o wmpop3lb $^ -lXext $(LIBDIR) $(LIBS)

all:: wmpop3lb

clean::
	for i in $(OBJS) ; do \
		rm -f $$i ; \
	done
	rm -f wmpop3lb
	rm -f *~

install:: wmpop3lb
	@echo "Use 'make install1' for system install."
	@echo "Use 'make install2' for single user install."
	@echo

install1:: wmpop3lb
	cp -f wmpop3lb /usr/local/bin/
	chmod 755 /usr/local/bin/wmpop3lb
	chown root:root /usr/local/bin/wmpop3lb
	@echo
	@echo "WMPop3 Installation finished..."

install2:: wmpop3lb
	mkdir $(HOME)/wmpop3lb
	cp -f wmpop3lb $(HOME)/wmpop3lb
	chmod 755 $(HOME)/wmpop3lb
	@echo "WMPop3 Installation finished..."
	@echo
	@echo "Make sure ~/wmpop3 is in your path"
	@echo
	@echo "Please run wmpop3lb once and then edit ~/.wmpop3rc"

