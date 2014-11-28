/*
 * Copyright (c) 1999-2005 Alfredo K. Kojima, Alban G. Hertroys
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * $Id: daargs.c,v 1.9 2008/05/01 09:49:06 dalroi Exp $
 */

#include <assert.h>
#include <string.h>

#include "daargs.h"
#include "dautil.h"

#define DEFAULT_OPTION_COUNT 3

extern struct DAContext *_daContext;

/*
 * Prototypes
 */

static void _daContextAddDefaultOptions();
static void _daContextAddOptions(DAProgramOption *options, int count);
static void printHelp(char *description);

int contains(char *needle, char *haystack);
int parseOption(DAProgramOption *option, int i, int argc, char** argv);
int readIntOption(int index, char **argv);

/*
 * Public functions
 */

void
DAParseArguments(
	int argc,
	char **argv,
	DAProgramOption *options,
	int count,
	char *programDescription,
	char *versionDescription)
{
    int i, j, size;
    int found = 0;

    _daContext = DAContextInit();

    _daContext->argc		= argc;
    _daContext->argv		= argv;
    _daContext->programName	= argv[0];

    size = (count + DEFAULT_OPTION_COUNT) * sizeof(DAProgramOption*);
    _daContext->options		= malloc(size);
    memset(_daContext->options, 0, size);

    _daContextAddDefaultOptions();
    _daContextAddOptions(options, count);

    for (i = 1; i < argc; i++) {
	char *optStr = argv[i];

	/* Handle default options */
	if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
	    printHelp(programDescription), exit(0);

	if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
	    puts(versionDescription), exit(0);

	if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--windowed") == 0) {
	    _daContext->windowed = 1;
	    continue;
	}

	found = 0;
	/* options with a one-to-one mapping */
	for (j = 0; j < count; j++) {
	    DAProgramOption *option = &options[j];

	    if ((option->longForm && strcmp(option->longForm, optStr) == 0)
	    || (option->shortForm && strcmp(option->shortForm, optStr) == 0)) {

		found = 1;
		i = parseOption(option, i, argc, argv);
	    }
	}

	/* mixed options */
	if (!found) {
	    /* XXX: Parsing all options again... */
	    for (j = 0; j < count; j++) {
		DAProgramOption *option = &options[j];

		if (option->shortForm && contains(option->shortForm, optStr)) {
		    found = 1;
		    i = parseOption(option, i, argc, argv);
		}
	    }
	}

	if (!found) {
	    printf("%s: unrecognized option '%s'\n", argv[0], argv[i]);
	    printHelp(programDescription), exit(1);
	}
    }
}

int
contains(char *needle, char *haystack)
{
    char c, *pos;

    assert(strlen(needle) == 2);

    c = needle[1];

    pos = strchr(haystack, c);

    return (pos != NULL);
}

int
parseOption(DAProgramOption *option, int i, int argc, char** argv)
{
    option->used = True;

    if (option->type == DONone)
	return i;

    i++;
    if (i >= argc)
	printf("%s: missing argument for option '%s'\n",
		argv[0],
		argv[i-1]),
	    exit(1);

    switch (option->type) {
	case DOInteger:
	    *option->value.integer = readIntOption(i, argv);

	    break;

	case DONatural:
	    *option->value.integer = readIntOption(i, argv);

	    if (*option->value.integer < 0)
		printf("%s: argument %s must be >= 0\n",
			argv[0],
			argv[i-1]),
		    exit(1);
	    break;

	case DOString:
	    *option->value.string = argv[i];
	    break;
    }

    return i;
}

int
readIntOption(int index, char **argv)
{
    int integer;

    if (sscanf(argv[index], "%i", &integer) != 1)
	DAError("error parsing argument for option %s\n", argv[index-1]),
	    exit(1);

    return integer;
}

int
DAGetArgC()
{
    return _daContext->argc;
}

char**
DAGetArgV()
{
    return _daContext->argv;
}

char*
DAGetProgramName()
{
    return _daContext->programName;
}


/*
 * Local functions
 */

struct DAContext*
DAContextInit()
{
    struct DAContext *context = malloc(sizeof(struct DAContext));

    memset(context, 0, sizeof(struct DAContext));

    return context;
}

void
DAFreeContext()
{
    if (_daContext->optionCount > 0) {
	int i;

	for (i = 0; i < _daContext->optionCount; i++) {
	    free(_daContext->options[i]);
	}

	free(_daContext->options);
    }

    free(_daContext);
}

static void
_daContextAddOption(DAProgramOption *option)
{
    /* If the buffer is full, double its size */
    if (sizeof(_daContext->options) == _daContext->optionCount * sizeof(DAProgramOption))
    {
	DAProgramOption **options;

	options = (DAProgramOption**)realloc(
		(DAProgramOption**)_daContext->options,
		2 * sizeof(_daContext->options));

	if (options == NULL)
	    DAError("Out of memory");

	_daContext->options = options;
    }

    _daContext->options[_daContext->optionCount] = option;
    _daContext->optionCount++;
}

static void
_daContextAddOptionData(char *shortForm, char *longForm,
	char *description, short type)
{
    DAProgramOption *option = malloc(sizeof(DAProgramOption));

    option->shortForm	= shortForm;
    option->longForm	= longForm;
    option->description	= description;
    option->type	= type;
    option->used	= False;
    option->value.ptr	= NULL;

    _daContextAddOption(option);
}

static void
_daContextAddDefaultOptions()
{
    _daContextAddOptionData("-h", "--help", "show this help text and exit", DONone);
    _daContextAddOptionData("-v", "--version",  "show program version and exit", DONone);
    _daContextAddOptionData("-w", "--windowed", "run the application in windowed mode", DONone);
}

static void
_daContextAddOptions(DAProgramOption *options, int count)
{
    int i;

    for (i = 0; i < count; i++) {
	_daContextAddOptionData(
		options[i].shortForm,
		options[i].longForm,
		options[i].description,
		options[i].type);
    }
}

static void
printHelp(char *description)
{
    int i;
    DAProgramOption **options	= _daContext->options;
    int count			= _daContext->optionCount;

    printf("Usage: %s [OPTIONS]\n", _daContext->programName);
    if (description)
	puts(description);

    for (i = 0; i < count; i++)
    {
	char blank[30];
	int c;

	if (options[i]->shortForm && options[i]->longForm)
	    c = printf("  %s, %s", options[i]->shortForm, options[i]->longForm);
	else if (options[i]->shortForm)
	    c = printf("  %s", options[i]->shortForm);
	else if (options[i]->longForm)
	    c = printf("  %s", options[i]->longForm);
	else
	    continue;

	if (options[i]->type != DONone) {
	    switch (options[i]->type) {
		case DOInteger:
		    c += printf(" <integer>");
		    break;
		case DOString:
		    c += printf(" <string>");
		    break;
		case DONatural:
		    c += printf(" <number>");
		    break;
	    }
	}

	memset(blank, ' ', 30);
	if (c > 29)
	    c = 1;
	blank[30-c] = 0;
	printf("%s %s\n", blank, options[i]->description);
    }
}



