struct current_weather {
    time_t last_update;
    int month;          /* 0,    1 - 12 */
    int date;           /* -1,   1 - 31 (GMT) */
    short time;         /* -1,   0000 - 2359 */
    short temp;         /* 999,  -210 - 390 (degrees C) */
    signed char rh;     /* -1,   0 - 100 (%) */
    short winddir;      /* -1,   0 - 16 (direction) */
    short windspeed;    /* -1,   0 - MAX (knots) */
    float pressure;     /* -1,   0 - MAX (inHg) */
    short heatindex;    /* 999,  -99 - 199 (degrees F) */
    short windchill;    /* 999,  -99 - 199 (degrees F) */
    signed char sky;    /* -1,   0-4 (condition) */
    signed char vis;    /* 7,    1-7 (status code) */
    signed char obs;    /* 0,    0-3 (type) */
    signed char frz;    /* 0,    0, 33, 66, 99 (intensity) */
    signed char snow;   /* 0,    0, 33, 66, 99 (intensity) */
    signed char rain;   /* 0,    0, 33, 66, 99 (intensity) */
    signed char tstorm; /* 0,    0, 33, 66, 99 (intensity) */
    double moon;        /* NAN,  -1 - 1 (percent and wax/wane) */
};

extern struct current_weather current;

void init_metar(void);
void update_metar(int force);
void metar_cleanup(void);
