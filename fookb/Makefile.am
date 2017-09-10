bin_PROGRAMS = fookb
fookb_SOURCES = fookb.c fookb.h globals.c images.c images.h opts.c opts.h \
	params.c params.h sound.c sound.h
dist_man_MANS = fookb.1x
dist_pkgdata_DATA = 1.xpm 2.xpm 3.xpm 4.xpm rus.xpm lat.xpm boom.xpm \
	En.xpm Ru.xpm beep_spring.au

AM_CFLAGS = $(Xlib_CFLAGS) $(libXpm_CFLAGS) $(WUtil_CFLAGS) -DHAVE_WINGS_WUTIL_H
LIBS += $(Xlib_LIBS) $(libXpm_LIBS) $(WUtil_LIBS)

EXTRA_DIST = copyright fookb.cf KNOWN_BUGS LICENSE README.russian.koi8-r \
	README.russian.utf-8 VERSION
