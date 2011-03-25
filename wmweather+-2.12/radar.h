#include <X11/xpm.h>

extern time_t radar_update_time;
extern Pixmap radar;
extern int do_radar_cross;

void init_radar(void);
void update_radar(int force);
void put_radar(int x, int y, int font);
void radar_cleanup(void);
