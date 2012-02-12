# FUNC_VSNPRINTF_EXISTS
# --------------------
# Checks if vsnprintf exists. x_cv_func_vsnprintf_exists is set.
AC_DEFUN([FUNC_VSNPRINTF_EXISTS],
[AC_REQUIRE([AC_FUNC_VPRINTF])
if test $ac_cv_func_vprintf != yes; then x_cv_func_vsnprintf_exists=no; else
AC_CHECK_FUNC(vsnprintf, [x_cv_func_vsnprintf_exists=yes], [x_cv_func_vsnprintf_exists=no])
fi
])# FUNC_VSNPRINTF_EXISTS

# FUNC_VSNPRINTF_SIZE
# ------------------
# Checks if vsnprintf honors its size argument. VSNPRINTF_IS_VSPRINTF is defined
# if not. x_cv_func_vsnprintf_size is set to yes or no.
#
# Note that this depends on FUNC_VSNPRINTF_EXISTS, so if that fails this will
# also fail (and define VSNPRINTF_IS_VSPRINTF).
AC_DEFUN([FUNC_VSNPRINTF_SIZE],
[AC_REQUIRE([FUNC_VSNPRINTF_EXISTS])
if test $x_cv_func_vsnprintf_exists != yes; then x_cv_func_vsnprintf_size=no; else
AC_CACHE_CHECK([if vsnprintf honors the size argument], x_cv_func_vsnprintf_size,
[AC_RUN_IFELSE(
[AC_LANG_PROGRAM(
[[#include <stdarg.h>
#if STDC_HEADERS || HAVE_STDIO_H
# include <stdio.h>
#else
int vsnprintf(char *str, size_t size, const char *format, va_list ap);
#endif
int doit(char *str, size_t size, const char *format, ...){
    va_list ap;
    int r;
    va_start(ap, format);
    r=vsnprintf(str, size, format, ap);
    va_end(ap);
    return r;
}
]],
[[char foo[]="ABC"; doit(foo, 2, "%d", 12);
exit((foo[0]=='1' && foo[1]=='\0' && foo[2]=='C')?0:1);]])],
[x_cv_func_vsnprintf_size=yes],
[x_cv_func_vsnprintf_size=no],
[x_cv_func_vsnprintf_size=no])])
fi
test $x_cv_func_vsnprintf_size != yes && AC_DEFINE(VSNPRINTF_IS_VSPRINTF, 1, [Define if vsnprintf ignores the size argument])
])# FUNC_VSNPRINTF_SIZE

# FUNC_VSNPRINTF_RETVAL
# ------------------
# Checks if vsnprintf returns the number of bytes that would have been written,
# as specified by C99. VSNPRINTF_BOGUS_RETVAL is defined if not.
# x_cv_func_vsnprintf_retval is set to yes or no.
#
# Note that this depends on FUNC_VSNPRINTF_SIZE, so if that fails this will fail
# too and VSNPRINTF_BOGUS_RETVAL will be set.
AC_DEFUN([FUNC_VSNPRINTF_RETVAL],
[AC_REQUIRE([FUNC_VSNPRINTF_SIZE])
if test $x_cv_func_vsnprintf_size != yes; then x_cv_func_vsnprintf_retval=no; else
AC_CACHE_CHECK([if vsnprintf return value is sane], x_cv_func_vsnprintf_retval,
[AC_RUN_IFELSE(
[AC_LANG_PROGRAM(
[[#include <stdarg.h>
#if STDC_HEADERS || HAVE_STDIO_H
# include <stdio.h>
#else
int vsnprintf(char *str, size_t size, const char *format, va_list ap);
#endif
int doit(char *str, size_t size, const char *format, ...){
    va_list ap;
    int r;
    va_start(ap, format);
    r=vsnprintf(str, size, format, ap);
    va_end(ap);
    return r;
}
]],
[[char foo[10]; exit((doit(foo, 1, "%d", 9876)==4)?0:1);]])],
[x_cv_func_vsnprintf_retval=yes],
[x_cv_func_vsnprintf_retval=no],
[x_cv_func_vsnprintf_retval=no])])
fi
test $x_cv_func_vsnprintf_retval != yes && AC_DEFINE(VSNPRINTF_BOGUS_RETVAL, 1, [Define if vsnprintf's return value isn't as specified by C99])
])# FUNC_VSNPRINTF_RETVAL

# FUNC_VSNPRINTF_NULL_OK
# ---------------------
# Checks whether vsnprintf acceps a NULL string if size is zero. Sets
# x_cv_func_vsnprintf_null_ok. If so, define VSNPRINTF_NULL_OK.
#
# Note that this depends on FUNC_VSNPRINTF_SIZE, so if that fails this will
# fail too and VSNPRINTF_NULL_OK will not be set.
AC_DEFUN([FUNC_VSNPRINTF_NULL_OK],
[AC_REQUIRE([FUNC_VSNPRINTF_SIZE])
if test $x_cv_func_vsnprintf_size != yes; then x_cv_func_vsnprintf_null_ok=no; else
AC_CACHE_CHECK([if vsnprintf(NULL, 0, ...) works], x_cv_func_vsnprintf_null_ok,
[AC_RUN_IFELSE(
[AC_LANG_PROGRAM(
[[#include <stdarg.h>
#if STDC_HEADERS || HAVE_STDIO_H
# include <stdio.h>
#else
int vsnprintf(char *str, size_t size, const char *format, va_list ap);
#endif
int doit(char *str, size_t size, const char *format, ...){
    va_list ap;
    int r;
    va_start(ap, format);
    r=vsnprintf(str, size, format, ap);
    va_end(ap);
    return r;
}
]],
[int r=doit(NULL, 0, "%d", 100); exit((r==3 || r==-1)?0:1);])],
[x_cv_func_vsnprintf_null_ok=yes],
[x_cv_func_vsnprintf_null_ok=no],
[x_cv_func_vsnprintf_null_ok=no])])
fi
test $x_cv_func_vsnprintf_null_ok = yes && AC_DEFINE(VSNPRINTF_NULL_OK, 1, [Define if vsnprintf(NULL, 0, ...) works properly])
])# FUNC_VSNPRINTF_NULL_OK

# FUNC_VSNPRINTF([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# -------------
# Checks various aspects of vsnprintf. In particular:
#  * Does it exist?
#  * Is the size honored?
#  * Is the return value correct?
#  * Is NULL with length 0 ok?
# If all the above pass, HAVE_WORKING_VSNPRINTF is defined and
# x_cv_func_vsnprintf_working is set to yes. Otherwise, it's set to no.
AC_DEFUN([FUNC_VSNPRINTF],
[AC_REQUIRE([FUNC_VSNPRINTF_RETVAL])
AC_REQUIRE([FUNC_VSNPRINTF_NULL_OK])
if test $x_cv_func_vsnprintf_retval = yes -a $x_cv_func_vsnprintf_null_ok = yes; then
    AC_DEFINE(HAVE_WORKING_VSNPRINTF, 1, [Define if vsnprintf works properly])
    x_cv_func_snprintf_working=yes
    $1
else
    x_cv_func_snprintf_working=no
    $2
fi
])# FUNC_VSNPRINTF

# FUNC_VSNPRINTF_LIBOBJ
# --------------------
# If FUNC_VSNPRINTF fails, does AC_LIBOBJ.
AC_DEFUN([FUNC_VSNPRINTF_LIBOBJ],
[FUNC_VSNPRINTF(, [AC_LIBOBJ([vsnprintf])
AC_DEFINE([vsnprintf], [rpl_vsnprintf], [Define to rpl_vsnprintf if the replacement function should be used.])])])
])#FUNC_VSNPRINTF_LIBOBJ
