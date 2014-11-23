#ifndef _DEBUG_H_
#define _DEBUG_H_


#ifdef	DBG_THREADS
#define debug_init()	debug_init_threads()
#else
#define	debug_init()	debug_init_nothreads()
#endif


void
debug_init_threads();

void
debug_init_nothreads();


void
fn_begin(char *format, ...);

void
fn_end();


#ifdef FNCALL_DEBUG
/* define macros with debugging on */

#define begin_func(_format, _args...)	fn_begin(_format , ##_args)

#define	return_val(_value) \
	do { \
		fn_end(); \
		return _value; \
	} while (0)

#define	return_void() \
	do { \
		fn_end(); \
		return; \
	} while (0)

#else
/* define macros with debugging off */

#define begin_func(_format)

#define	return_val(_value)		return _value

#define	return_void()			return

#endif


#endif
