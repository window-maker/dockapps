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

int get_max_qual(char ifname[255]);
struct wifi wifi_info(void);
struct wifi {
    char ifname[255];
    int ifnum;
    int link;
    char level[255];
    char noise[255];
    float max_qual;
    float status;
};
