#ifndef _WMACPI_H_
#define _WMACPI_H_

#ifdef PRO
#define eprint(level, fmt, arg...)					\
	switch (level) {						\
	case 0:								\
	    break;							\
	case 1:								\
	    fprintf(stderr, __FUNCTION__": " fmt, ##arg);		\
	    fprintf(stderr, "\n");					\
	    break;							\
	}
#else
#define eprint(level, fmt, arg...) \
	do { } while (0)
#endif

typedef enum {
    REMAIN,
    TIMER
} DspMode;

typedef enum {
    BLINK,
    OFF
} Mode;

typedef enum {
    POWER,			/* on AC, Battery charged */
    CHARGING,			/* on AC, Charging */
    HIGH,			/* on Battery, HIGH */
    LOW,			/* on Battery, LOW */
    CRIT			/* on Battery, CRIT */
} State;

typedef struct {
    State power;		/* power state: Battery levels or AC */
    int percentage;		/* battery percentage (-1 if no battery) */
    int rtime;			/* remaining time */
    int timer;			/* how long been on battery? */
    int crit_level;		/* anything below this is critical low */
} APMInfo;

/* detect plugin events */
void process_plugin_timer(void);
/* check if apm/acpi is enabled, etc */
int power_init(void);
/* fill APMInfo with data */
void acquire_info(void);

#endif /* _WMACPI_H_ */
