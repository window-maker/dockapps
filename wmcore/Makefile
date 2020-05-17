INSTALL	 = /usr/bin/install
LD	 = $(CC)

CFLAGS	+= -Wall -O3
LDFLAGS += -Wall

LIBDIR	 = -L/usr/X11R6/lib
LIBS	 = -lXpm -lXext -lX11 -ldockapp

OBJS	 = wmcore.o \
	   misc.o \
	   list.o

all : wmcore

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $*.o

wmcore: $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBDIR) $(LIBS)

clean:
	rm -f $(OBJS) wmcore

install:
	$(INSTALL) -m 0755 wmcore $(DESTDIR)/usr/bin

uninstall:
	rm -f /usr/bin/wmcore
