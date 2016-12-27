PREFIX = $(shell cat ../PREFIX)

all:

clean:

install: *.xpm
	install -d -m 0755 $(PREFIX)/share/wmCalClockKbd/
	for i in *.xpm; do \
	  install -c -m 0644 $$i $(PREFIX)/share/wmCalClockKbd/; \
	done
	install -c -m 0644 COPYING $(PREFIX)/share/wmCalClockKbd/
