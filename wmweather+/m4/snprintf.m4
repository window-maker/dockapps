# FUNC_SNPRINTF_EXISTS
# --------------------
# Checks if snprintf exists. x_cv_func_snprintf_exists is set.
AC_DEFUN([FUNC_SNPRINTF_EXISTS],
[AC_CHECK_FUNC(snprintf, [x_cv_func_snprintf_exists=yes], [x_cv_func_snprintf_exists=no])])# FUNC_SNPRINTF_EXISTS

# FUNC_SNPRINTF_SIZE
# ------------------
# Checks if snprintf honors its size argument. SNPRINTF_IS_SPRINTF is defined
# if not. x_cv_func_snprintf_size is set to yes or no.
#
# Note that this depends on FUNC_SNPRINTF_EXISTS, so if that fails this will
# also fail (and define SNPRINTF_IS_SPRINTF).
AC_DEFUN([FUNC_SNPRINTF_SIZE],
[AC_REQUIRE([FUNC_SNPRINTF_EXISTS])
if test $x_cv_func_snprintf_exists != yes; then x_cv_func_snprintf_size=no; else
AC_CACHE_CHECK([if snprintf honors the size argument], x_cv_func_snprintf_size,
[AC_RUN_IFELSE(
[AC_LANG_PROGRAM(
[[#if STDC_HEADERS || HAVE_STDIO_H
# include <stdio.h>
#else
int snprintf(char *str, size_t size, const char *format, ...);
#endif
]],
[[char foo[]="ABC"; snprintf(foo, 2, "%d", 12);
exit((foo[0]=='1' && foo[1]=='\0' && foo[2]=='C')?0:1);]])],
[x_cv_func_snprintf_size=yes],
[x_cv_func_snprintf_size=no],
[x_cv_func_snprintf_size=no])])
fi
test $x_cv_func_snprintf_size != yes && AC_DEFINE(SNPRINTF_IS_SPRINTF, 1, [Define if snprintf ignores the size argument])
])# FUNC_SNPRINTF_SIZE

# FUNC_SNPRINTF_RETVAL
# ------------------
# Checks if snprintf returns the number of bytes that would have been written,
# as specified by C99. SNPRINTF_BOGUS_RETVAL is defined if not.
# x_cv_func_snprintf_retval is set to yes or no.
#
# Note that this depends on FUNC_SNPRINTF_SIZE, so if that fails this will fail
# too and SNPRINTF_BOGUS_RETVAL will be set.
AC_DEFUN([FUNC_SNPRINTF_RETVAL],
[AC_REQUIRE([FUNC_SNPRINTF_SIZE])
if test $x_cv_func_snprintf_size != yes; then x_cv_func_snprintf_retval=no; else
AC_CACHE_CHECK([if snprintf return value is sane], x_cv_func_snprintf_retval,
[AC_RUN_IFELSE(
[AC_LANG_PROGRAM(
[[#if STDC_HEADERS || HAVE_STDIO_H
# include <stdio.h>
#else
int snprintf(char *str, size_t size, const char *format, ...);
#endif
]],
[[char foo[10]; exit((snprintf(foo, 1, "%d", 9876)==4)?0:1);]])],
[x_cv_func_snprintf_retval=yes],
[x_cv_func_snprintf_retval=no],
[x_cv_func_snprintf_retval=no])])
fi
test $x_cv_func_snprintf_retval != yes && AC_DEFINE(SNPRINTF_BOGUS_RETVAL, 1, [Define if snprintf's return value isn't as specified by C99])
])# FUNC_SNPRINTF_RETVAL

# FUNC_SNPRINTF_NULL_OK
# ---------------------
# Checks whether snprintf acceps a NULL string if size is zero. Sets
# x_cv_func_snprintf_null_ok. If so, define SNPRINTF_NULL_OK.
#
# Note that this depends on FUNC_SNPRINTF_SIZE, so if that fails this will fail
# too and SNPRINTF_BOGUS_RETVAL will be set.
AC_DEFUN([FUNC_SNPRINTF_NULL_OK],
[AC_REQUIRE([FUNC_SNPRINTF_SIZE])
if test $x_cv_func_snprintf_size != yes; then x_cv_func_snprintf_null_ok=no; else
AC_CACHE_CHECK([if snprintf(NULL, 0, ...) works], x_cv_func_snprintf_null_ok,
[AC_RUN_IFELSE(
[AC_LANG_PROGRAM(
[[#if STDC_HEADERS || HAVE_STDIO_H
# include <stdio.h>
#else
int snprintf(char *str, size_t size, const char *format, ...);
#endif
]],
[int r=snprintf(NULL, 0, "%d", 100); exit((r==3 || r==-1)?0:1);])],
[x_cv_func_snprintf_null_ok=yes],
[x_cv_func_snprintf_null_ok=no],
[x_cv_func_snprintf_null_ok=no])])
fi
test $x_cv_func_snprintf_null_ok = yes && AC_DEFINE(SNPRINTF_NULL_OK, 1, [Define if snprintf(NULL, 0, ...) works properly])
])# FUNC_SNPRINTF_NULL_OK

# FUNC_SNPRINTF([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# -------------
# Checks various aspects of snprintf. In particular:
#  * Does it exist?
#  * Is the size honored?
#  * Is the return value correct?
#  * Is NULL with length 0 ok?
# If all the above pass, HAVE_WORKING_SNPRINTF is defined and
# x_cv_func_snprintf_working is set to yes. Otherwise, it's set to no.
AC_DEFUN([FUNC_SNPRINTF],
[AC_REQUIRE([FUNC_SNPRINTF_RETVAL])
AC_REQUIRE([FUNC_SNPRINTF_NULL_OK])
if test $x_cv_func_snprintf_retval = yes -a $x_cv_func_snprintf_null_ok = yes; then
    AC_DEFINE(HAVE_WORKING_SNPRINTF, 1, [Define if snprintf works properly])
    x_cv_func_snprintf_working=yes
    $1
else
    x_cv_func_snprintf_working=no
    $2
fi
])# FUNC_SNPRINTF

# FUNC_SNPRINTF_LIBOBJ
# --------------------
# If FUNC_SNPRINTF fails, does AC_LIBOBJ
AC_DEFUN([FUNC_SNPRINTF_LIBOBJ],
[FUNC_SNPRINTF(, [AC_LIBOBJ([snprintf])
AC_DEFINE([snprintf], [rpl_snprintf], [Define to rpl_snprintf if the replacement function should be used.])])])
])#FUNC_SNPRINTF_LIBOBJ


