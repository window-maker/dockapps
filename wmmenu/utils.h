#ifndef utils_h_
#define utils_h_

#include <stdio.h>
#include <string.h>
#include "types.h"

extern char * File_ReadAll (FILE *) ;
extern bool File_FindInPath (char * out, int outSz,
    const char * path, const char * basename) ;

#define streq(S1,S2) (strcmp ((S1), (S2)) == 0)
#define streql(S1,S2,L) (strncmp ((S1), (S2), (L)) == 0)

extern char * File_ReadOutputFromCommand (const char * cmd) ;

#endif /* utils_h_ */
