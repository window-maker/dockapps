#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <X11/X.h>
#include <X11/xpm.h>
#include "wmwifi.h"
#include <net/ethernet.h>
#include <linux/if.h>
#include <linux/socket.h>
#include <linux/wireless.h>

/*
 * This is my quick and dirty way of accessing the Linux Wireless Extensions
 * from the /proc/net/wireless interface.
 * Open the file, and get the Interface name, link, level, and noise. 
 * - Jess
 */

int get_wifi_info(struct wifi *wfi)
{
    struct iwreq wrq;
    struct iw_range range;
    int skfd;
    float ret;
    int i;
    FILE *fp;
    char buff[1024] = "";

    if (fp = fopen("/proc/net/wireless", "r")) {
    //if (fp = fopen("./demo.txt", "r")) {
	for (i = 0; i < (wfi->ifnum + 3); i++) {
	    fgets(buff, sizeof(buff), fp);
	    buff[strlen(buff) + 1] = '\0';
	}
	fclose(fp);
    } else {
	wfi->max_qual = -1;
	fprintf(stdout,
		"ERROR: Cannot Open Linux Wireless Extensions!\n\b");
//	exit(1);
    }

    sscanf(buff, "%s %*s %f %d", &wfi->ifname, &wfi->link, &wfi->level);
    /* Don't know why I have to make a second call to sscanf
     * to pull wfi->noise, but for some wierd reason I cannot
     * pull it above along with the other stats.
     */
    sscanf(buff, "%*s %*s %*s %*s %d", &wfi->noise);

    /* Thanks to Jeroen Nijhof <jnijhof@nijhofnet.nl>
     * for catching this, I guess I overlooked it.
     * Strip out the trailing ':' if exists.
     */
    if (wfi->ifname[strlen(wfi->ifname) - 1] == ':') {
	    wfi->ifname[strlen(wfi->ifname) - 1] = '\0';
    }

    memset(&wfi->essid, 0, IW_ESSID_MAX_SIZE + 1);

    skfd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&range, 0, sizeof(struct iw_range));

    wrq.u.data.pointer = (caddr_t) & range;
    wrq.u.data.length = sizeof(struct iw_range);
    wrq.u.data.flags = 0;
    strncpy(wrq.ifr_name, wfi->ifname, IFNAMSIZ);
    ret = ioctl(skfd, SIOCGIWRANGE, &wrq);
    if (ret > -1) {
      ret = range.max_qual.qual;
    } else {
      fprintf(stdout, "ERROR\n");
    /* Maybe put auto-learning HERE? */
    }

    wfi->max_qual = ret;

    wrq.u.essid.pointer = (caddr_t) wfi->essid;
    wrq.u.essid.length = IW_ESSID_MAX_SIZE + 1;
    wrq.u.essid.flags = 0;
    ret = ioctl(skfd, SIOCGIWESSID, &wrq);

    ret = ioctl(skfd, SIOCGIWRATE, &wrq);
    memcpy(&(wfi->bitrate), &(wrq.u.bitrate), sizeof(struct iw_param));

    if (strlen(wfi->essid) < 1) {
	strcpy(wfi->essid, "not_set");
    }

    if (wfi->link > wfi->max_link) {
	wfi->max_link = wfi->link;
    }
    if (wfi->max_link > wfi->max_qual) {
	wfi->max_qual = wfi->max_link;
    }
    /*
     * For debugging purposes.
     * Check these values against /proc/net/wireless and
     * iwconfig
     *
    fprintf(stdout, "wfi->ifname   -> %s\n", wfi->ifname);
    fprintf(stdout, "wfi->max_qual -> %f\n", wfi->max_qual);
    fprintf(stdout, "wfi->essid    -> %s\n", wfi->essid);
    fprintf(stdout, "wfi->link     -> %f\n", wfi->link);
    fprintf(stdout, "wfi->level    -> %d\n", wfi->level);
    fprintf(stdout, "wfi->noise    -> %d\n", wfi->noise - 0x100);
    fprintf(stdout, "wfi->bitrate  -> %d\n", wfi->bitrate.value);
    fprintf(stdout, "wfi->max_link -> %.f\n", wfi->max_link);
    */
    

    close(skfd);

    return ret;
}

void next_if(struct wifi *wfi)
{
    int max;

    max = get_max_ifs();
    wfi->max_link = 0;

    if (wfi->ifnum < max) {
	wfi->ifnum++;
    } else {
	wfi->ifnum = 0;
    }
}
/* Not used anymore, how many mouse button's do
 * you have?
 */
void last_if(struct wifi *wfi)
{
    int max;

    max = get_max_ifs();
    wfi->max_link = 0;

    if (wfi->ifnum > 0) {
	wfi->ifnum--;
    } else {
	wfi->ifnum = max;
    }
}
int get_max_ifs(void)
{
    FILE *fp;
    char buff[255];
    int i = 0;

    if (fp = fopen("/proc/net/wireless", "r")) {
    //if (fp = fopen("./demo.txt", "r")) {
	for (i = -3; fgets(buff, sizeof(buff), fp); i++) {
	}
	fclose(fp);
    }
    return i;
}
