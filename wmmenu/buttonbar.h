#ifndef buttonbar_h_
#define buttonbar_h_

extern void ButtonBar_Build (void) ;
extern void ButtonBar_SetPositionFromDockApp (int x, int y, int w, int h) ;
extern void ButtonBar_Show (void) ;
extern void ButtonBar_Highlight (int col, int row) ;
extern void ButtonBar_Unhighlight (void) ;
extern void ButtonBar_Hide (void) ;
extern void ButtonBar_Rebuild (void) ;

#endif /* buttonbar_h_ */
