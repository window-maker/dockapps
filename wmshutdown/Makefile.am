bin_PROGRAMS = wmshutdown
wmshutdown_SOURCES = wmshutdown.c wmshutdown.xpm

AM_CFLAGS = $(gtk_CFLAGS) $(x11_CFLAGS)
AM_CPPFLAGS =  -DDATADIR=\"$(pkgdatadir)\"
LIBS += $(gtk_LIBS) $(x11_LIBS)

dist_man_MANS = wmshutdown.1
