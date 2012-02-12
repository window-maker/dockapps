# $Id: Makefile,v 1.3 2002/08/16 17:22:26 essmann Exp $
#
# Copyright (c) 2001 Bruno Essmann <essmann@users.sourceforge.net>
# All rights reserved.

.SILENT:

INSTALLDIR = /usr/local

all: wmpager

wmpager:
	@echo "*** `pwd`: $@" ; \
	cd src ; \
	$(MAKE) $@

clean:
	@echo "*** `pwd`: $@" ; \
	cd src ; \
	$(MAKE) $@

install:
	echo "*** `pwd`: $@" ; \
	if [ ! -f src/wmpager ] ; then \
		$(MAKE) wmpager ; \
	fi ; \
	if [ -z "`id | grep 'uid=.*(root)'`" ] ; then \
		echo "you must be root to install wmpager!" ; \
		exit ; \
	fi ; \
	for i in bin/wmpager man/man1/wmpager.1x ; do \
		if [ -f ${INSTALLDIR}/$$i ] ; then \
			echo "there's already a $$i in ${INSTALLDIR}" ; \
			echo "installation aborted." ; \
			exit ; \
		fi ; \
	done ; \
	if [ ! -d ${INSTALLDIR}/bin ] ; then \
		mkdir -p ${INSTALLDIR}/bin ; \
	fi ; \
	echo "copying wmpager to ${INSTALLDIR}/bin" ; \
	cp src/wmpager ${INSTALLDIR}/bin/wmpager ; \
	chmod 755 ${INSTALLDIR}/bin/wmpager ; \
	if [ ! -d ${INSTALLDIR}/man/man1 ] ; then \
		mkdir -p ${INSTALLDIR}/man/man1 ] ; \
	fi ; \
	echo "copying manual page to ${INSTALLDIR}/man/man1" ; \
	cp man/man1/wmpager.1x ${INSTALLDIR}/man/man1/wmpager.1x ; \
	chmod 644 ${INSTALLDIR}/man/man1/wmpager.1x ; \
	echo "copying theme files to ${INSTALLDIR}/share/wmpager" ; \
	if [ ! -d ${INSTALLDIR}/share/wmpager ] ; then \
		mkdir -p ${INSTALLDIR}/share/wmpager ; \
	fi ; \
	cp src/buttons.xpm ${INSTALLDIR}/share/wmpager/blue.xpm ; \
	cp themes/*.xpm ${INSTALLDIR}/share/wmpager ; \
	chmod 644 ${INSTALLDIR}/share/wmpager/* ; \
	echo "installation complete, make sure your have ${INSTALLDIR}/bin" ; \
	echo "in your PATH and ${INSTALLDIR}/man in your MANPATH."

uninstall:
	echo "*** `pwd`: $@" ; \
	if [ -z "`id | grep 'uid=.*(root)'`" ] ; then \
		echo "you must be root to uninstall wmpager!" ; \
		exit ; \
	fi ; \
	for i in bin/wmpager man/man1/wmpager.1x ; do \
		if [ -f ${INSTALLDIR}/$$i ] ; then \
			echo "removing $$i from ${INSTALLDIR}" ; \
			rm -f ${INSTALLDIR}/$$i ; \
		fi ; \
	done ; \
	if [ -d ${INSTALLDIR}/share/wmpager ] ; then \
		echo "removing directory $$i from ${INSTALLDIR}" ; \
		rm -rf ${INSTALLDIR}/share/wmpager ; \
	fi ; \
	echo "uninstallation complete."

