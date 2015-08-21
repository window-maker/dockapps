bin_PROGRAMS = wmfsm
if GETOPT
wmfsm_SOURCES = wmfsm.c
else
wmfsm_SOURCES = wmfsm.c getopt.c getopt1.c getopt.h
endif
wmfsm_LDADD = @LIBS@ @X11LIBS@ @X_LIBS@ @X_EXTRA_LIBS@
man_MANS = wmfsm.1
dist_pkgdata_DATA = wmfsm_master_highcolor.xpm wmfsm_master_cyan.xpm \
	wmfsm_master_lowcolor.xpm
EXTRA_DIST = $(man_MANS) wmfsm_mask.xbm wmfsmrc.sample
DISTCLEANFILES = wmfsm_master.xpm
