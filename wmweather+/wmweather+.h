#define BIGBUF_LEN 4096

char *get_filename(char *file);
char *get_pid_filename(char *file);

extern char *ProgName;
extern int devnull;
extern char *bigbuf;
extern char *monthnames[];
extern char *monthnames2[];
extern char *wdaynames[];
extern char *directions[];

extern char *email_address;
extern char *metar_station;
extern char *metar_uri;
extern char *metar_post;
extern char **warning_zones;
extern char *warning_uri;
extern char *warning_post;
extern char *avn_station;
extern char *avn_uri;
extern char *avn_post;
extern char *eta_station;
extern char *eta_uri;
extern char *eta_post;
extern char *mrf_station;
extern char *mrf_uri;
extern char *mrf_post;
extern char *radar_uri;
extern char *radar_post;
extern char *radar_crop;
extern char *radar_cross;
extern char *viewer;
extern int pressure_mode;
extern int windspeed_mode;
extern int temp_mode;
extern int length_mode;
extern double latitude, longitude;
extern int start_do_animation;
extern int starting_mode;
