#ifndef XMMS_SN_H
#define XMMS_SN_H


#define XMMS_SN_BACKWARDS 0
#define XMMS_SN_FORWARDS 1
#define XMMS_SN_HORIZONTAL 0
#define XMMS_SN_VERTICAL 1

#define XMMS_SN_NUM_LETTERS (int)'z' - (int)'!' 



typedef struct
{
    int on;
    int draw;
    int vert;
    int horz;
    int direction;

    int fb;
    int speed;
    GaiColor c;
} Xmms_sn_settings;

void xmms_sn_init(void);
void xmms_sn_update(int);
void xmms_sn_exit(void);
Xmms_sn_settings *xmms_sn_get_settings_ptr(void);


#endif
