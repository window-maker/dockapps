#ifndef RCFILE_H_INCLUDED
#define RCFILE_H_INCLUDED

  /************/
 /* Typedefs */
/************/

typedef struct _rckeys rckeys;

struct _rckeys {
	const char	*label;
	char		**var;
};

typedef struct _rckeys2 rckeys2;

struct _rckeys2 {
	const char	*family;
	const char	*label;
	char		**var;
};

  /***********************/
 /* Function Prototypes */
/***********************/

void parse_rcfile(const char *, rckeys *);

#endif
