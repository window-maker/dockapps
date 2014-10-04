bin_PROGRAMS = wmmemload

wmmemload_SOURCES =   \
	main.c \
	mem.h \
	mem_@OS@.c \
	dockapp.c \
	dockapp.h \
	backdrop_off.xpm \
	backdrop_on.xpm \
	parts_mem.xpm \
	parts_swap.xpm

EXTRA_wmmemload_SOURCES = \
	mem_linux.c \
	mem_freebsd.c \
	mem_freebsd_sysctl.c \
	mem_openbsd.c \
	mem_solaris.c

AM_CPPFLAGS = @DFLAGS@ @HEADER_SEARCH_PATH@

wmmemload_LDADD = @LIBRARY_SEARCH_PATH@ @XLIBS@

INSTALL_PROGRAM = \
	@INSTALL_PROGRAM@ \
	@SETGID_FLAGS@ \
	$(AM_INSTALL_PROGRAM_FLAGS)
