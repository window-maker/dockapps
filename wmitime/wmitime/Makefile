#LANG = fr
LIBDIR = -L/usr/X11R6/lib
LIBS   = -lXpm -lXext -lX11 -lm
FLAGS = -O2
OBJS =	wmitime.o \
		../wmgeneral/wmgeneral.o \
		../wmgeneral/misc.o \
		../wmgeneral/list.o


.c.o:
	cc -I/usr/X11R6/share/include $(FLAGS) -D$(LANG) -c -Wall $< -o $*.o

wmtime: $(OBJS)
	cc $(FLAGS) -D$(LANG) -o wmitime $^ -lXext $(LIBDIR) $(LIBS)

all:: wmtime

clean::
	for i in $(OBJS) ; do \
		rm -f $$i ; \
	done
	rm -f wmitime
	rm -f *~

install:: wmitime
	cp -f wmitime /usr/local/bin/
	chmod 755 /usr/local/bin/wmitime
	chown root:root /usr/local/bin/wmitime
#	cp wminetrc $(HOME)/.wminetrc
#	chmod 600 $(HOME)/.wminetrc
#	cp wminetrc /etc/wminetrc
#	chmod 644 /etc/wminetrc
	@echo "wmitime Installation finished..."













