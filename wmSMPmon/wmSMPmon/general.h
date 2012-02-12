#ifndef WMSMP_GENERAL_H
#define WMSMP_GENERAL_H

/*###### Definitions for general utility functions #######################*/

void bye_bye (int eno, const char *str);

const char *fmtmk (const char *fmts, ...);

void std_err (const char *str);

void *alloc_c (unsigned numb);

#endif /* WMSMP_GENERAL_H */
