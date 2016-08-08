extern void tes_sncpy(char *, char *, int);
extern char *tes_xgets(char *, int , FILE *);
extern int fak_parse_line(char *, char *, char *);
extern int fak_use_alt(int);
extern void fak_validate_pixmap(char *, char *);
extern void fak_init_theme(int); 
extern int fak_load_theme(char *, int); 
extern void fak_icon_text(char *, unsigned int, unsigned int, unsigned int); 
extern void fak_text(char *, unsigned int, unsigned int, unsigned int);
extern int fak_flush_expose(Window);
extern void fak_minus(void);
extern void fak_singlemask(int);
extern void fak_maskset();
extern void fak_redraw();

