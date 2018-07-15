#ifndef MOUSE_REGIONS_H_INCLUDED
#define MOUSE_REGIONS_H_INCLUDED

  /***********/
 /* Defines */
/***********/

#define MAX_MOUSE_REGION (9)

  /***********************/
 /* Function Prototypes */
/***********************/

void AddMouseRegion(int index, int left, int top, int right, int bottom);
int CheckMouseRegion(int x, int y);
void EnableMouseRegion(int index);
void DisableMouseRegion(int index);

#endif
