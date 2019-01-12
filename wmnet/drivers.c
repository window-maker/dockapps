/*  wmnet -- X IP accounting monitor
 *  Copyright 1998 Jesse B. Off  <joff@iastate.edu>
 *
 *  $Id: drivers.c,v 1.1 1998/10/07 03:42:21 joff Exp joff $
 *
 *  This software is released under the GNU Public License agreement.
 *  No warranties, whatever.... you know the usuals.... this is free
 *  software.  if you use it, great... if you wanna make a change to it,
 *  great, but please send me the diff.
 */

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<X11/Xlib.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<unistd.h>
#include"config.h"


/* For FreeBSD */
#ifdef USE_KVM
#include<net/if.h>
#include<kvm.h>
#include<nlist.h>

kvm_t *kvmfd;
struct nlist symbols[] = {
 { "_ifnet" },
 { NULL }
};
unsigned long ifnet_savedaddr;
int kvm_test(void);
int kvm_updateStats(void);
#endif /* USE_KVM */


#ifdef USE_LINUX_PPP
#include<net/ppp_defs.h>

#ifdef linux_libc5
# include<linux/if_ppp.h>
#else
# include<net/if_ppp.h>
#endif

#include<sys/ioctl.h>
int pppfd;
int ppp_test(void);
int updateStats_ppp(void);
static struct ifpppstatsreq ppp_stats_req;
#endif

#define ACCOUNT_IN_FOUND        1
#define ACCOUNT_OUT_FOUND       2

extern char buffer[256];
extern char *in_rule_string, *out_rule_string, *device;
extern unsigned long long int totalbytes_in, totalbytes_out, lastbytes_in, lastbytes_out;
extern unsigned long long int totalpackets_in, totalpackets_out, lastpackets_in, lastpackets_out;
extern unsigned int diffbytes_in, diffbytes_out;
extern unsigned int out_rule, in_rule;  /* number of rule in /proc/net/ip_acct to use */
extern Bool current_tx, current_rx, rx, tx;



char *available_drivers(void);

#ifdef USE_IPFWADM
int updateStats_ipfwadm(void);
int ipfwadm_test(void);
#endif
#ifdef USE_IPCHAINS
int updateStats_ipchains(void);
int ipchains_test(void);
#endif
#ifdef USE_2_1_DEV
int updateStats_dev(void);
int dev_test(void);
#endif


typedef int (*parser_func)(void);
static struct drivers_struct {
 char * name;
 parser_func function;
 parser_func test;
} drivers[] = {
#ifdef USE_2_1_DEV
 {"devstats", updateStats_dev, dev_test},
#endif
#ifdef USE_IPFWADM
 {"ipfwadm", updateStats_ipfwadm, ipfwadm_test},
#endif
#ifdef USE_IPCHAINS
 {"ipchains", updateStats_ipchains, ipchains_test},
#endif
#ifdef USE_LINUX_PPP
 {"pppstats",updateStats_ppp, ppp_test},
#endif
#ifdef USE_KVM
 {"kmem",kvm_updateStats, kvm_test},
#endif
 {NULL, NULL}
};

char* available_drivers(void) {
 int ind = 0;
 int len = 0;
 char *string, *ptr;
 while(drivers[ind].name != NULL) {
  len += strlen(drivers[ind].name) + 1;
  ind++;
 }
 ptr = string = (char *)malloc(len);
 *string = '\0';
 ind = 0;
 while(drivers[ind].name != NULL) {
  strcpy(string, drivers[ind].name);
  string += strlen(drivers[ind].name);
  *string++ = ',';
  ind++;
 }
 *(--string) = '\0';
 return ptr;
}



parser_func find_driver(void) {
 int ind = 0;
 while(drivers[ind].name != NULL) {
  if(drivers[ind].test()) {
   return drivers[ind].function;
  }
  ind++;
 }
 fprintf(stderr, "wmnet: no appropriate stat driver found\n");
 exit(30);
}


parser_func setup_driver(char * parser_name) {
 int ind = 0;
 if (parser_name == NULL) return find_driver();
 while(drivers[ind].name != NULL) {
  if(!strcmp(parser_name, drivers[ind].name)) {
    if (drivers[ind].test()) return drivers[ind].function;
    fprintf(stderr, "wmnet: driver %s not appropriate for this machine\n", parser_name);
    exit(18);
  }
  ind++;
 }
 fprintf(stderr, "wmnet: no driver %s\n", parser_name);
 exit(18);
}




#ifdef linux
/* All the data gathering is done in here.
 * Return True if no change to tx/rx.
 * Return False if display will need to be updated.
 */
#ifdef USE_IPFWADM
int ipfwadm_test(void) {
  if(open("/proc/net/ip_acct", O_RDONLY) == -1) return False;
  fprintf(stderr, "wmnet: using ipfwadm driver to monitor accounting rules %d and %d\n", in_rule, out_rule);
  return True;
}

int updateStats_ipfwadm(void) {
	FILE *ip_acct;
	unsigned int flag = 0, lineno = 0;
        unsigned int offset = 37;
	char *ptr;
	rx = False;
	tx = False;


	if ((ip_acct = fopen("/proc/net/ip_acct", "r")) == NULL) {
		fprintf(stderr, "wmnet: /proc/net/ip_acct unavailable\n"
				"You either don't have IP accounting compiled in, or this isn't a 2.0 linux kernel.\n");
		exit(4);
	}

	/* IP Accounting Rules for 2.0.x linux kernels*/
	while(flag != (ACCOUNT_IN_FOUND|ACCOUNT_OUT_FOUND) && fgets(buffer, 256, ip_acct)) {
		switch(lineno == out_rule ? ACCOUNT_OUT_FOUND : ( lineno == in_rule ? ACCOUNT_IN_FOUND : -1 ) ) {
			case ACCOUNT_IN_FOUND:
				/* accounting in */
				flag |= ACCOUNT_IN_FOUND;
				while(buffer[offset++] != ' ');
				offset += 18;
				totalpackets_in = strtoul(&buffer[offset], &ptr, 10);
				if (totalpackets_in == lastpackets_in) break;
				totalbytes_in = strtoul(ptr, NULL, 10);
				diffbytes_in += totalbytes_in - lastbytes_in;
				lastpackets_in = totalpackets_in;
				lastbytes_in = totalbytes_in;
				rx = True;
				break;
			case ACCOUNT_OUT_FOUND:
				/* accounting out */
				flag |= ACCOUNT_OUT_FOUND;
				while(buffer[offset++] != ' ');
				offset += 18;
				totalpackets_out = strtoul(&buffer[offset], &ptr, 10);
				if (totalpackets_out == lastpackets_out) break;
				totalbytes_out = strtoul(ptr, NULL, 10);
				diffbytes_out += totalbytes_out - lastbytes_out;
				lastpackets_out = totalpackets_out;
				lastbytes_out = totalbytes_out;
				tx = True;
				break;
		}
		lineno++;
		offset = 37;
	}

	if(flag != (ACCOUNT_IN_FOUND|ACCOUNT_OUT_FOUND)) {
		fprintf(stderr,"wmnet: couldn't find %s accounting rule to monitor in /proc/net/ip_acct\n",
		  (flag == ACCOUNT_IN_FOUND) ? "the TX" : ((flag == ACCOUNT_OUT_FOUND) ? "the RX" : "a single"));
		exit(4);
	}
	fclose(ip_acct);

	/* return True if no change to tx/rx
	 * return False if display will need to be updated
	 */
	return((rx == current_rx) && (tx == current_tx));

}
#endif /* USE_IPFWADM */

#ifdef USE_IPCHAINS
int ipchains_test(void) {
  if (open("/proc/net/ip_fwchains",O_RDONLY) == -1) return False;
  if (in_rule_string == NULL) in_rule_string = "acctin";
  if (out_rule_string == NULL) out_rule_string = "acctout";
  fprintf(stderr, "wmnet: using ipchains driver to monitor chains %s and %s\n", in_rule_string, out_rule_string);
  return True;
}

/* ipchains parser mostly from Bjoern Kriews <bkr@cut.de> */
int updateStats_ipchains(void) {
	FILE *ip_acct;
	unsigned int flag = 0;
	static char name[32];
	unsigned long pack, bytes;
	rx = False;
	tx = False;


	if ((ip_acct = fopen("/proc/net/ip_fwchains", "r")) == NULL) {
		fprintf(stderr, "/proc/net/ip_fwchains does not exist?\n"
				"Do you have IP accounting in your kernel?\n");
		exit(4);
	}

	/* IP Chain Rules for Linux kernel 2_1.x */
	while(flag != (ACCOUNT_IN_FOUND|ACCOUNT_OUT_FOUND) && fgets(buffer, 256, ip_acct)) {
		*name = 0;

		sscanf(buffer, "%30s %*s - %*d %*d %*d %*d %lu %*d %lu",
			name, &pack, &bytes);


		if(strcmp(name, in_rule_string) == 0) {
			flag |= ACCOUNT_IN_FOUND;

			totalpackets_in = pack;
			if (totalpackets_in != lastpackets_in) {
				totalbytes_in = bytes;
				diffbytes_in += totalbytes_in - lastbytes_in;
				lastpackets_in = totalpackets_in;
				lastbytes_in = totalbytes_in;
				rx = True;
			}

		} else if (strcmp(name, out_rule_string) == 0) {
			flag |= ACCOUNT_OUT_FOUND;

			totalpackets_out = pack;
			if (totalpackets_out != lastpackets_out) {
				totalbytes_out = bytes;
				diffbytes_out += totalbytes_out - lastbytes_out;
				lastpackets_out = totalpackets_out;
				lastbytes_out = totalbytes_out;
				tx = True;
			}
		}
	}

	if(flag != (ACCOUNT_IN_FOUND|ACCOUNT_OUT_FOUND)) {
		fprintf(stderr,"I couldn't find %s IP chain to monitor in /proc/net/ip_fwchains.\n",
		  (flag == ACCOUNT_IN_FOUND) ? "the TX" : ((flag == ACCOUNT_OUT_FOUND) ? "the RX" : "a single"));
		exit(4);
	}
	fclose(ip_acct);

	/* return True if no change to tx/rx
	 * return False if display will need to be updated
	 */
	return((rx == current_rx) && (tx == current_tx));

}
#endif /* USE_IPCHAINS */


#ifdef USE_2_1_DEV

int updateStats_dev(void) {
	FILE *dev;
        char *ptr;
	unsigned int flag = 0;
	static char interface[16];
	rx = False;
	tx = False;


	if ((dev = fopen("/proc/net/dev", "r")) == NULL) {
		fprintf(stderr, "/proc/net/dev does not exist?\n"
				"Perhaps we are not running Linux?\n");
		exit(4);
	}
        /* the first two lines we can skip */
        fgets(buffer, 256, dev);
        fgets(buffer, 256, dev);

	/* IP Chain Rules for Linux kernel 2_1.x */
	while(flag != (ACCOUNT_IN_FOUND|ACCOUNT_OUT_FOUND) && fgets(buffer, 256, dev)) {

		sscanf(buffer, "%16s %llu %llu %*d %*d %*d %*d %*d %*d %llu %llu %*d %*d %*d %*d %*d %*d",
		       interface, &totalbytes_in, &totalpackets_in, &totalbytes_out, &totalpackets_out);

		/* strip trailing colon */
		ptr = interface;
		while(*ptr != ':') ptr++;
		*ptr = '\0';

		if (!strcmp(interface, device)) {

			flag = (ACCOUNT_IN_FOUND|ACCOUNT_OUT_FOUND);

			if (totalpackets_in != lastpackets_in) {
				diffbytes_in += totalbytes_in - lastbytes_in;
				lastpackets_in = totalpackets_in;
				lastbytes_in = totalbytes_in;
				rx = True;
			}

			if (totalpackets_out != lastpackets_out) {
				diffbytes_out += totalbytes_out - lastbytes_out;
				lastpackets_out = totalpackets_out;
				lastbytes_out = totalbytes_out;
				tx = True;
			}
		}
	}

	fclose(dev);

	/* return True if no change to tx/rx
	 * return False if display will need to be updated
	 */
	return((rx == current_rx) && (tx == current_tx));
}

int dev_test(void) {
  int devfd;
  if((devfd = open("/proc/net/dev", O_RDONLY)) == -1) return False;
  read(devfd, buffer, 36);
  if(buffer[35] == '|') return False;
  if(device == NULL) device = "eth0";
  fprintf(stderr, "wmnet: using devstats driver to monitor %s\n", device);
  return True;
}

#endif /* USE_2_1_DEV */

#ifdef USE_LINUX_PPP
int ppp_test(void) {
  pppfd = socket(AF_INET, SOCK_DGRAM, 0);
  if(device == NULL) device = "ppp0";
  strncpy(ppp_stats_req.ifr__name, device, 15);
  ppp_stats_req.stats_ptr =(caddr_t) &ppp_stats_req.stats;
  fprintf(stderr, "wmnet: using pppstats driver to monitor %s\n", device);
  return True;

}

int updateStats_ppp(void) {
 if(ioctl(pppfd, SIOCGPPPSTATS, &ppp_stats_req) != 0) {
   return False;
 }
        totalpackets_in = ppp_stats_req.stats.p.ppp_ipackets;
        if (totalpackets_in != lastpackets_in) {
                totalbytes_in = ppp_stats_req.stats.p.ppp_ibytes;
                diffbytes_in += totalbytes_in - lastbytes_in;
                lastpackets_in = totalpackets_in;
                lastbytes_in = totalbytes_in;
                rx = True;
        }


        totalpackets_out = ppp_stats_req.stats.p.ppp_opackets;
        if (totalpackets_out != lastpackets_out) {
                totalbytes_out =ppp_stats_req.stats.p.ppp_obytes;
                diffbytes_out += totalbytes_out - lastbytes_out;
                lastpackets_out = totalpackets_out;
                lastbytes_out = totalbytes_out;
                tx = True;
        }

	/* return True if no change to tx/rx
	 * return False if display will need to be updated
	 */
  return((rx == current_rx) && (tx == current_tx));

}


#endif /* USE_LINUX_PPP */


#endif /* linux */

#ifdef USE_KVM
int kvm_test(void) {
  if (((kvmfd = kvm_open(NULL, NULL, NULL, O_RDONLY, buffer)) == NULL) ||
      (kvm_nlist(kvmfd, symbols) < 0) ||
      kvm_read(kvmfd, (unsigned long)symbols[0].n_value, &ifnet_savedaddr, sizeof(unsigned long)) == -1 ) return False;
  if(device == NULL) device = "ec0";
  fprintf(stderr, "wmnet: using kmem driver to monitor %s\n", device);
  return True;
}

int kvm_updateStats(void) {
 struct ifnet * ifnet = (struct ifnet *)buffer;
 unsigned long ifnet_addr = ifnet_savedaddr;
 char devname[16];
 int flag = 0;
 while (ifnet_addr && flag != (ACCOUNT_IN_FOUND|ACCOUNT_OUT_FOUND)) {
  kvm_read(kvmfd, ifnet_addr, buffer, sizeof(struct ifnet));
#ifdef __OpenBSD__
  snprintf(devname, 15, "%s", ifnet->if_xname);
#else
  kvm_read(kvmfd, (unsigned long)ifnet->if_name, devname, 15);
  snprintf(devname, 15, "%s%d", devname, ifnet->if_unit);
#endif
  if(!strncmp(devname, device, strlen(device))) { /* we found our struct */
        totalpackets_in =ifnet->if_ipackets;
        if (totalpackets_in != lastpackets_in) {
                totalbytes_in = ifnet->if_ibytes;
                diffpackets_in += totalpackets_in - lastpackets_in;
                diffbytes_in += totalbytes_in - lastbytes_in;
                lastpackets_in = totalpackets_in;
                lastbytes_in = totalbytes_in;
                rx = True;
        }


        totalpackets_out = ifnet->if_opackets;
        if (totalpackets_out != lastpackets_out) {
                totalbytes_out =ifnet->if_obytes;
                diffpackets_out += totalpackets_out - lastpackets_out;
                diffbytes_out += totalbytes_out - lastbytes_out;
                lastpackets_out = totalpackets_out;
                lastbytes_out = totalbytes_out;
                tx = True;
        }
        flag = (ACCOUNT_IN_FOUND|ACCOUNT_OUT_FOUND);

  } else {
#ifdef __OpenBSD__
        ifnet_addr = (unsigned long)ifnet->if_list.tqe_next;
#else
        ifnet_addr = (unsigned long)ifnet->if_next;
#endif
  }
 }

/* return True if no change to tx/rx
 * return False if display will need to be updated
 */
 return((rx == current_rx) && (tx == current_tx));
}


#endif




