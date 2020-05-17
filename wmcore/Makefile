INSTALL	 = /usr/bin/install
LD	 = $(CC)

CFLAGS	+= -Wall -O3 \
	   $(shell pkg-config dockapp --cflags) \
	   $(shell pkg-config x11     --cflags) \
	   $(shell pkg-config xext    --cflags) \
	   $(shell pkg-config xpm     --cflags)
LDFLAGS += -Wall

LIBS	 = $(shell pkg-config dockapp --libs) \
	   $(shell pkg-config x11     --libs) \
	   $(shell pkg-config xext    --libs) \
	   $(shell pkg-config xpm     --libs)

OBJS	 = wmcore.o \
	   misc.o \
	   list.o

all : wmcore

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $*.o

wmcore: $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f $(OBJS) wmcore

install:
	$(INSTALL) -m 0755 wmcore $(DESTDIR)/usr/bin

uninstall:
	rm -f /usr/bin/wmcore
