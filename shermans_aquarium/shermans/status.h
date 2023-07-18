
/* 

   The linux part is complete and the other OS's parts are more or less non-existing.

   If you use another OS like *BSD, OS X or some unix and are
   interested to make this work on your machine look at status_<your_os>.c and see what is
   needed to be done, please mail me and we can try to make implement the missning parts.

   
*/
#include <gai/gai.h>
#include "defines.h"
#include "../config.h"

#ifdef LINUX
#include "status_linux.h"
#endif

#ifdef FREEBSD
#include "status_freebsd.h"
#endif

#ifdef DARWIN
#include "status_darwin.h"
#endif


#define SENSORS_TEMP1 1
#define SENSORS_TEMP2 2
#define SENSORS_TEMP3 3
#define SENSORS_TEMP4 4
#define SENSORS_FAN1 5
#define SENSORS_FAN2 6
#define SENSORS_FAN3 7


#define SENSORS_CURRENT 1
#define SENSORS_LIMIT 2
#define SENSORS_HYSTERESIS 3

#define NET_RECV 0
#define NET_SENT 1

#define NET_LO 0
#define NET_ETH0 1
#define NET_ETH1 2
#define NET_PPP0 3

#define NET_X NET_SENT+1
#define NET_Y NET_PPP0+1

/* Init status */
void status_init(void);

/* Frees and such stuff done by init */
void status_exit(void);

/* Reports status of Fans, Thermometers and such stuff */
int status_sensors(int);

/* Swap disk usage in % */
int status_swap(void);

/* Disc space usage in % */
int status_disc(char *);

/* CPU load in % - Smoothed down */
int status_cpu(void);

/* Memory usage in % */
int status_mem(void);

/* Network devices usage. Returned is the bytes transfered betwen this and the 
   measurement before.*/
int status_net(int, int);
