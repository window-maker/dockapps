LIBDIR = -L/usr/X11R6/lib
LIBS   = -lXpm -lXext -lX11
OBJS =  wmisdn.o regions.o
HS = optics/optics.h
VER = 1.81
DIR = wmisdn-$(VER)

.cc.o:
	g++ -c -Wall -DVER=$(VER) $< -o $*.o

wmisdn: $(OBJS)
	g++ -o wmisdn $^ $(LIBDIR) $(LIBS)

install:: wmisdn
	install -d /usr/local/sbin/
	install -s -o root -g root -m 0700 wmisdn /usr/local/sbin/wmisdn
	install -d /usr/local/doc/wmisdn/
	install -m 0644 CHANGES COPYING README /usr/local/doc/wmisdn/

clean::
	for i in $(OBJS); do rm -f $$i; done
	rm -f wmisdn
	rm -f *~

dist::
	cd ../ ; tar --exclude=$(DIR)/stuff --exclude=$(DIR)/dist -c $(DIR)/ | gzip -c > $(DIR)/dist/wmisdn-$(VER).tgz

build::
	make clean
	make
