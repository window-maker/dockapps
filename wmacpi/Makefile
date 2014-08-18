# set options.  pick one, acpi or apm.  comment out the other one.  don't
# uncomment both, bad things will happen :)

OPT	:= -O2 

# uncomment this to make wmacpi use less system colors (looks uglier too)
#OPT	+= -DLOW_COLOR

# debugging options (don't bother with these)
#OPT	= -pg -g -DPRO -DACPI

CC	:= gcc
CFLAGS	:= $(OPT) -Wall -W -g -ansi -I/usr/X11R6/include
LDFLAGS := $(OPT) -L/usr/X11R6/lib -lX11 -lXpm -lXext

WMSRC	:= wmacpi-ng.c libacpi.c
CLSRC := acpi-ng.c libacpi.c
HEADERS := libacpi.h wmacpi-ng.h
targets := wmacpi-ng acpi-ng
doc_targets := debian/wmacpi-ng.1 debian/acpi-ng.1

PREFIX := /usr/local

all: $(targets)

# build the list of object files
WMOBJ := $(patsubst %.c,%.o,$(filter %.c,$(WMSRC)))
CLOBJ := $(patsubst %.c,%.o,$(filter %.c,$(CLSRC)))

# include per-file dependencies
include $(WMOBJ:.o=.d)
include $(CLOBJ:.o=.d)

wmacpi-ng:	$(WMOBJ)
	$(CC) $(LDFLAGS) -o $@ $^

acpi-ng: $(CLOBJ)
	$(CC) $(LDFLAGS) -o $@ $^

# build per-file dependencies - note that -MM may not be supported
# in gcc versions older than 2.95.4, but most likely is.
%.d: %.c
	gcc -MM $(CFLAGS) $< > $@

clean:
	rm -f TAGS *.o *~ trace *.out *.bb *.bbg 

clean-all: clean
	rm -f *.d $(targets)

install: $(targets)
	install -d $(PREFIX)/bin/
	install -pc $(targets) $(PREFIX)/bin/
	install -d $(PREFIX)/man/man1/
	install -pc $(doc_targets) $(PREFIX)/man/man1/

tags:
	etags $(WMSRC) $(CLSRC) $(HEADERS)
