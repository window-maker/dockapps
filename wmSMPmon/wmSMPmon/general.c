/*######################################################################
  #                                                                    #
  # This file contains some general utility functions for wmSMPmon.    #
  #                                                                    #
  # All of them were taken from the program 'top' from the procps      #
  # suite.                                                             #
  # With thanks to the author of top:                                  #
  #                      James C. Warner <warnerjc@worldnet.att.net>   #
  #                                                                    #
  # This file is placed under the conditions of the GNU Library        #
  # General Public License, version 2, or any later version.           #
  # See file COPYING for information on distribution conditions.       #
  #                                                                    #
  ######################################################################*/

#include "standards.h"
#include "general.h"

/*
 * The usual program end --
 * called only by functions in this section. */
void bye_bye(int eno, const char *str)
{
	fflush(stdout);
	if (str) {
		if (eno)
			perror(str);
		else {
			fputs(str, stderr);
			eno = 1;
		}
	}
	exit(eno);
}

/*
 * This routine simply formats whatever the caller wants and
 * returns a pointer to the resulting 'const char' string... */
const char *fmtmk(const char *fmts, ...)
{
	static char buf[BIGBUFSIZ];      /* with help stuff, our buffer */
	va_list va;                      /* requirements exceed 1k */

	va_start(va, fmts);
	vsnprintf(buf, sizeof(buf), fmts, va);
	va_end(va);
	return ((const char *)buf);
}

/*
 * Standard error handler to normalize the look of all err o/p */
void std_err(const char *str)
{
	static char buf[SMLBUFSIZ];

	fflush(stdout);
	/* we'll use our own buffer so callers can still use fmtmk()
	 * and, yes the leading tab is not the standard convention,
	 * but the standard is wrong -- OUR msg won't get lost in
	 * screen clutter, like so many others! */
	snprintf(buf, sizeof(buf), "\t%s: %s\n", Myname, str);

	/* not to worry, he'll change our exit code to 1 due to 'buf' */
	bye_bye(0, buf);
}

/*
 * Handle our own memory stuff without the risk of leaving the
 * user's terminal in an ugly state should things go sour. */
void *alloc_c(unsigned numb)
{
	void * p;

	if (!numb)
		++numb;
	if (!(p = calloc(1, numb)))
		std_err("failed memory allocate");
	return (p);
}
