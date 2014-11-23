#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>



/* tabulators */
char		tabs[1024];


#ifdef	DBG_THREADS
#include <pthread.h>

static pthread_key_t	align_key;
static pthread_once_t	align_key_once = PTHREAD_ONCE_INIT;


static void
align_destroy(void *align)
{
	free(align);
}


static void
align_key_create()
{
	pthread_key_create(&align_key, align_destroy);
}


void
debug_init_threads()
{
	int	*new_align;

	memset(tabs, 9, 1024);
	
	if (!(new_align = malloc(sizeof(int))))
		abort();
	*new_align = 1;

	pthread_once(&align_key_once, align_key_create);

	pthread_setspecific(align_key, new_align);
}
#endif


/*
 * no threads
 */

static int		main_align = 1;

void
debug_init_nothreads()
{
	memset(tabs, 9, 1024);
}




void
fn_begin(char *format, ...)
{
	va_list		args;
	char		msg_buf[1024];
	char		final_msg[2048];
	int		*align;

#ifdef DBG_THREADS
		align = pthread_getspecific(align_key);
#else
		align = &main_align;
#endif
	
	va_start(args, format);
	vsprintf(msg_buf, format, args);
	va_end(args);

	strcat(msg_buf, " {\n");

	sprintf(final_msg, "%d: %.*s%s", getpid(), *align, tabs, msg_buf);
	fprintf(stderr, "%s", final_msg);
	(*align)++;
}


void
fn_end()
{
	int		*align;

#ifdef DBG_THREADS
		align = pthread_getspecific(align_key);
#else
		align = &main_align;
#endif

	(*align)--;

	fprintf(stderr, "%d: %.*s}\n", getpid(), *align, tabs);
}




/*
void
fn2(int i)
{
	_D(fn_begin("fn2(i = %d)", i));
	return_void();
}

int
fn1()
{
	_D(fn_begin("fn1"));
	fn2(1);
	return_val(0);
}
	
int
main()
{
	_D(fn_begin("main"));
	fn1();
	fn2(2);
	return_val(0);
}
*/
