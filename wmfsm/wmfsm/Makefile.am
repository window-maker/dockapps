bin_PROGRAMS = wmfsm
if GETOPT
wmfsm_SOURCES = wmfsm.c
else
wmfsm_SOURCES = wmfsm.c getopt.c getopt1.c getopt.h
endif
wmfsm_LDADD = ../wmgeneral/libwmgeneral.a @LIBS@ @X11LIBS@ @X_LIBS@ @X_EXTRA_LIBS@
man_MANS = wmfsm.1
EXTRA_DIST = $(man_MANS) wmfsm_master_highcolor.xpm wmfsm_master_cyan.xpm wmfsm_master_lowcolor.xpm wmfsm_mask.xbm wmfsmrc.sample
CLEANFILES = wmfsm_master.xpm

