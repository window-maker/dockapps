#!/bin/sh
# autogen.sh -- Use this script to create generated files from the SVN distribution
# Taken from glib CVS

PROJECT=wmacpiload
TEST_TYPE=-f
TEST_FILE=src/main.c

ACLOCAL_FLAGS="${ACLOCAL_FLAGS}"

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

ORIGDIR=`pwd`
cd ${srcdir}

DIE=0

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "You must have autoconf installed to compile $PROJECT."
    echo "Download the appropriate package for your distribution,"
    echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
    DIE=1
}

(automake --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "You must have automake installed to compile $PROJECT."
    echo "Get ftp://sourceware.cygnus.com/pub/automake/automake-1.4.tar.gz"
    echo "(or a newer version if it is available)"
    DIE=1
}

(libtoolize --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "You must have libtool installed to compile $PROJECT."
    echo "Visit http://www.gnu.org/software/libtool/ for more information."
    DIE=1
}

if test "${DIE}" -eq 1; then
    exit 1
fi

test ${TEST_TYPE} ${TEST_FILE} || {
    echo "You must run this script in the top-level $PROJECT directory"
    exit 1
}

case ${CC} in
    *xlc | *xlc\ * | *lcc | *lcc\ *) am_opt=--include-deps;;
esac

aclocal ${ACLOCAL_FLAGS}

#libtoolize --force --copy

# Optionally feature autoheader
(autoheader --version)  < /dev/null > /dev/null 2>&1 && autoheader

automake --add-missing --copy ${am_opt}

autoconf

cd ${ORIGDIR}

rm -rf autom4te.cache
