prefix = /usr/local
BINDIR = $(prefix)/bin
MANDIR = $(prefix)/man
ETCDIR = $(prefix)/etc

# comment out both lines to get the old, Xpm-only, version
GDKPIXBUF = 1
#GDKPIXBUF2 = 1

XROOT = /usr/X11R6
DAROOT = /usr/local

# If you want to use gdk-pixbuf, pass GDKPIXBUF=1 to make command
ifdef GDKPIXBUF
PIXBUF_DEF = -DWITH_GDKPIXBUF
PIXBUF_INC := $(shell gdk-pixbuf-config --cflags)
PIXBUF_LIB := -rdynamic -L$(shell gdk-pixbuf-config --prefix)/lib -lgdk_pixbuf_xlib
endif

# If you want to use gdk-pixbuf-2.0, pass GDKPIXBUF2=1 to make command
ifdef GDKPIXBUF2
PIXBUF_CFG = /opt/gtk2/bin/pkg-config gdk-pixbuf-xlib-2.0
PIXBUF_DEF = -DWITH_GDKPIXBUF
PIXBUF_INC := $(shell $(PIXBUF_CFG) --cflags)
PIXBUF_LIB := -rdynamic -L$(shell $(PIXBUF_CFG) --variable=prefix)/lib -lgdk_pixbuf_xlib-2.0
endif

CC = gcc #-g
CFLAGS = -pipe $(CPPFLAGS) -O2
CPPFLAGS = -MMD -ansi -pedantic -Wall $(INCDIRS) $(DEFINES)
INCDIRS = -I$(DAROOT)/include -I$(XROOT)/include $(PIXBUF_INC)
DEFINES = -DETCDIR='"$(ETCDIR)"' $(PIXBUF_DEF)
LDFLAGS = -s
LIBDIRS = -L$(DAROOT)/lib -L$(XROOT)/lib $(PIXBUF_LIB)
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
	$(CC) $(CFLAGS) -c $<

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
