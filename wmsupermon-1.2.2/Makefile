LIBDIR = -L/usr/X11R6/lib
LIBS   = -lXpm -lXext -lX11 -lm
OBJS =  wmsupermon.o dockapp.o stat_dev.o panes.o expr.o
FLAGS= -O2 -Wall
#FLAGS= -g -Wall -O0

all: wmsupermon

wmsupermon: $(OBJS)
	$(CC) -o wmsupermon $^ $(LIBDIR) $(LIBS) $(LFLAGS)

clean:
	$(RM) *.o wmsupermon core* *.core expr.c tags

tarball:
	grep -F '"$(subst wmsupermon-,,$(notdir $(CURDIR)))"' wmsupermon.c > /dev/null
	grep -F '"$(shell date +%Y/%m/%d)"' wmsupermon.c > /dev/null
	cd .. && tar cvzf $(notdir $(CURDIR)).tar.gz $(notdir $(CURDIR)) \
	--exclude '*.o' --exclude tags --exclude '*core*' --exclude expr.c \
	--exclude wmsupermon --exclude '.*.swp'

.c.o:
	$(CC) -c $(FLAGS) $< -o $*.o

# generated with gcc -MM *.c
dockapp.o: dockapp.c dockapp.h
expr.o: expr.c expr.h
panes.o: panes.c expr.h panes.h stat_dev.h
stat_dev.o: stat_dev.c expr.h stat_dev.h
wmsupermon.o: wmsupermon.c dockapp.h wmsupermon-master.xpm expr.h stat_dev.h panes.h

