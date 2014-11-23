LIBPATH = /usr/X11R6/lib
LIBNAME = libfoodock
LFOO = $(LIBNAME).so

lib:	foodock.c foodock.h
	$(CC) -fpic -shared -o $(LFOO)  foodock.c `gtk-config --cflags`
	strip $(LFOO)

clean:
	rm -rf $(LFOO) foodock.o example.o example

install:
	cp $(LFOO) $(LIBPATH)
	chmod 644 $(LIBPATH)/$(LFOO)
	ldconfig

uninstall:
	rm -f $(LIBPATH)/$(LFOO)
	ldconfig

example: example.c
	$(CC) -c -o example.o example.c `gtk-config --cflags`
	$(CC) -o example example.o `gtk-config --libs` -lfoodock
