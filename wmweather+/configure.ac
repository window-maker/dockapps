# Process this file with autoconf to produce a configure script.
AC_INIT(wmweather+.c)
AM_CONFIG_HEADER(config.h)
AM_INIT_AUTOMAKE(wmweather+, 2.12)

# We need GNU sources for certain features
AH_VERBATIM([_GNU_SOURCE],
[/* Enable GNU extensions on systems that have them */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif])

DATE=`date '+%B %e, %Y'`
AC_SUBST(DATE)
AC_SUBST(VERSION)

# for wmgeneral
WMGENERAL_GUI="libwmgeneral-x11.a"
AC_SUBST(WMGENERAL_GUI)

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB

# Checks for libraries.
AC_CHECK_LIB([m],[cos])
CHECK_LIBPCRE(, AC_MSG_ERROR(libpcre is required))
LIBCURL_CHECK_CONFIG(yes,,,AC_MSG_ERROR(libcurl is required))

# Checks for header files.
AC_PATH_XTRA
AC_FIND_XPM
if test "$LINK_XPM" = ""; then
    AC_MSG_ERROR(cannot find libxpm)
fi
CHECK_LIBWRASTER(, AC_MSG_ERROR(libwraster is required))
AC_HEADER_STDC
if test "$ac_cv_header_stdc" != "yes"; then AC_MSG_WARN(standard C headers not found); fi
AC_HEADER_SYS_WAIT
if test "$ac_cv_header_sys_wait_h" != "yes"; then AC_MSG_WARN(sys/wait.h not found); fi
AC_CHECK_HEADERS([fcntl.h limits.h stdlib.h string.h unistd.h sys/time.h],, AC_MSG_WARN($ac_header not found))
AC_HEADER_TIME

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_CHECK_TYPES([ssize_t])
AH_BOTTOM(
[#if !HAVE_SSIZE_T
typedef signed ssize_t;
#endif])
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_FORK
if test "$ac_cv_func_fork_works" != "yes"; then AC_MSG_WARN(fork() doesn't seem to work); fi
AC_FUNC_MALLOC
if test "$ac_cv_func_malloc_0_nonnull" != "yes"; then AC_MSG_WARN(malloc() doesn't seem to work); fi
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
if test "$ac_cv_func_vprintf" != "yes"; then AC_MSG_WARN(vprintf() doesn't seem to exist); fi
AC_CHECK_FUNCS([atexit dup2 mkdir pow select strchr strcspn strdup strerror strrchr strspn strstr strtol],, AC_MSG_WARN($ac_func doesn't seem to be available))
FUNC_SNPRINTF_LIBOBJ
FUNC_VSNPRINTF_LIBOBJ

CFLAGS="$CFLAGS \$(X_CFLAGS) \$(LIBCURL_CPPFLAGS)"
LIBS="$LIBS \$(LIBCURL)"
XLIBS="\$(X_PRE_LIBS) \$(X_LIBS) \$(LINK_XPM) -lX11 -lXext \$(X_EXTRA_LIBS)"
AC_SUBST(XLIBS)

AC_CONFIG_FILES([Makefile wmweather+.1])
AC_CONFIG_FILES([b0rken/Makefile])
AC_CONFIG_FILES([m4/Makefile])
AC_CONFIG_FILES([wmgeneral/Makefile])
AC_OUTPUT
