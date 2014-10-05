TESTS = test_wmbiff test_tlscomm
noinst_PROGRAMS = test_wmbiff test_tlscomm
bin_PROGRAMS = wmbiff
wmbiff_SOURCES = wmbiff.c socket.c Pop3Client.c mboxClient.c \
	maildirClient.c Imap4Client.c tlsComm.c tlsComm.h ShellClient.c  \
	passwordMgr.c passwordMgr.h charutil.c charutil.h Client.h  \
	regulo.c regulo.h  MessageList.c MessageList.h
EXTRA_wmbiff_SOURCES = gnutls-common.c gnutls-common.h
wmbiff_LDADD = -L../wmgeneral -lwmgeneral @LIBGCRYPT_LIBS@ @GNUTLS_COMMON_O@
wmbiff_DEPENDENCIES = ../wmgeneral/libwmgeneral.a Makefile @GNUTLS_COMMON_O@
test_wmbiff_SOURCES = ShellClient.c charutil.c charutil.h Client.h \
	test_wmbiff.c passwordMgr.c Imap4Client.c regulo.c Pop3Client.c \
	tlsComm.c tlsComm.h socket.c
test_tlscomm_SOURCES = test_tlscomm.c \
	tlsComm.c tlsComm.h
EXTRA_test_wmbiff_SOURCES = gnutls-common.c gnutls-common.h
test_wmbiff_LDADD = @LIBGCRYPT_LIBS@
man_MANS = wmbiff.1 wmbiffrc.5
skindir = $(datadir)/wmbiff/skins
skin_DATA = wmbiff-master-led.xpm wmbiff-master-contrast.xpm \
	wmbiff-classic-master-led.xpm wmbiff-classic-master-contrast.xpm

EXTRA_DIST = $(man_MANS) sample.wmbiffrc wmbiff-master-led.xpm \
	wmbiff-classic-master-led.xpm

MAINTAINERCLEANFILES = Makefile.in

# CODING STYLE AND INDENTATION [2001-Mar-12]:
#
# There have been some problems with coding style in the past.  Many people
# contributed to wmbiff (thank you!), and the code got really messy.  To help
# resolve this, I used GNU indent with what I believe to be the most
# widely-accepted coding style options (K&R style) with 4-space TAB indents
# (because some of the code is highly nested) to clean up the code.  Not
# everyone may be happy with this, but has been determined to be necessary for
# consistency and legibility.
#
# In other words, make sure you run "make clean" and "make indent", and do not
# change the options on the indent command, before you submit patches against
# wmbiff.  This will make everyone's life easier.
#
#	-- Dwayne C. Litzenberger <dlitz@dlitz.net>
indent:
	indent -npro -kr -i4 -ts4 $(wmbiff_SOURCES) test_*.c || true

# to perform surgery on a few changed files.
.c.indent:
	indent -npro -kr -i4 -ts4 $< || true

dist-hook-local: indent config-h-check

distclean-local:
	-rm -f wmbiff-master-contrast.xpm wmbiff-master.xpm \
		wmbiff-classic-master-contrast.xpm

# remove colors, then substitute old colors, then repalletize
# for some reason $< doesn't always work.
wmbiff-master-contrast.xpm:  wmbiff-master-led.xpm Makefile
	egrep -v '^"[:%][[:space:]]c #' < wmbiff-master-led.xpm | \
	sed -e 's/:/./g' -e 's/%/$$/g' -e 's/ 15 / 13 /' | \
	sed -e 's/#\([0-9A-F]\{2\}\)[0-9A-F]\{2\}\([0-9A-F]\{2\}\)[0-9A-F]\{2\}\([0-9A-F]\{2\}\)[0-9A-F]\{2\}/#\1\2\3/'\
	> $@ || rm $@

wmbiff-classic-master-contrast.xpm:  wmbiff-classic-master-led.xpm Makefile
	egrep -v '^"[:%][[:space:]]c #' < wmbiff-classic-master-led.xpm | \
	sed -e 's/:/./g' -e 's/%/$$/g' -e 's/ 15 / 13 /' | \
	sed -e 's/#\([0-9A-F]\{2\}\)[0-9A-F]\{2\}\([0-9A-F]\{2\}\)[0-9A-F]\{2\}\([0-9A-F]\{2\}\)[0-9A-F]\{2\}/#\1\2\3/'\
	> $@ || rm $@

# fail if there's a .c file that doesn't include config.h
config-h-check:
	ls *.c | sort > cfiles
	grep -l config.h *.c | sort | diff - cfiles
	rm cfiles

# just a reminder of how to run valgrind to get decent output.
valgrind:
	valgrind --leak-check=yes ./wmbiff -exit

