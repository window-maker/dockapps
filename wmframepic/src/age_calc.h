#ifndef AGE_CALC_H_
#define AGE_CALC_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUMBER_OF_ROWS 3
#define NUMBER_OF_COLUMNS 100

char **get_phrases(int day, int month, int year);
void clear_phrases(char **phrases);

#endif /* AGE_CALC_H_ */
