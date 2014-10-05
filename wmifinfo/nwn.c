
/*
 * This code reads the /proc/driver/poldhu/eth# file for the No Wires Needed poldhu
 * cards. Since there seems to be no official range for the 'Quality' value I assume
 * it's between 0 and 15, and multiply by 4 to get the range 0-63...
 *
 * $Id: nwn.c,v 1.2 2002/09/15 14:31:41 ico Exp $
 *
 */

#ifdef ENABLE_NWN_SUPPORT

#include <stdio.h>
#include <string.h>

#define POLDHUPATH "/proc/driver/poldhu/%s"
#define SWALLOWPATH "/proc/driver/swallow/%s"

int nwn_get_link(char *ifname)
{
	FILE *f;
	char buf[256];
	char *key, *val;
	char *p;
	char bssid[32] = "";
	int inbssid = 0;
	int link = 0;

	sprintf(buf, POLDHUPATH, ifname);
	f = fopen(buf, "r");

	if(f == NULL) {
		sprintf(buf, SWALLOWPATH, ifname);
		f  = fopen(buf, "r");
	}

	if(f == NULL) {
		return(0);
	}

	while(fgets(buf, sizeof(buf), f)) {
		p = strchr(buf, '\n');
		if(p) *p=0;

		p = strchr(buf, ':');

		if(p) {
			*p=0;
			key = buf;
			val = p+2;
			p--;
			while((*p == ' ') || (*p == '\t')) {
				*p=0;
				p--;
			}

			if(strcmp(key, "Current BSSID") == 0) {
				strcpy(bssid, val);
			}

			if((strcmp(key, "BSSID") == 0) &&
			   (strcmp(val, bssid) == 0)) {
			   	inbssid = 1;
			}

			if((inbssid == 1) &&
			   (strcmp(key, "Quality") == 0)) {
			   	sscanf(val, "%X", &link);
			   	link *= 4;
			   	inbssid = 0;
			}
		}
	}

	fclose(f);

	return(link);
}

#endif
