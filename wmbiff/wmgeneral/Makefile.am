noinst_LIBRARIES = libwmgeneral.a
libwmgeneral_a_SOURCES = misc.c misc.h wmgeneral.c wmgeneral.h

MAINTAINERCLEANFILES = Makefile.in


indent:
	indent -npro -kr -i4 -ts4 $(libwmgeneral_a_SOURCES) || true

dist-hook-local: indent
