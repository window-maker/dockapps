#include <stdarg.h>

void warn(char *fmt, ...) __attribute__ ((__format__ (__printf__, 1, 2)));
void die(char *fmt, ...) __attribute__ ((__format__ (__printf__, 1, 2), __noreturn__));

void vwarn(char *fmt, va_list ap) __attribute__ ((__format__ (__printf__, 1, 0)));
void vdie(char *fmt, va_list ap) __attribute__ ((__format__ (__printf__, 1, 0), __noreturn__));
