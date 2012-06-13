#
# Mixer.app Makefile
#

DESTDIR =
GNUSTEP_BINDIR = /usr/local/GNUstep/Apps/AlsaMixer.app
X11_BINDIR = /usr/X11R6/bin

CXX=c++
CXXFLAGS += -Wall -pedantic -fno-rtti -fno-exceptions -O2 -I/usr/X11R6/include
LDFLAGS += -L/usr/X11R6/lib -lXpm -lXext -lX11 -lasound

OBJECTS = Main.o Mixer.o Xpm.o AMixer/AMixer.o AMixer/AItem.o AMixer/AChannel.o

all: AlsaMixer.app

AlsaMixer.app: $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

.PHONY:	install clean distclean

install: install-gnustep

install-gnustep: all
	install -d  $(DESTDIR)$(GNUSTEP_BINDIR)
	install -m 0755 AlsaMixer.app $(DESTDIR)$(GNUSTEP_BINDIR)/AlsaMixer

install-x11: all
	install -d  $(DESTDIR)$(X11_BINDIR)
	install -m 0755 AlsaMixer.app $(DESTDIR)$(X11_BINDIR)/AlsaMixer.app

clean:
	rm -f *~ $(OBJECTS) AlsaMixer.app

# End of file
