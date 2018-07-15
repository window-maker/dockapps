bin_PROGRAMS = wmthrottle

wmthrottle_SOURCES =   \
	throttle.c \
	dockapp.c \
	dockapp.h \
	backdrop_off.xpm \
	backdrop_on.xpm \
	mouse_regions.c \
	mouse_regions.h \
	parts.xpm

CPPFLAGS = @CPPFLAGS@ @DFLAGS@

INCLUDES = @HEADER_SEARCH_PATH@

wmthrottle_LDADD = @LIBRARY_SEARCH_PATH@ @XLIBS@

INSTALL_PROGRAM = \
	@INSTALL_PROGRAM@ \
	@SETGID_FLAGS@ \
	$(AM_INSTALL_PROGRAM_FLAGS)
