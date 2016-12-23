/* readln.c - Edward H. Flora - ehflora@ksu.edu */
/* Last Modified: 2/24/98 */
/*
 *	This function reads a string from a file pointed to by fp until a
 *   new line (EOLN) or an end of file is encountered.  It returns a pointer
 *   to the string.  The new line character is included in the string returned
 *   by the function.
 */

/* EXTERNALS:
 ******************
 * Functions Called:
 *      Standard Libraries.
 *
 * Header Files Included:
 *      Standard Includes only.
 */

/* PORTABILITY:
 ******************
 * Coded in ANSI C, using ANSI prototypes.
 */

/******  Include Files  *************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EOLN '\n'		/* Defines the new line character */
#define SIZE1 10		/* Defines the increment to increase the */
				/*  string by until a newline of EOF is found */

/******  Function to Read a Line from fp  *******************************/
char *readln(FILE *fp){
  char *tmp, *t1, *t2;
  int count = 0, s1 = SIZE1;

  if((tmp = malloc(SIZE1*sizeof(char))) == NULL)return NULL;
  				/* If cannot allocate memory */
  if((t1 = fgets(tmp, SIZE1, fp)) == NULL){
    free(tmp);
    return NULL;
  }				/* If there is no string to be read */
  while(strchr(t1,EOLN) == NULL){ /* Loop while EOLN is not in the string */
    count += strlen(t1);
    if((s1 - count) > 1)break;
    s1 +=SIZE1;
    if((t2 = realloc(tmp, s1*sizeof(char))) == NULL){
      free(tmp);
      return NULL;
    }				/* If cannot allocate more memory */
    else tmp = t2;
    if((t1 = fgets((tmp+count),((s1 - count)),fp)) == NULL)break;
  }				/* End of While Loop */
  if((t2 = realloc(tmp,strlen(tmp)+1)) == NULL)free(tmp);
  return (t2);
}
/******  End of Function readln  ****************************************/

