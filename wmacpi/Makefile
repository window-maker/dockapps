# set options.  pick one, acpi or apm.  comment out the other one.  don't
# uncomment both, bad things will happen :)

OPT	:= -O2

# uncomment this to build the command line acpi tool
BUILD_CLI = 1

# uncomment this to make wmacpi use less system colors (looks uglier too)
#OPT	+= -DLOW_COLOR

# debugging options (don't bother with these)
#OPT	= -pg -g

CC	:= gcc
CFLAGS	+= $(OPT) -Wall -W -g -ansi
LDFLAGS += $(OPT) -lX11 -ldockapp

WMSRC	:= wmacpi.c libacpi.c
HEADERS := libacpi.h wmacpi.h
targets := wmacpi
doc_targets := wmacpi.1

ifdef BUILD_CLI
targets += wmacpi-cli
doc_targets += wmacpi-cli.1
endif

PREFIX := /usr/local

all: $(targets)

# build the list of object files
WMOBJ := $(patsubst %.c,%.o,$(filter %.c,$(WMSRC)))

# include per-file dependencies
-include $(WMOBJ:.o=.d)

wmacpi:	$(WMOBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

# for the Debian package, we want to make building the command line tools
# optional. So, we hide all the necessary stuff here . . .
ifdef BUILD_CLI
CLSRC := wmacpi-cli.c libacpi.c
CLOBJ := $(patsubst %.c,%.o,$(filter %.c,$(CLSRC)))
-include $(CLOBJ:.o=.d)

wmacpi-cli: $(CLOBJ)
	$(CC) $(LDFLAGS) -o $@ $^
endif

# build per-file dependencies - note that -MM may not be supported
# in gcc versions older than 2.95.4, but most likely is.
%.d: %.c
	gcc -MM $(CFLAGS) $< > $@

clean:
	rm -f TAGS *.o *~ trace *.out *.bb *.bbg
	rm -f *.d $(targets)

clean-all: clean

install: $(targets)
	install -d $(DESTDIR)$(PREFIX)/bin/
	install -pc $(targets) $(DESTDIR)$(PREFIX)/bin/
	install -d $(DESTDIR)$(PREFIX)/share/man/man1/
	install -pc $(doc_targets) $(DESTDIR)$(PREFIX)/share/man/man1/

tags:
	etags $(WMSRC) $(CLSRC) $(HEADERS)
