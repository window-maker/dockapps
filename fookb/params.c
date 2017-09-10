#include <stdio.h>
#include <string.h>		/* strlen & strcat */
#include <ctype.h>		/* toupper */
#include <stdlib.h>
#include <WINGs/WUtil.h>
#include "params.h"
#include "opts.h"

/* Let's make lint happy */
#define lputs(x) (void)puts(x)

char *read_param(char *string)
{
	WMPropList *pl;
	WMPropList *value;
	WMPropList *tmp;
	char *path;
	char *result;

/* Command line parameters take precedence over all */

	if (!strcmp(string, "Icon1") && icon1)
		return icon1;
	if (!strcmp(string, "Icon2") && icon2)
		return icon2;
	if (!strcmp(string, "Icon3") && icon3)
		return icon3;
	if (!strcmp(string, "Icon4") && icon4)
		return icon4;
	if (!strcmp(string, "IconBoom") && iconboom)
		return iconboom;

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

}
