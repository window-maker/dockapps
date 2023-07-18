# Simple makefile for the gtk-dock example
all: gtk-dock gtk-panel

gtk-dock: gtk-dock.o
	gcc `pkg-config gtk+-2.0 --libs` gtk-dock.o -o gtk-dock

gtk-dock.o: gtk-dock.c
	gcc -c -O2 -Wall `pkg-config gtk+-2.0 --cflags` gtk-dock.c


gtk-panel: gtk-panel.o
	gcc `pkg-config gtk+-2.0 --libs` gtk-panel.o -o gtk-panel

gtk-panel.o: gtk-panel.c
	gcc -c -O2 -Wall `pkg-config gtk+-2.0 --cflags` gtk-panel.c
clean:
	rm -f *.o gtk-panel gtk-dock *~