CC     = gcc
INCLUDES  = `pkg-config --cflags gtk+-2.0` -I.
LFLAGS = `pkg-config --libs gtk+-2.0`

# for BSD
#INCLUDES = `gtk12-config --cflags` -I.
#LFLAGS = `gtk12-config --libs`
X11BASE = /usr/X11R6
CFLAGS =  $(INCLUDES) -DICON_TYPE=$(ICON_TYPE)
INCDIR = -I$(X11BASE)/include/X11 -I$(X11BASE)/include -I$/usr/include -I$/usr/local/include
DESTDIR= /usr/local
LIBDIR = -L/usr/lib -L${X11BASE}/lib -L/usr/local/lib
PACKAGE = wmCalendar

# for Linux
LIBS   = $(LIBDIR) -lXpm -lical -lpthread
# for Solaris (untested!!!)
# LIBS   = -lXpm -lX11 -lXext -lsocket -lnsl -lical

OBJECTS   = 	wmCalendar.o \
		settings.o \
		dockapp.o \
		calendarfunc.o \
		calendar.o

.c.o:
	$(CC) $(CFLAGS) -D`uname -s` -DHAVE_SYS_TIME_H -c $< -o $*.o $(INCDIR)


all:	wmCalendar.o wmCalendar

wmCalendar: $(OBJECTS)
	 $(CC) $(COPTS)  $(SYSTEM) $(LFLAGS) -o $@ $(OBJECTS) -o wmCalendar $(INCDIR)$(LIBDIR) $(LIBS)

wmCalendar.o: wmCalendar_master.xpm   wmCalendar_master2.xpm
clean:
	for i in $(OBJECTS) ; do \
		rm -f $$i; \
	done
	rm -f wmCalendar

install:: wmCalendar
	install -c -s -m 0755 wmCalendar $(DESTDIR)/bin/
	install -c -m 0644 wmCalendar.1 $(DESTDIR)/man/man1

