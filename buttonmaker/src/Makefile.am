## Process this file with automake to produce Makefile.in

## Created by Anjuta

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" $(BUTTONMAKER_CFLAGS) \
	$(DOCKAPP_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = buttonmaker

buttonmaker_SOURCES = \
	main.c

buttonmaker_LDFLAGS =

buttonmaker_LDADD = \
	$(BUTTONMAKER_LIBS) $(DOCKAPP_LIBS) \
	-lX11 -lXpm -lXext \
	$(X_LIBS) \
	$(X_EXTRA_LIBS)

