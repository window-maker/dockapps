#ifndef ARG_PARSER_H_
#define ARG_PARSER_H_

#include <argtable2.h>

enum {
	COMMAND_LINE_SUCCESS,
	COMMAND_LINE_FAIL,
	COMMAND_LINE_HELP
};

int get_values_from_command_line(int argc, char **argv,
		char *my_name,
		char **my_file,
		int *my_day,
		int *my_month,
		int *my_year);

#endif /* ARG_PARSER_H_ */
