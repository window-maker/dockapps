bin_PROGRAMS=wmhdplop
noinst_PROGRAMS=@GKHDPLOP@
EXTRA_PROGRAMS=gkhdplop.so
dist_man_MANS = wmhdplop.1

EXTRA_DIST=autogen.sh gkrellm_hdplop.c

IMLIB2_LIBS=@IMLIB2_LIBS@
IMLIB2_CFLAGS=@IMLIB2_CFLAGS@

wmhdplop_SOURCES = wmhdplop.c wmhdplop.h util.c util.h procstat.c procstat.h devnames.c devnames.h global.h dockapp_imlib2.c dockapp_imlib2.h
wmhdplop_LDADD=-lm $(IMLIB2_LIBS)
wmhdplop_CFLAGS=$(IMLIB2_CFLAGS)
# gtk cflags and gtk lib flags
GTK2_CFLAGS = @GTK2_CFLAGS@
GTK2_LIBS = @GTK2_LIBS@

LIBTOOLSUX_GKHDPLOP_SRC=wmhdplop.c util.c util.h procstat.c procstat.h devnames.c devnames.h global.h dockapp_imlib2.c dockapp_imlib2.h gkrellm_hdplop.c

gkhdplop_so_SOURCES=$(LIBTOOLSUX_GKHDPLOP_SRC)
gkhdplop_so_CFLAGS=-DGKRELLM -fPIC $(GTK2_CFLAGS) $(IMLIB2_CFLAGS)
gkhdplop_so_LDADD=$(IMLIB2_LIBS) -lm
gkhdplop_so_LDFLAGS=-shared

all: @GKHDPLOP@
