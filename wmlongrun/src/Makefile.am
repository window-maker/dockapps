bin_PROGRAMS = wmlongrun

wmlongrun_SOURCES =   \
	longrun_@OS@.c \
	longrun.h \
	main.c \
	dockapp.c \
	dockapp.h \
	common.h \
	backdrop_off.xpm \
	backdrop_on.xpm \
	backdrop_led.xpm \
	backdrop_off_g.xpm \
        backdrop_on_g.xpm \
	backdrop_led_g.xpm \
	parts.xpm

EXTRA_wmlongrun_SOURCES = \
	longrun_linux.c \
	longrun_freebsd.c

AM_CPPFLAGS = \
	@CPPFLAGS@ \
	@DFLAGS@ \
        @HEADER_SEARCH_PATH@

wmlongrun_LDADD = \
	@LIBRARY_SEARCH_PATH@ \
	@XLIBS@

INSTALL_PROGRAM = \
	@INSTALL_PROGRAM@ \
	@SETUID_FLAGS@ \
	$(AM_INSTALL_PROGRAM_FLAGS)
