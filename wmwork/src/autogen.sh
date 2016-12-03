#!/bin/sh -e
# $Id: autogen.sh,v 1.4 2005/12/02 07:36:59 godisch Exp $
#
# Copyright (c) 2002-2005 Martin A. Godisch <martin@godisch.de>

test -e wmwork.c
umask 022
aclocal
autoheader
autoconf
./configure
make distclean
rm -rf autom4te.cache
rm -f configure.scan autoscan.log
cd ..
chmod -R go=u-w .
version="`pwd`"
version="${version##*-}"
cd ..
tar cvzf wmwork_$version.orig.tar.gz --exclude CVS --exclude .cvsignore --exclude debian wmwork-$version
