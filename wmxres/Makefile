# Edited for Debian GNU/Linux
DESTDIR = 

CC		=	gcc
CFLAGS 		=	-O2 -Wall -fstack-protector -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security
FINAL_CFLAGS 		=	-O2 -Wall -fstack-protector -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Wl,-z,norelro
DEBUG_CFLAGS	=	-g -Wall
INCDIR		=	-I/usr/X11R6/include
LIBDIR		=	-L/usr/X11R6/lib
LIBS		=	-lXpm -lXext -lX11 -lXxf86vm
OBJS		=	wmxres/wmxres.o \
			wmgeneral/wmgeneral.o \

# ifneq (,$(findstring debug,$(DEB_BUILD_OPTIONS)))
# CFLAGS += -g
# endif

ifeq (,$(findstring nostrip,$(DEB_BUILD_OPTIONS)))
  INSTALL_FLAGS += -s
endif

.c.o:
	cc -c $(CFLAGS) -D$(shell echo `uname -s`) $< -o $*.o $(INCDIR)

all: wmxres/wmxres

wmxres/wmxres: $(OBJS) 
	gcc $(CFLAGS) -o wmxres/wmxres $^ $(LIBDIR) $(LIBS)

debug:  $(OBJS)
	gcc $(DEBUG_CFLAGS) -o wmxres/wmxres $^ $(LIBDIR) $(LIBS)
install:: all
	install $(INSTALL_FLAGS) -m 4755 -o root wmxres/wmxres \
          $(DESTDIR)/usr/bin

.PHONY :clean
clean:
	for i in $(OBJS) ; do \
		rm -f $$i; \
	done
	rm -f wmxres/wmxres
