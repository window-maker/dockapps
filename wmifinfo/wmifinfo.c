/*
 * $Id: wmifinfo.c,v 1.3 2004/03/03 18:29:50 ico Exp $
 */
 	 
#include <stdio.h>
#include <unistd.h>
#ifdef linux
#include <getopt.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <X11/X.h>
#include <X11/xpm.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#if defined(__OpenBSD__)
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/route.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <netinet/if_ether.h>
#include <net/if_ieee80211.h>
#include <dev/ic/if_wi_ieee.h>
#include <dev/ic/if_wireg.h>
#include <dev/ic/if_wi_hostap.h>
#define ROUNDUP(a) \
        ((a) > 0 ? (1 + (((a) - 1) | (sizeof(long) - 1))) : sizeof(long))
#endif
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#ifdef ENABLE_NWN_SUPPORT
#include "nwn.h"
#endif

#include "xutils.h"
#include "bitmaps/wmifinfo_led.xbm"
#include "bitmaps/wmifinfo_led.xpm"
#include "bitmaps/wmifinfo_lcd.xbm"
#include "bitmaps/wmifinfo_lcd.xpm"

#define MAXIFS 10
#ifdef linux
#define DELAY 1000000L
#else
#define DELAY 100000L
#endif
#define MODE_LED 1
#define MODE_LCD 2

struct ifinfo_t {
	char id[16];
	int state;
	char hw[6];
	uint32_t ip;
	uint32_t nm;
	uint32_t gw;
	int sl;
	int bytes;
};

struct font_t {
	char *chars;
	int sx;
	int sy;
	int dx;
	int dy;
	int charspline;
};


char	LabelColor[30]    	= "#79bdbf";
char	WindGustColor[30] 	= "#ff0000";
char	DataColor[30]     	= "#ffbf50";
char	BackColor[30]     	= "#181818";
char	StationTimeColor[30]    = "#c5a6ff";

struct font_t font1 = { " 0123456789ABCDEF", 64, 74, 4, 5, 17};
struct font_t font2 = { "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789: ", 1, 65, 5, 7, 26};

char *exec_dflt = "echo Use the -u and -d options for setting ifup/ifdown commands.";
char *exec_up;
char *exec_down;

int mode = MODE_LED;
char startif[16] = "";
char ifname[MAXIFS][16];
int ifaces;
int ifno = 0;
struct ifinfo_t ifinfo;
int fd = 0;
struct ifconf ifc;
volatile int exec_busy=0;


void parse_cmdline(int argc, char *argv[]);
void ButtonPressEvent(XButtonEvent *);
char *strupper(char *str);
void getifnames(void);
int getifinfo(char *ifname, struct ifinfo_t *info);


	

void drawtext(char *str, struct font_t *font, int x, int y)
{
	int i = 0;
	int ix, iy;
	char *p;
		
	while(str[i]) {
		p = strchr(font->chars, str[i]);
		ix = (p) ? (p - font->chars) : 0;
		
		iy = (ix / font->charspline);
		ix = (ix % font->charspline);
	
		copyXPMArea(	font->sx + ix * font->dx, 
				font->sy + iy * font->dy,
				font->dx, 
				font->dy,
				x + font->dx * i, 
				y);
				
		i++;
	}
}


void drawipaddr(uint32_t a, int linenum)
{
	char buf[4];
	int i;
	uint32_t addr = ntohl(a);
	
	for(i=0; i<4; i++) {
		snprintf(buf, 4, "%3d", (addr >> ((3-i)*8)) & 255);
		drawtext(buf, &font1, 5 + i*14, 19 + linenum*9);
	}
}

void drawhwaddr(unsigned char *addr)
{
	char buf[4];
	int i;
	
	for(i=0; i<6; i++) {
		snprintf(buf, 4, "%02X", addr[i]);
		drawtext(buf, &font1, 6 + i*9, 46);
	}
}

int main(int argc, char *argv[])
{

	XEvent event;
	char buf[16];
	int d=0;
	int pifno=-1;
	int lastbytes=0;
	struct timeval tv;
	fd_set fds;
	int x;
	int prev_exec_busy=0;

	exec_up = exec_dflt;
	exec_down = exec_dflt;
	
	parse_cmdline(argc, argv);

	initXwindow(argc, argv);

	if(mode == MODE_LED) {
		openXwindow(argc, argv, wmifinfo_led_xpm, wmifinfo_led_bits,
			wmifinfo_led_width, wmifinfo_led_height, BackColor,
			LabelColor, WindGustColor, DataColor, StationTimeColor);
	} else {
		openXwindow(argc, argv, wmifinfo_lcd_xpm, wmifinfo_lcd_bits,
			wmifinfo_lcd_width, wmifinfo_lcd_height, BackColor,
			LabelColor, WindGustColor, DataColor, StationTimeColor);
	}

	// Initialize global variables

	fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	ifc.ifc_len = sizeof(struct ifreq) * 10;
	ifc.ifc_buf = malloc(ifc.ifc_len);

	while(1) {
	
		while (XPending(display)) {
			XNextEvent(display, &event);
			
			switch (event.type) {
				case Expose:
					RedrawWindow();
					break;
				case ButtonPress:
					ButtonPressEvent(&event.xbutton);
					break;
				case ButtonRelease:
					break;

			}
		}

		if ((d++ == 3) || (ifno != pifno) || (exec_busy != prev_exec_busy)) {
		
			d=0;
			
			copyXPMArea(64, 0, 64, 64, 0, 0);
			getifnames();

			if(ifaces>0) {
				ifno = ifno % ifaces;
			
				getifinfo(ifname[ifno], &ifinfo);
			
				if(ifno != pifno) lastbytes = ifinfo.bytes;
				
				sprintf(buf, "%-7s", ifinfo.id);	
				strupper(buf);
				drawtext(buf, &font2, 6, 4);

				if(memcmp(ifinfo.hw, "\x00\x00\x00\x00\x00\x00", 6) != 0) {
					drawhwaddr(ifinfo.hw);
				}
			
				if(ifinfo.ip) drawipaddr(ifinfo.ip, 0);
				if(ifinfo.nm) drawipaddr(ifinfo.nm, 1);
				if(ifinfo.gw) drawipaddr(ifinfo.gw, 2);

				// WLAN signal level
				
#ifdef linux
				x = ifinfo.sl/4;
#elif defined(__OpenBSD__)
				x = ifinfo.sl/7;
#endif
				if(x>13) x=13;
				copyXPMArea(4, 82, x*4, 4, 6, 53);
				
				// LED
				
				x=0;
				if(exec_busy) { 
					x=0;
				} else {
					if(ifinfo.state) x += 8;
					if(lastbytes == ifinfo.bytes) x+= 16;
				}
				copyXPMArea(64 + x, 81, 8, 8, 50, 4);
				lastbytes = ifinfo.bytes;
				
			}

			RedrawWindow();
			prev_exec_busy = exec_busy;
			pifno = ifno;
		}

#ifdef linux
		tv.tv_sec = 0;
		tv.tv_usec = DELAY;
		
		FD_ZERO(&fds);
		FD_SET(ConnectionNumber(display), &fds);
		
		select(ConnectionNumber(display)+1, &fds, NULL, NULL, &tv);
#elif defined(__OpenBSD__)
		usleep(DELAY);
#endif

	}

}

void print_usage()
{
	printf(
		"usage: wmifinfo [-lh] [-i interface]\n"
		"  -d <cmd>         Command to exec for iface-down\n"
		"  -i <interface>   Start with given interface, if available\n"
		"  -l               LCD display mode\n"
		"  -h               Print this help\n"
		"  -u <cmd>         Command to exec for iface-up\n"
		"  -v               Show version info and exit\n"
	);

}

void print_version()
{
	printf("%s version %s, compile-time options: ", NAME, VERSION);
	
#ifdef ENABLE_NWN_SUPPORT
	printf("ENABLE NWN SUPPORT ");
#endif

	printf("\n");
}

void parse_cmdline(int argc, char *argv[])
{
	int c;
	
	while((c = getopt(argc, argv, "d:i:lhu:v")) != EOF) {
		switch(c) {
			case 'd' :
				exec_down = strdup(optarg);
				break;
			case 'i' :
				strncpy(startif, optarg, sizeof(startif));
				break;
			case 'l' :
				mode = MODE_LCD;
				break;
			case 'h' :
				print_usage();
				exit(0);
			case 'u' :
				exec_up = strdup(optarg);
				break;
			case 'v' :
				print_version();
				exit(0);
		}
	}

}

void sigchldhandler(int a)
{
	wait(NULL);
	exec_busy = 0;
}

void exec_cmd(char *cmd)
{
	int pid;
	
	signal(SIGCHLD, sigchldhandler);

	if(exec_busy) return;
	exec_busy=1;

	pid=fork();
	if(pid == 0) {
		system(cmd);
		exit(0);
	} 
	
	return;
}

void ButtonPressEvent(XButtonEvent * xev)
{
	char buf[256];
	
	if (xev->type != ButtonPress) return;

	switch (xev->button) {
		case Button1:
		
			ifno++;
			break;
			
		case Button2:
		case Button3:

			if(ifinfo.state == 0) {
				sprintf(buf, exec_up, ifinfo.id);
			} else {
				sprintf(buf, exec_down,  ifinfo.id);
			}
			
			exec_cmd(buf);
			
			break;
	}
}


char *strupper(char *str)
{

	int i;

	for (i = 0; i < strlen(str); i++)
		str[i] = toupper(str[i]);

	return str;

}


int getifinfo(char *ifname, struct ifinfo_t *info)
{
	struct ifreq ifr;
	int r;
	struct sockaddr_in *sa;

#ifdef linux
	static FILE *froute = NULL;
	static FILE *fwireless = NULL;
	static FILE *fdev = NULL;
#elif defined(__OpenBSD__)
	struct ifreq ibuf[32];
	struct ifconf ifc;
	struct ifreq *ifrp, *ifend;
#endif
	
	char parent[16];
	char buf[1024];
	char *p;
	char a[16];
	int b,c,d;

#ifdef linux
	if(froute == NULL) froute = fopen("/proc/net/route", "r");
	if(fwireless == NULL) fwireless = fopen("/proc/net/wireless", "r");
	if(fdev == NULL) fdev = fopen("/proc/net/dev", "r");
#endif

	
	strcpy(parent, ifname);
	p=strchr(parent, ':');
	if(p) *p=0;

	strcpy(info->id, ifname);
	
	strcpy(ifr.ifr_name, ifname);	

	// Get status (UP/DOWN)

	if(ioctl(fd, SIOCGIFFLAGS, &ifr) != -1) {
		sa = (struct sockaddr_in *)&(ifr.ifr_addr);
		info->state = (ifr.ifr_flags & 1) ? 1 : 0;
	} else {
		info->state = 0;
	}
	
	// Get mac address
	
#ifdef linux
	if(ioctl(fd, SIOCGIFHWADDR, &ifr) != -1) {
		memcpy(info->hw, ifr.ifr_hwaddr.sa_data, 6);
	} else {
		memset(info->hw, 0, 6);
	}
#elif defined(__OpenBSD__)
	ifc.ifc_len = sizeof(ibuf);
	ifc.ifc_buf = (caddr_t) ibuf;
	if (ioctl(fd, SIOCGIFCONF, (char *) &ifc) == -1 ||
			ifc.ifc_len < sizeof(struct ifreq)) {
		memset(info->hw, 0, 6);
	} else {
		/* Search interface configuration list for link layer address. */
		ifrp = ibuf;
		ifend = (struct ifreq *) ((char *) ibuf + ifc.ifc_len);
		while (ifrp < ifend) {
			/* Look for interface */
			if (strcmp(ifname, ifrp->ifr_name) == 0 &&
					ifrp->ifr_addr.sa_family == AF_LINK &&
					((struct sockaddr_dl *) &ifrp->ifr_addr)->sdl_type == IFT_ETHER) {
				memcpy(info->hw, LLADDR((struct sockaddr_dl *) &ifrp->ifr_addr), 6);
				break;
			}
			/* Bump interface config pointer */
			r = ifrp->ifr_addr.sa_len + sizeof(ifrp->ifr_name);
			if (r < sizeof(*ifrp))
				r = sizeof(*ifrp);
			ifrp = (struct ifreq *) ((char *) ifrp + r);
		}
	}
#endif
		
	// Get IP address	
	
	if(ioctl(fd, SIOCGIFADDR, &ifr) != -1) {
		sa = (struct sockaddr_in *)&(ifr.ifr_addr);
		info->ip = sa->sin_addr.s_addr;
	} else {
		info->ip = 0;
	}
	
	// Get netmask
	
	if(ioctl(fd, SIOCGIFNETMASK, &ifr) != -1) {
		sa = (struct sockaddr_in *)&(ifr.ifr_addr);
		info->nm = sa->sin_addr.s_addr;
	} else {
		info->nm = 0;
	}
	
	// Get default gateway if on this interface
	
	info->gw = 0;	
#ifdef linux
	if(froute != NULL) {
		fseek(froute, 0, 0);
		
		while(fgets(buf, sizeof(buf), froute)) {
			r = sscanf(buf, "%s %x %x", a, &b, &c);
				
			if((strcmp(a, info->id) == 0) && (b == 0)) {
				info->gw = c;
			}
		}
		
	}
#elif defined(__OpenBSD__)
	{
	struct rt_msghdr *rtm = NULL;
	char *buf = NULL, *next, *lim = NULL;
	size_t needed;
	int mib[6];
	struct sockaddr *sa;
	struct sockaddr_in *sin;

	mib[0] = CTL_NET;
	mib[1] = PF_ROUTE;
	mib[2] = 0;
	mib[3] = AF_INET;
	mib[4] = NET_RT_DUMP;
	mib[5] = 0;
	if (sysctl(mib, 6, NULL, &needed, NULL, 0) == -1) {
		perror("route-sysctl-estimate");
		exit(1);
	}
	if (needed > 0) {
		if ((buf = malloc(needed)) == 0) {
			printf("out of space\n");
			exit(1);
		}
		if (sysctl(mib, 6, buf, &needed, NULL, 0) == -1) {
			perror("sysctl of routing table");
			exit(1);
		}
		lim  = buf + needed;
	}

	if (buf) {
		for (next = buf; next < lim; next += rtm->rtm_msglen) {
			rtm = (struct rt_msghdr *)next;
			sa = (struct sockaddr *)(rtm + 1);
			sin = (struct sockaddr_in *)sa;

			if (sin->sin_addr.s_addr == 0) {
				sa = (struct sockaddr *)(ROUNDUP(sa->sa_len) + (char *)sa);
				sin = (struct sockaddr_in *)sa;
				info->gw = sin->sin_addr.s_addr;
				break;
			}
		}
		free(buf);
	}
	}
#endif

	// Get wireless link status if wireless
	
	info->sl = 0;
#ifdef linux
	if(fwireless != NULL) {
		fseek(fwireless, 0, 0);
		
		while(fgets(buf, sizeof(buf), fwireless)) {
			r = sscanf(buf, "%s %d %d ", a, &b, &c);
			if(strchr(a, ':'))  *(strchr(a, ':')) = 0;
			if(strcmp(a, parent) == 0) {
				info->sl = c;
			}
		}
	}
	
#ifdef ENABLE_NWN_SUPPORT
	info->sl = nwn_get_link(parent);
#endif
#elif defined(__OpenBSD__)
	{
	struct wi_req	wreq;
	struct ifreq	ifr;

	wreq.wi_len = WI_MAX_DATALEN;
	wreq.wi_type = WI_RID_COMMS_QUALITY;

	strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	ifr.ifr_data = (caddr_t)&wreq;

	if (ioctl(fd, SIOCGWAVELAN, &ifr) != -1)
		info->sl = letoh16(wreq.wi_val[0]);
	}
#endif
	
	// Get Total tx/rx bytes	

#ifdef linux
	if(fdev != NULL) {
		fseek(fdev, 0, 0);
		
		while(fgets(buf, sizeof(buf), fdev)) {
			r = sscanf(buf, "%s %d %d %d %d %d %d %d %d %d", a, &b, &d,&d,&d,&d,&d,&d,&d, &c);
			if(strchr(a, ':'))  *(strchr(a, ':')) = 0;
			if(strcmp(a, parent) == 0) {
				info->bytes = b + c;
			}
		}
	}
#endif
	
	return(0);
}


void addifname(char *name)
{
	int i;

	if(strcmp(name, "lo") == 0) return;
	if(strncmp(name, "vlan", 4) == 0) return;
	
	for(i=0; i<ifaces; i++) {
		if(strcmp(ifname[i], name) == 0) return;
	}

	strcpy(ifname[ifaces], name);
	
	
	ifaces++;
	
	return;
}


/* 
 * get list of interfaces. First read /proc/net/dev, then do a SIOCGIFCONF
 */
 
void getifnames(void)
{	
	char pifname[MAXIFS][16];
	int pifaces;
	int i,j;
	int isnew;
	
	/* 
	 * Copy list of interface names and clean the old list
	 */
	 
	for(i=0; i<ifaces; i++) strncpy(pifname[i], ifname[i], sizeof(pifname[i]));
	pifaces = ifaces;
	ifaces = 0;

#ifdef linux
	FILE *f;
	char buf[128];
	char *p1, *p2;
	int ifcount;
	
	f = fopen("/proc/net/dev", "r");
	
	if(f == NULL) {
		fprintf(stderr, "Can't open /proc/net/dev\n");
		exit(1);
	}

	fgets(buf, sizeof(buf),f);	
	while(fgets(buf, sizeof(buf), f)) {
		p1=buf;
		while(*p1 == ' ') p1++;
		p2=p1;
		while(*p2 && (*p2 != ':')) p2++;
		if(*p2 == ':') {
			*p2=0;
			addifname(p1);
		}
	}
	
	fclose(f);

	ifc.ifc_len = sizeof(struct ifreq) * 10;
	
	if(ioctl(fd, SIOCGIFCONF, &ifc) == -1) {
		fprintf(stderr, "SIOCGIFCONF : Can't get list of interfaces : %s\n", strerror(errno));
		exit(1);
	}
	
	ifcount = ifc.ifc_len / sizeof(struct ifreq);
		
	for(i=0; i<ifcount; i++) {
		addifname(ifc.ifc_req[i].ifr_name);
	}
#endif
#ifdef __OpenBSD__
	struct ifreq ibuf[32];
	struct ifconf ifc;
	struct ifreq *ifrp, *ifend;
	int r;

	ifc.ifc_len = sizeof(ibuf);
	ifc.ifc_buf = (caddr_t) ibuf;
	if (ioctl(fd, SIOCGIFCONF, (char *) &ifc) == -1 ||
			ifc.ifc_len < sizeof(struct ifreq)) {
		fprintf(stderr, "SIOCGIFCONF : Can't get list of interfaces : %s\n", strerror(errno));
		exit(1);
	}
	/* Search interface configuration list for link layer address. */
	ifrp = ibuf;
	ifend = (struct ifreq *) ((char *) ibuf + ifc.ifc_len);
	while (ifrp < ifend) {
		if (ifrp->ifr_addr.sa_family == AF_LINK &&
		    ((struct sockaddr_dl *) &ifrp->ifr_addr)->sdl_type == IFT_ETHER) {
			addifname(ifrp->ifr_name);
		}
		/* Bump interface config pointer */
		r = ifrp->ifr_addr.sa_len + sizeof(ifrp->ifr_name);
		if (r < sizeof(*ifrp))
			r = sizeof(*ifrp);
		ifrp = (struct ifreq *) ((char *) ifrp + r);
	}
#endif

	/*
	 * Check if the new list contains interfaces that were not in the old list. If a new
	 * interface is found, make it the current one to display. (-i will override)
	 */
	
	for(i=0; i<ifaces; i++) {
		isnew = 1;
		for(j=0; j<pifaces; j++) if(strcmp(ifname[i], pifname[j]) == 0) isnew = 0;
		if(isnew) ifno = i;
	}

	for(i=0; i<ifaces; i++) {
		if(strcasecmp(ifname[i], startif) == 0) {
			printf("whop\n");
			ifno = ifaces;
			startif[0] = 0;
		}
	}
	 
}
