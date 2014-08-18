#ifndef _LIBACPI_H_
#define _LIBACPI_H_


#define LIBACPI_VER "0.92"

/* Here because we need it for definitions in this file . . . */
#define MAX_NAME 128
#define MAXBATT 8
#define SAMPLES 50

typedef enum {
    REMAIN,
    TIMER
} DspMode;

typedef enum {
    AC,
    BATT,
    PS_ERR,
} power_state_t;
    
typedef enum {
    HIGH,
    MED,
    LOW,
    CRIT,
    HARD_CRIT,
    BS_ERR,
} batt_state_t;

typedef enum {
    CHARGE,
    DISCHARGE,
    CH_ERR,
} charge_state_t;

typedef enum {
    OK,
    CRITICAL,
    CS_ERR,
} cap_state_t;

typedef struct {
    /* general info */
    char name[MAX_NAME];
    /* these two are conveniences */
    char info_file[MAX_NAME];
    char state_file[MAX_NAME];
    int present; 
    int design_cap;		/* assuming mAh */
    int last_full_cap;
    int design_voltage;		/* in mV */
    /* state info */
    cap_state_t capacity_state;
    charge_state_t charge_state;
    int present_rate;		/* in mAh */
    int remaining_cap;		/* in mAh */
    int present_voltage;	/* in mV */
    /* calculated states */
    batt_state_t state;
    int percentage;		/* stored here because this is a per battery thing */
    int charge_time;		/* time left to charge this battery */
    /* and a flag to indicate that this is valid . . . */
    int valid;
    /* number of times we've gotten bad info on this battery's present rate */
    int bad_count;		
} battery_t;
    
typedef struct {
    char *name;
    char state_file[MAX_NAME];
    power_state_t power;
} adapter_t;

/* how to calculate the time remaining */
enum rtime_mode {
    RT_RATE,			/* using the current rate, as per the ACPI spec */
    RT_CAP,			/* using the remaining capacity over time */
};

typedef struct {
    int rtime;			/* remaining time */
    int timer;			/* how long been on battery? */
    int crit_level;		/* anything below this is critical low */
    int battery_count;		/* number of batteries found */
    enum rtime_mode rt_mode;	/* remaining time mode */
    int rt_forced;		/* was our rt_mode forced? if so, we do what we were told */
    battery_t *binfo;		/* pointer to the battery being monitored */
    adapter_t adapter;
} global_t;

/*
 * Moving percentage to the battery is right, but I think we need a global
 * remaining capacity somewhere, too . . . 
 */

/*
 * To provide a convenient debugging function . . . 
 *
 * It's a macro because I'm too lazy to deal with varargs.
 */

#define pdebug(fmt, arg...)				\
    do {						\
	if (verbosity > 2)				\
	    fprintf(stderr, fmt, ##arg);		\
    } while (0)

#define pinfo(fmt, arg...)				\
    do {						\
	if (verbosity > 1)				\
	    fprintf(stderr, fmt, ##arg);		\
    } while (0)

#define perr(fmt, arg...)				\
    do {						\
	if (verbosity > 0)				\
	    fprintf(stderr, fmt, ##arg);		\
    } while (0)

#define pfatal(fmt, arg...)				\
    fprintf(stderr, fmt, ##arg)				\
	

/* Since these /are/ needed here . . . */
battery_t batteries[MAXBATT];
int verbosity;

/* check if apm/acpi is enabled, etc */
int power_init(global_t *globals);
/* reinitialise everything */
int power_reinit(global_t *globals);
int reinit_ac_adapters(global_t *globals);
int reinit_batteries(global_t *globals);

/* fill global_t with data */
void acquire_batt_info(global_t *globals, int batt);
void acquire_all_batt_info(global_t *globals);
void acquire_global_info(global_t *globals);
void acquire_all_info(global_t *globals);

#endif /* _WMACPI_H_ */
