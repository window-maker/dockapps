## Process this file with automake to produce Makefile.in

## Created by Anjuta - will be overwritten
## If you don't want it to overwrite it,
## 	Please disable it in the Anjuta project configuration

INCLUDES = \
	$(X_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = wmWeatherApp

wmWeatherApp_SOURCES = \
	wmgeneral.h\
	wmgeneral.c\
	pixmaps.h\
	main.c

wmWeatherApp_LDFLAGS = 

wmWeatherApp_LDADD =  \
	-lX11 -lXpm -lXext \
	$(X_LIBS) \
	$(X_EXTRA_LIBS)

