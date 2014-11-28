fontdir = @FONTDIR@
dist_font_DATA = luxel-ascii-06x09.pcf.gz \
	seg7-ascii-05x07.pcf.gz

install-data-hook:
	@rm -f $(DESTDIR)$(fontdir)/font.dir
	$(MKFONTDIR) $(DESTDIR)$(fontdir)
	@RUN_FCCACHE@
