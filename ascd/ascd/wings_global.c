#include <dirent.h>
#include <WINGs.h>

WMScreen *scr;

/* the main window */

WMWindow *win;

WMButton *b_title;

WMPopUpButton *pop, *pop2, *pop3;

WMButton *b_autoplay;
WMButton *b_autorepeat;
WMButton *b_ignoreavoid;
WMTextField *b_device;

WMSlider *b_volume;

WMButton *b_scroll;
WMButton *b_artist;
WMButton *b_upper;

WMTextField *b_cuetime;
WMTextField *b_fadestep;

WMTextField *b_minvol;
WMTextField *b_maxvol;
WMTextField *b_mutedvol;

WMButton *b_mode1;
WMButton *b_mode2;
WMButton *b_mode3;
WMButton *b_mode4;

WMButton *b_save;
WMButton *b_quit;
WMButton *b_close;
WMButton *b_about;

WMButton *b_help;

/* the database window */

WMWindow *dbwin;

WMTextField *db_artist;
WMTextField *db_title;
WMTextField *db_track;
WMButton *db_avoid;
WMButton *db_playauto;

WMSlider *db_volume;

WMButton *db_close;
WMButton *db_prev;
WMButton *db_next;

WMWidget *db_tlabel;

/* the 'about' window */

WMWindow *aboutwin;
WMWidget *about_th1;
WMWidget *about_th2;
WMWidget *about_th3;
WMWidget *about_th4;
WMWidget *about_th5;
WMWidget *about_th6;

/* misc */

int en_vue = FALSE;
int db_en_vue = FALSE;
int about_en_vue = FALSE;
int pistes = 0;

int db_curtrack = 1;
