
/* Header file for Sherman's aquarium's date displayer <cja@gmx.net> */

#ifndef DATE_H
#define DATE_H

typedef struct
{
    int on;			/* Are we displaying date or not? */
    int draw;
    int horz;
    int vert;

    GaiColor c;
    
} Date_settings;

void date_init(void);
void date_update(int);
void date_exit(void);
Date_settings *date_get_settings_ptr(void);
#endif

