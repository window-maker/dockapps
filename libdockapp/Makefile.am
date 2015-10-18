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
	@echo 'URL: http://windowmaker.org/dockapps/?name=libdockapp' >> $@
	@echo 'Version: $(VERSION)' >> $@
	@echo 'Requires: x11 xext xpm' >> $@
	@echo 'Libs: -L${libdir} -ldockapp' >> $@
	@echo 'Cflags: -I${includedir}' >> $@

EXTRA_DIST = BUGS

update-changelog:
	@git log `git log -1 --pretty=%h -- ChangeLog`..HEAD \
		--pretty --numstat --summary -- . | \
		git2cl | \
		sed 's/libdockapp: \?//g' | \
		sed 's/libdockapp\///g' > ChangeLog.new
	@mv ChangeLog ChangeLog.old
	@cat ChangeLog.new ChangeLog.old > ChangeLog
	@rm ChangeLog.new ChangeLog.old
