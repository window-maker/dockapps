if COND_FONT
COND_FONT = fonts
else
COND_FONT =
endif

if COND_EXAMPLES
COND_XMPS = examples
endif

SUBDIRS = src $(COND_FONT) $(COND_XMPS)

pkgconfigdir = $(libdir)/pkgconfig
dist_pkgconfig_DATA = dockapp.pc

dockapp.pc: Makefile
	@echo "Generating $@"
	@echo 'Name: dockapp' >	 $@
	@echo 'Description: Window Maker dockapp support' >> $@
	@echo 'URL: http://libdockapp.sourceforge.net' >> $@
	@echo 'Version: $(VERSION)' >> $@
	@echo 'Requires: x11 xext xpm' >> $@
	@echo 'Libs: -L${libdir} -ldockapp' >> $@
	@echo 'Cflags: -I${includedir}' >> $@

EXTRA_DIST = BUGS
