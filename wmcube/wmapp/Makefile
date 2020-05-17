export CXX     = g++
export CFLAGS  = -fPIC -g -Wall -pedantic -O2
export LFLAGS  = -L$(CURDIR) -L/usr/X11R6/lib -lXpm -lXext -lX11 -lwmapp

SOURCES = $(wildcard *.cc)
OBJECTS = $(SOURCES:.cc=.o)

libwmapp.a: $(OBJECTS)
	ar rcs libwmapp.a $^

%.d: %.cc
	@set -e; $(CXX) -MM $(CFLAGS) $< | sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
		   [ -s $@ ] || rm -f $@

-include $(SOURCES:.cc=.d)

%.o:
	$(CXX) $(CFLAGS) -c $< -o $@

wmexample: libwmapp.a
	$(MAKE) -C example1

wmatc: libwmapp.a
	$(MAKE) -C example2

clean:
	-rm -f tags *.o *.d *\~ libwmapp.a
	$(MAKE) -C example1 clean
	$(MAKE) -C example2 clean
	
dist-clean: clean
	-rm -f libwmapp.a wmexample wmatc

.PHONY:	all clean dist-clean wmexample wmatc

