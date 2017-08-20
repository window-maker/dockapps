#include <stdlib.h>
#include <string.h>
#include "arg_parser.h"



int get_values_from_command_line(int argc, char **argv,
		char *my_name,
		char **my_file,
		int *my_day,
		int *my_month,
		int *my_year) {

	struct arg_str  *name		= arg_str1("n", "name", "<string>", "the name of the person");
	struct arg_file  *xpm_file	= arg_file1("f", "xpm_file", "<file>", "the xpm file path");
	struct arg_int 	*day		= arg_int1("d", "day", "int", "the day of birth");
	struct arg_int 	*month		= arg_int1("m", "month", "int", "the month of birth");
	struct arg_int 	*year		= arg_int1("y", "year", "int", "the year of birth");
    struct arg_lit  *help    	= arg_lit0(NULL, "help", "print this help and exit");
    struct arg_end  *end     	= arg_end(20);
    void* argtable[] = {name, xpm_file, day, month, year, help, end};
    const char* progname = "wmframepic";
    int nerrors;
    int exitcode = COMMAND_LINE_SUCCESS;
    int i;

    /* verify the argtable[] entries were allocated sucessfully */
    if (arg_nullcheck(argtable) != 0) {
		/* NULL entries were detected, some allocations must have failed */
		printf("%s: insufficient memory\n", progname);
		exitcode = COMMAND_LINE_FAIL;
		goto exit;
	}

    /* Parse the command line as defined by argtable[] */
    nerrors = arg_parse(argc,argv,argtable);

    /* special case: '--help' takes precedence over error reporting */
    if (help->count > 0) {
		printf(
"This is a dockapp written for Window Maker and any other window manager\n\
that suports the dock (or slit), like the Window Maker itself, Fluxbox, \n\
Blackbox and others.\n\n");
		printf(
"It's meant to show the picture of your kid (or any beloved one) on your\n\
screen and when pressed, it will show how old he/she is. Pretty silly but\n\
I'm sure you will enjoy!\n\n");

		printf("Usage: %s", progname);
		arg_print_syntax(stdout, argtable, "\n");

		arg_print_glossary(stdout, argtable, "  %-25s %s\n");
		exitcode = COMMAND_LINE_HELP;
		goto exit;
	}

    /* If the parser returned any errors then display them and exit */
    if (nerrors > 0) {
		/* Display the error details contained in the arg_end struct.*/
		arg_print_errors(stdout, end, progname);
		printf("Try '%s --help' for more information.\n", progname);
		exitcode = COMMAND_LINE_FAIL;
		goto exit;
	}

    /* only get here is command line arguments were parsed sucessfully */

    strncpy(my_name, *name->sval, 9);
    *my_file = malloc(strlen(xpm_file->filename[0]) * sizeof(char) + 1);
    strcpy(*my_file, xpm_file->filename[0]);
    my_name[9] = '\0';
    *my_day = *day->ival;
    *my_month = *month->ival;
    *my_year = *year->ival;

    exit:
    /* deallocate each non-null entry in argtable[] */
    arg_freetable(argtable,sizeof(argtable)/sizeof(argtable[0]));

    return exitcode;
}
