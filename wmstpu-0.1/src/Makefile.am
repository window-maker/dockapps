## Process this file with automake to produce Makefile.in

## Created by Anjuta - will be overwritten
## If you don't want it to overwrite it,
## 	Please disable it in the Anjuta project configuration

INCLUDES = \
	$(X_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = wmstpu

wmstpu_SOURCES = \
	wmgeneral.h\
	wmgeneral.c\
	main.c

wmstpu_LDFLAGS = 

wmstpu_LDADD =  \
	-lX11 -lXpm -lXext \
	$(X_LIBS) \
	$(X_EXTRA_LIBS)

