OBJECTS = `find ./* -name '*.o'`

bin_PROGRAMS = wmwifi 

wmwifi_SOURCES =   \
	wmwifi.c \
	wireless.c \
	dockapp.c \
	dockapp.h \
	backdrop_off.xpm \
	backdrop_on.xpm
#wmwifi_LDADD = -L/usr/X11R6/lib -L/usr/local/lib -lX11  -lXext -lXpm 
#wmupmon_LDADD = @LIBRARY_SEARCH_PATH@ @XLIBS@
wmwifi_LDFLAGS =  -I/usr/include
