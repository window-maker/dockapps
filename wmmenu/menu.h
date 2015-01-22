#ifndef menu_h_
#define menu_h_

extern void Menu_LoadFromFile (const char * name) ;
extern int Menu_HasChanged (void) ;
extern void Menu_Reload (void) ;

extern const char * Menu_GetPixmap (void) ;
extern const char * Menu_GetTitle (void) ;

extern int Menu_GetNbEntries (void) ;
extern const char * Menu_GetEntryPixmap (int i) ;
extern const char * Menu_GetEntryCommand (int i) ;

extern void Menu_SetNbRows (const char *s) ;
extern int Menu_GetNbRows (void) ;
extern int Menu_GetNbColumns (void) ;

#endif /* menu_h_ */
