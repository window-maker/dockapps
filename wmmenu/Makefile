prefix = /usr/local
BINDIR = $(prefix)/bin
MANDIR = $(prefix)/man
ETCDIR = $(prefix)/etc

# comment to get the old, Xpm-only, version
GDKPIXBUF2 = 1

# If you want to use gdk-pixbuf-2.0, pass GDKPIXBUF2=1 to make command
ifdef GDKPIXBUF2
PIXBUF_CFG = pkg-config gdk-pixbuf-xlib-2.0
PIXBUF_DEF = -DWITH_GDKPIXBUF
PIXBUF_INC := $(shell $(PIXBUF_CFG) --cflags)
PIXBUF_LIB := -rdynamic -L$(shell $(PIXBUF_CFG) --variable=prefix)/lib -lgdk_pixbuf_xlib-2.0 -lgdk_pixbuf-2.0 -lgobject-2.0
endif

CC = gcc #-g
CFLAGS = -pipe -O2
CPPFLAGS = -MMD -ansi -pedantic -Wall
INCDIRS = $(PIXBUF_INC)
DEFINES = -DETCDIR='"$(ETCDIR)"' $(PIXBUF_DEF)
LDFLAGS = -s
LIBDIRS = $(PIXBUF_LIB)
LDLIBS = -ldockapp -lXpm -lXext -lX11 -lm

MKDIRHIER = mkdir -p
MAN1DIR = $(MANDIR)/man1

SRCS1 = \
	wmmenu.c options.c error.c menu.c utils.c pixmaps.c \
	xobjects.c buttonbar.c events.c

OBJS1 = $(SRCS1:.c=.o)
PRGS = wmmenu
MAN1 = $(PRGS:%=%.1)

RM = rm -f
INSTBIN = install -c
INSTMAN = install -c

all : $(PRGS)

.c.o :
	$(CC) $(INCDIRS) $(DEFINES) $(CPPFLAGS) $(CFLAGS) -c $<

wmmenu : $(OBJS1)
	$(CC) $(LDFLAGS) -o $@ $(OBJS1) $(LIBDIRS) $(LDLIBS)

install : $(PRGS) $(MAN1)
	$(MKDIRHIER) $(BINDIR)
	$(INSTBIN) $(PRGS) $(BINDIR)
	$(MKDIRHIER) $(MAN1DIR)
	$(INSTMAN) $(MAN1) $(MAN1DIR)

clean :
	$(RM) *.o *.d core tags $(PRGS)

# dependencies -- do not change lines below
-include *.d
