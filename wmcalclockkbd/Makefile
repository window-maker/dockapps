MAKE = make

all: src pixmaps
	$(MAKE) -C src
	$(MAKE) -C pixmaps

clean: src pixmaps
	$(MAKE) -C src clean
	$(MAKE) -C pixmaps clean

install: src pixmaps
	$(MAKE) -C src install
	$(MAKE) -C pixmaps install

dist:
	$(MAKE) clean
	cd ../; tar c wmCalClockKbd-0.1a > wmCalClockKbd-0.1a.tar; gzip wmCalClockKbd-0.1a.tar
