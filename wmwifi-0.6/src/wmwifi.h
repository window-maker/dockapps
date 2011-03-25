/* Standard headers */
#include <sys/types.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>		/* gethostbyname, getnetbyname */
#include <net/ethernet.h>	/* struct ether_addr */
#include <sys/time.h>		/* struct timeval */
#include <linux/wireless.h>

#define KILO    1e3
#define MEGA    1e6
#define GIGA    1e9

extern Bool wmwifi_learn;

struct wifi {
    char 	ifname[255];
    char 	essid[IW_ESSID_MAX_SIZE + 1];
    int 	ifnum; 
    float 	link;
    int 	level;
    unsigned int 	noise;
    float 	max_link;
    float 	max_qual;
    struct iw_param 	bitrate;
};
int get_wifi_info(struct wifi *wfi);
int get_max_ifs(void);
void next_if(struct wifi *wfi);
//void last_if(struct wifi *wfi);
