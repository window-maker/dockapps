#include <stdio.h>
#include <string.h>		/* strlen & strcat */
#include <ctype.h>		/* toupper */
#include <stdlib.h>
#include "params.h"
#include "opts.h"

/* Let's make lint happy */
#define lputs(x) (void)puts(x)

char *read_param(char *string)
{
	XrmValue xvalue;

#ifdef HAVE_WINGS_WUTIL_H
	WMPropList *pl;
	WMPropList *value;
	WMPropList *tmp;
	char *path;
#endif

	char *newstring;
	char *newString;
	char *result;
	char *str_type[20];
	
	/* Let's make lint happy */
	xvalue.size = 0;

#ifndef HAVE_WINGS_WUTIL_H
	newstring = (char *) malloc(7 + strlen(string));
	newString = (char *) malloc(7 + strlen(string));
	/* 7 is strlen("fookb.") + 1 */
	
	if ((NULL == newstring) || (NULL == newString)) {
		lputs("Not enough memory");
		exit(EXIT_FAILURE);
	}

	strcpy(newstring, "fookb.");
	strcpy(newString, "Fookb.");

	strcat(newstring, string);
	strcat(newString, string);
#else
	/* libWUtil contains handy function */
	newstring = wstrconcat("fookb.", string);
	newString = wstrconcat("Fookb.", string);
#endif
	newstring[6] = tolower((unsigned char)newstring[6]);
	newString[6] = toupper((unsigned char)newString[6]);

/* Command line parameters take precedence over all */

	if (XrmGetResource(cmdlineDB,
				newstring,
				newString,
				str_type,
				&xvalue) == True) {
		result = (char *) malloc(xvalue.size + 1);
		if (NULL == result) {
			lputs("Not enough memory");
			exit(EXIT_FAILURE);
		}
		strncpy(result, xvalue.addr, (size_t)xvalue.size);
		result[(int) xvalue.size + 1] = '\0';

#ifdef HAVE_WINGS_WUTIL_H
		wfree(newstring);
		wfree(newString);
#else
		free(newstring);
		free(newString);
#endif
		return result;
	}

#ifdef HAVE_WINGS_WUTIL_H

	wfree(newstring);
	wfree(newString);

	/*
	 * Here we start the game with property lists.
	 * pl will be property list, read from DEFAULTS_FILE.
	 * tmp will be temporary key for this property list,
	 * constructed using ``string'', supplied to function.
	 * value will be property list, which contain the value
	 * of parameter
	 */

	path = wexpandpath(DEFAULTS_FILE);
	pl = WMReadPropListFromFile(path);
	wfree(path);

	if (!pl) {
		lputs("Cannot open config file: ");
		lputs(DEFAULTS_FILE);
		exit(EXIT_FAILURE);
	}

	tmp = WMCreatePLString(string);
	value = WMGetFromPLDictionary(pl, tmp);
	WMReleasePropList(tmp);

	/*
	 * pl and value objects will exist as long as fookb is running
	 */

	if (!value) {
		lputs("Cannot find in config file value for: ");
		lputs(string);
		exit(EXIT_FAILURE);
	}

	if (!WMIsPLString(value)) {
		lputs("Value for: ");
		lputs(string);
		lputs("in config file is not a string.");
		exit(EXIT_FAILURE);
	}

	result = WMGetFromPLString(value);

	if (!result) {
		lputs("Something wrong with libWUtils :(");
		lputs("Please report this error to fookb author.");
		exit(EXIT_FAILURE);
	}

	return result;

#else				/* HAVE_WINGS_WUTIL_H */

	if (XrmGetResource(finalDB, newstring, newString, str_type,
			   &xvalue) == True) {
		result = (char *) malloc(xvalue.size + 1);
		if (NULL == result) {
			lputs("Not enough memory");
			exit(EXIT_FAILURE);
		}
		strncpy(result, xvalue.addr, (size_t)xvalue.size);
		result[(int) xvalue.size + 1] = '\0';
		free(newstring);
		free(newString);
		return result;
	} else {
		(void)printf("Fatal error: cannot find configuration parameter %s\n",
				newstring);
		exit(EXIT_FAILURE);
	}

#endif

}
