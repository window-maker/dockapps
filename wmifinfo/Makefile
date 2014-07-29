
# Set this to 'y' if you want support for reading the link quality
# of nowiresneeded 1148 PCMCIA or Swallow cards

ENABLE_NWN_SUPPORT=n


##################################################################
# Nothing to configure under here

NAME=wmifinfo
VERSION=0.09

CC = gcc
LD = gcc
COPTS = -Wall -O2 -D'VERSION="$(VERSION)"' -D'NAME="$(NAME)"'
LDOPTS = -lX11 -L/usr/X11R6/lib -lXpm -lXext
BINDIR = /usr/local/bin

BIN =	wmifinfo
FILES = wmifinfo.o xutils.o 

ifeq ("$(ENABLE_NWN_SUPPORT)", "y")
FILES += nwn.o
COPTS += -DENABLE_NWN_SUPPORT
endif

all:	$(BIN)

.c.o: 	
	$(CC) $(COPTS) -c $< 

$(BIN):	$(FILES)
	$(LD) -o $@ $(FILES) $(LDOPTS)

clean:
	rm -f *.o $(BIN) core ./.#* *.orig *.rej

install:
	cp $(BIN) $(BINDIR)

dist:	clean
	rm -rf /tmp/wmifinfo-$(VERSION)
	cd .. && cp -a wmifinfo /tmp/wmifinfo-$(VERSION)
	cd /tmp && tar --exclude CVS -zcvf wmifinfo-$(VERSION).tgz wmifinfo-$(VERSION)/
	
