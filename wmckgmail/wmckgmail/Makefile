# wmckgmail 1.1 : A dockapp to monitor the number of unread mails in a gmail inbox
# written by    : Sylvain Tremblay <stremblay@gmail.com>
# release date  : September 09, 2006

LIBDIR = -L/usr/X11R6/lib
LIBS   = -lXpm -lXext -lX11 -lm -ldockapp
FLAGS = -O2
OBJS =	wmckgmail.o

.c.o:
	cc -I/usr/X11R6/share/include $(FLAGS) -D$(LANG) -c -Wall $< -o $*.o

wmckgmail: $(OBJS)
	cc $(FLAGS) -D$(LANG) -o wmckgmail $^ -lXext $(LIBDIR) $(LIBS)

all:: wmckgmail

clean::
	for i in $(OBJS) ; do \
		rm -f $$i ; \
	done
	rm -f wmckgmail
	rm -f *~

install:: wmckgmail
	@echo "Installing wmckgmail binary in /usr/local/bin/wmckgmail"
	@cp wmckgmail /usr/local/bin

install-man::
	@echo "Installing wmckgmail man page in /usr/local/man/man1/wmckgmail.1";
	@cp wmckgmail.1 /usr/local/man/man1
