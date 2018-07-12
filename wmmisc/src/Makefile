include general.mk

#CPPFLAGS   += -DUSE_MTRACE

objects     = wmgeneral.o     \
	      dockapp_draw.o  \
	      dockapp_utils.o \
	      dockapp_main.o

headers     = wmgeneral.h     \
	      dockapp_draw.h  \
	      dockapp_utils.h \
	      dockapp_main.h

sources     = wmgeneral.c     \
	      dockapp_draw.c  \
	      dockapp_utils.c \
	      dockapp_main.c

package     = wmmisc

all: $(package)

$(package): $(objects)
	rm -f $(package)
	$(CC) $(CFLAGS) -o $(package) $(objects) $(LDFLAGS)

wmgeneral.o: wmgeneral.c wmgeneral.h
	rm -f wmgeneral.o
	$(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c -o wmgeneral.o wmgeneral.c

dockapp_draw.o: dockapp_draw.c wmgeneral.h dockapp_draw.h
	rm -f dockapp_draw.o
	$(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c -o dockapp_draw.o dockapp_draw.c

dockapp_utils.o: dockapp_utils.c wmgeneral.h dockapp_draw.h dockapp_utils.h
	rm -f dockapp_utils.o
	$(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c -o dockapp_utils.o dockapp_utils.c

dockapp_main.o: dockapp_main.c wmgeneral.h dockapp_draw.h dockapp_utils.h dockapp_main.h
	rm -f dockapp_main.o
	$(CC) $(CPPFLAGS) $(INCLUDES) $(CFLAGS) -c -o dockapp_main.o dockapp_main.c

install:
	install -c -s -o root -g root -m 755 $(package) $(BINDIR)

uninstall:
	rm -f $(BINDIR)/$(package)

clean:
	rm -f $(package) $(objects) *~ core core.* gmon.out
