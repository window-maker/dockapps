# Copyright (c) 2001-2003 Aaron Trickey <aaron@amtrickey.net>
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
# AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
# AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

# Makefile for the ``wmget'' (formerly ``wmcurl'') project.
# This Makefile requires GNU make and probably other GNU stuff...


########################################################################
# Build Targets:
#
# all [default]:	Builds the wmget application and documentation
# install:		Installs the application
# uninstall:		Attempts to uninstall, if this makefile installed it
# dockapplib:		Recurses into the dockapp dir and builds the library
# doc:			Builds all documentation (okay, it's only a manpage)
# clean:		Cleans, except for generated HTML/man docs
# docclean:		Cleans generated docs
# slackpkg:		Builds a Slackware package in packages/slackware
# sourceball:		Builds a source+docs tarball in packages/source
########################################################################

all:		wmget doc

.PHONY:		all install uninstall dockapplib doc clean \
		docclean slackpkg sourceball


##### BUILD SETTINGS AND VARIABLES #####################################

# To specify a different prefix, you can override this on the command line
#	make PREFIX=/opt/dockapps install
PREFIX=		/usr/local

INSTALLDIR=	install -d -m 755
INSTALLBIN=	install -m 555
INSTALLMAN=	install -m 444
CC=		gcc
CFLAGS=		-Wall -W -I/usr/X11R6/include -O
# The following line is what I use during development
#CFLAGS:=	$(CFLAGS) -Werror -g 
LDFLAGS=	-L/usr/X11R6/lib -lXpm -lXext -lX11 -lm -lcurl
DOCS=		wmget.1

VERSION:=	$(shell grep '\#define WMGET_VERSION ' wmget.h \
			| sed -e 's/.*"\(.*\)".*/\1/' )

OBJS=		server.o \
		request.o \
		cancel.o \
		list.o \
		retrieve.o \
		iq.o \
		wmget.o \
		configure.o \
		messages.o \
		usage.o

DALIBDIR=	dockapp
DALIB=		$(DALIBDIR)/dockapp.a

ALL_SRCS=	$(subst .o,.c,$(OBJS))


##### PROGRAM ##########################################################

install:	all
		echo $(PREFIX) > install.prefix ; \
		$(INSTALLDIR) $(PREFIX)/bin ; \
		$(INSTALLBIN) wmget $(PREFIX)/bin/wmget ; \
		$(INSTALLDIR) $(PREFIX)/man/man1 ; \
		$(INSTALLMAN) wmget.1 $(PREFIX)/man/man1/wmget.1 ; \

uninstall:
		cd `cat install.prefix` && rm -f bin/wmget man/man1/wmget.1
		rm -f install.prefix

wmget:		dockapplib $(OBJS)
		$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(DALIB) -o $@


##### LIBRARY ##########################################################

dockapplib:
		make -C $(DALIBDIR)



##### DOCUMENTATION ####################################################

doc:		$(DOCS)


# NOTE: The wmget refentry page uses an <?xml-stylesheet?> PI, I use an XML
# catalog file to map the given URL to a local path

wmget.1:	wmget.refentry.xml
		xsltproc --nonet $<


##### CLEANUP ##########################################################

clean:
		rm -f *.o wmget core pod2html-* install.prefix \
			wmget.html
		make -C dockapp clean

docclean:
		rm -f $(DOCS)


##### SLACKWARE PACKAGE ################################################

SLACK_PFX=	slackroot/$(PREFIX)
SLACK_PACKAGE=	packages/slackware/wmget-$(VERSION).tgz

slackpkg:	wmget doc
		-mkdir -p packages/slackware
		rm -rf slackroot
		$(INSTALLDIR) $(SLACK_PFX)
		$(INSTALLDIR) $(SLACK_PFX)/bin
		$(INSTALLDIR) $(SLACK_PFX)/man
		$(INSTALLDIR) $(SLACK_PFX)/man/man1
		$(INSTALLBIN) wmget $(SLACK_PFX)/bin/wmget
		$(INSTALLMAN) wmget.1 $(SLACK_PFX)/man/man1/wmget.1
		cd slackroot && \
		tar czv	-f ../$(SLACK_PACKAGE) 	\
			--owner=root 		\
			--group=root 		\
			*
		rm -rf slackroot


##### SOURCE PACKAGE ###################################################

SOURCEBALL=	packages/source/wmget-$(VERSION)-src.tar.gz

sourceball:	doc clean
		-mkdir -p packages/source
		cd .. &&			\
		tar czv	--exclude RCS 		\
			--exclude .\*		\
			--exclude packages	\
			--exclude tags		\
			--exclude \*~		\
			--exclude working	\
			--exclude www		\
			-f wmget/$(SOURCEBALL) 	\
			wmget


##### WEB SITE #########################################################

WWW_SRC=	$(HOME)/amtrickey.net/src
WWW_DOWNLOAD=	$(WWW_SRC)/download
WWW_WMGET=	$(WWW_SRC)/wmget

www:		slackpkg sourceball
		cp $(SLACK_PACKAGE) $(WWW_DOWNLOAD)
		cp $(SOURCEBALL) $(WWW_DOWNLOAD)
		cp NEWS wmget.refentry.xml $(WWW_WMGET)



