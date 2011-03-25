#ifndef PI
# define PI 3.1415926535897932384626433832795029L
#endif

/*
 * Note that all floating point calculations are rounded with .5 going up.
 * Since C automatically truncates, adding .5 to every calculation does
 * rounding for us nicely.
 *
 * To indicate unavailable data
 *   999 is used for temperature
 *   x<0 is used for rh, pressure, and windspeed
 */

/* Calculations */
int rh_C(int temp_C, int dewpt_C);
int rh_F(int temp_F, int dewpt_F);
int heatindex_C(int temp_C, int rh);
int heatindex_F(int temp_F, int rh);
int windchill_C(int temp_C, int windspeed); /* knots */
int windchill_F(int temp_F, int windspeed); /* knots */

/* Length Conversions */
int in2cm(int in);
float m2mi(int meters);

/* Windspeed Conversions */
int knots2mph(int knots);
int knots2kph(int knots);
int knots2mps(int knots);
int knots2beaufort(int knots);
int kph2knots(int kph);
int mps2knots(int mps);

/* Temperature Conversions */
int temp_C2F(int temp_C);
int temp_F2C(int temp_F);

/* Pressure Conversions */
float inHg2mmHg(float inHg);
float inHg2hPa(float inHg);
float inHg2atm(float inHg);
float hPa2inHg(float hPa);

/* Time Conversions */
time_t mkgmtime(struct tm *tm);
int utc2local(int hm, int *mon, int *day, int *year, int *wday);
int local2utc(int hm, int *mon, int *day, int *year, int *wday);
void fix_date(int *month, int *day, int *year, int *wday);
int hm2min(int hm);

/* Letter Case (destructive!) */
char *str_upper(char *str);
char *str_lower(char *str);

/* Angle conversions */
double rad2deg(double angle);
double deg2rad(double angle);

/* Date conversions */
int mdy2doy(int mn, int dy, int y);
double mdy2jd(int year, int month, int day);
double jd2jcent(double jd);
double jcent2jd(double t);

/* Lat/Long conversions */
int str2dd(char *s, double *lat, double *lon);
