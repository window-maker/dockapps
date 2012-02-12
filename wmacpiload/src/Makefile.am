bin_PROGRAMS = wmacpiload

wmacpiload_SOURCES =   \
	main.c \
	dockapp.c \
	dockapp.h \
	backdrop_off.xpm \
	backdrop_on.xpm \
	parts.xpm

CPPFLAGS = @CPPFLAGS@ @DFLAGS@

INCLUDES = @HEADER_SEARCH_PATH@

wmacpiload_LDADD = @LIBRARY_SEARCH_PATH@ @XLIBS@

INSTALL_PROGRAM = \
	@INSTALL_PROGRAM@ \
	@SETGID_FLAGS@ \
	$(AM_INSTALL_PROGRAM_FLAGS)
