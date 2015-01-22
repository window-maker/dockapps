#ifndef options_h_
#define options_h_

#include "types.h"

/* defaults to "not set" */
extern int TileXSize ;
extern int TileYSize ;

/* defaults to "not set" */
extern char MenuName [] ;

/* empty by default; can be written to */
extern char * PixmapPath ;

/* empty by default; can be written to */
extern char * TilePath ;

/* empty by default; can be written to */
extern char * HighlightPath ;

/* defaults to false */
extern bool ClickOnly ;

/* defaults to true */
extern bool AutoScale ;

/* defaults to false */
extern bool HighlightBehind ;

/* defaults to 1ms */
extern int HideTimeout ;

extern int Options_Argc ;
extern char * Options_Argv [] ;

extern void Options_ParseDefaults (void) ;
extern void Options_Parse (int argc, char ** argv) ;
extern void Options_SetMenuName (const char *) ;

#endif /* options_h_ */
