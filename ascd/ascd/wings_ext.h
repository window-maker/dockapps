extern WMScreen *scr;

/* the main window */

extern WMWindow *win;

extern WMButton *b_title;

extern WMPopUpButton *pop, *pop2, *pop3;

extern WMButton *b_autoplay;
extern WMButton *b_autorepeat;
extern WMButton *b_ignoreavoid;
extern WMTextField *b_device;

extern WMSlider *b_volume;

extern WMButton *b_scroll;
extern WMButton *b_artist;
extern WMButton *b_upper;

extern WMTextField *b_cuetime;
extern WMTextField *b_fadestep;

extern WMTextField *b_minvol;
extern WMTextField *b_maxvol;
extern WMTextField *b_mutedvol;

extern WMButton *b_mode1;
extern WMButton *b_mode2;
extern WMButton *b_mode3;
extern WMButton *b_mode4;

extern WMButton *b_save;
extern WMButton *b_quit;
extern WMButton *b_close;
extern WMButton *b_about;

extern WMButton *b_help;

/* the database window */

extern WMWindow *dbwin;

extern WMTextField *db_artist;
extern WMTextField *db_title;
extern WMTextField *db_track;
extern WMButton *db_avoid;
extern WMButton *db_playauto;

extern WMSlider *db_volume;

extern WMButton *db_close;
extern WMButton *db_prev;
extern WMButton *db_next;

extern WMWidget *db_tlabel;

/* the 'about' window */

extern WMWindow *aboutwin;
extern WMWidget *about_th1;
extern WMWidget *about_th2;
extern WMWidget *about_th3;
extern WMWidget *about_th4;
extern WMWidget *about_th5;
extern WMWidget *about_th6;

/* misc */

extern int en_vue;
extern int db_en_vue;
extern int about_en_vue;
extern int pistes;
int db_curtrack;
