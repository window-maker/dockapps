# This file is processed by GNU automake to generate Makefile.in

INCLUDES = -I$(top_srcdir)/pixmaps

bin_PROGRAMS = wmnotify
wmnotify_SOURCES = wmnotify.c wmnotify.h options.c options.h configfile.c configfile.h \
                   xevents.c xevents.h dockapp.c dockapp.h sound.c sound.h \
                   network.c network.h pop3.c pop3.h imap.c imap.h ssl.c ssl.h common.h

CLEANFILES     = *~
DISTCLEANFILES = .deps/*.P
MAINTAINERCLEANFILES = Makefile.in

# we want these in the dist tarball
EXTRA_DIST = 
