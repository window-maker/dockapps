#ifndef FORECAST_H
#define FORECAST_H

/* functions to manage the 'current forecast' */
struct forecast *current_forecast_get(void);
void current_forecast_next(int dir);

struct forecast {
    char ID[4];             /* Forecast type ("AVN", "MRF", etc) */
    char *station;          /* station name */
    time_t last_update;     /* last updated time */
    signed char month;      /* 0,    1 - 12 */
    signed char day;        /* -1,   1 - 31 */
    short year;             /* -1,   number */
    signed char wday;       /* -1,   0-6 */
    signed char hour;       /* -1,    0 - 23 (local) */
    short low;              /* 999,  -210 - 390 (degrees F) */
    short high;             /* 999,  -210 - 390 (degrees F) */
    short temp;             /* 999,  -210 - 390 (degrees F) */
    short dewpt;            /* 999,  -210 - 390 (degrees F) */
    signed char rh;         /* -1,   0 - 100 (%) */
    short winddir;          /* -1,   0 - 16 (direction) */
    short windspeed;        /* -1,   0 - MAX */
    short heatindex;        /* 999,  -99 - 199 (degrees F) */
    short windchill;        /* 999,  -99 - 199 (degrees F) */
    short precipamt;        /* -1,   0 - 7 (amount code) */
    short snowamt;          /* -1,   0 - 8 (amount code) */
    signed char sky;        /* -1,   0-4 (condition) */
    signed char vis;        /* 7,    1-7 (status code) */
    signed char obs;        /* 0,    0-3 (type) */
    signed char pcp_total;  /* 0,    0-100 (percent chance) */
    signed char frz;        /* 0,    0-100 (percent chance) */
    signed char snow;       /* 0,    0-100 (percent chance) */
    signed char rain;       /* 0,    0-100 (percent chance) */
    signed char tstorm;     /* 0,    0-100 (percent chance) */
    signed char svtstorm;   /* 0,    0-100 (percent chance) */
    double moon;            /* NAN,  -1 - 1 (percent and wax/wane) */
    time_t time;            /* -1,   time_t value */
};

void add_forecast(struct forecast *f, char *ID, char *station);
time_t forecast_time(struct forecast *f);
time_t parse_time_string(char *s);
time_t find_next_time(char *file, char *pat, int minutes);
void reset_forecast(struct forecast *f);

#endif
