/* $Id: wl.c,v 1.18 2008/05/13 04:42:17 hacki Exp $ */

/*
 * Copyright (c) 2005, 2006 Marcus Glocker <marcus@nazgul.ch>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
 
/* Ported to FreeBSD by Nathan Lay <nslay@hotmail.com> 4/30/06 */

#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#ifdef __OpenBSD__
#include <dev/ic/if_wi_ieee.h>
#endif
#ifdef __FreeBSD__
#include <dev/wi/if_wavelan_ieee.h>
#endif
#include <err.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_media.h>
#include <net80211/ieee80211.h>
#include <net80211/ieee80211_ioctl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "wl.h"

/*
 * global variables for this file
 */
#ifdef __OpenBSD__
static char *speed[25] = {
	"DS1", "1",
	"DS2", "2",
	"DS5", "5.5",
	"DS11", "11",
	"OFDM6", "6",
	"OFDM9", "9",
	"OFDM12", "12s",
	"OFDM18", "18",
	"OFDM24", "24",
	"OFDM36", "36",
	"OFDM48", "48",
	"OFDM54", "54",
	NULL
};
#endif

#ifdef __FreeBSD__
static char *speed[] = {
	"FH/1Mbps", "1",
	"FH/2Mbps", "2",
	"DS/1Mbps", "1",
	"DS/2Mbps", "2",
	"DS/5.5Mbps", "5.5",
	"DS/11Mbps", "11",
	"DS/22Mbps", "22",
	"OFDM/6Mbps", "6",
	"OFDM/9Mbps", "9",
	"OFDM/12Mbps", "12",
	"OFDM/18Mbps", "18",
	"OFDM/24Mbps", "24",
	"OFDM/36Mbps", "36",
	"OFDM/48Mbps", "48",
	"OFDM/54Mbps", "54",
	"OFDM/72Mbps", "72",
	"DS/354Kbps", "0.354",
	"DS/512Kbps", "0.512",
	NULL
};
#endif

/*
 * get_wep()
 *	get wep status
 * Return:
 *	0 = wep disabled, 1 = wep enabled, -1 = error
 */
int
get_wep(const char *interface)
{
	int			r = 0, s, inwkey;
#ifdef __OpenBSD__
	struct ieee80211_nwkey	nwkey;
#endif
#ifdef __FreeBSD__
	struct ieee80211req	nwkey;
#endif

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		return (-1);

	memset(&nwkey, 0, sizeof(nwkey));
#ifdef __FreeBSD__
	nwkey.i_type = IEEE80211_IOC_WEP;
#endif
	strlcpy(nwkey.i_name, interface, sizeof(nwkey.i_name));
#ifdef __OpenBSD__
	if ((inwkey = ioctl(s, SIOCG80211NWKEY, (caddr_t)&nwkey)) == -1) {
		close(s);
		return (-1);
	}
#endif
#ifdef __FreeBSD__
	if ((inwkey = ioctl(s, SIOCG80211, (caddr_t)&nwkey)) == -1) {
		close(s);
		return (-1);
	}
#endif
	close(s);

#ifdef __OpenBSD__
	if (inwkey == 0 && nwkey.i_wepon > 0)
		r = 1;
#endif
#ifdef __FreeBSD__
	if (inwkey == 0 && nwkey.i_val > 0)
		r = 1;
#endif

	return (r);
}

/*
 * get_channel()
 *	get channel number
 * Return:
 *	<channel number> = success, 0 = no data, -1 = error
 */
int
get_channel(const char *interface)
{
	int			s, ichan;
#ifdef __OpenBSD__
	struct ieee80211chanreq	channel;
#endif
#ifdef __FreeBSD__
	struct ieee80211req	channel;
#endif

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		return (-1);

	memset(&channel, 0, sizeof(channel));
#ifdef __FreeBSD__
	channel.i_type = IEEE80211_IOC_CHANNEL;
#endif
	strlcpy(channel.i_name, interface, sizeof(channel.i_name));
#ifdef __OpenBSD__
	if ((ichan = ioctl(s, SIOCG80211CHANNEL, (caddr_t)&channel)) == -1) {
		close(s);
		return (-1);
	}
#endif
#ifdef __FreeBSD__
	if ((ichan = ioctl(s, SIOCG80211, (caddr_t)&channel)) == -1) {
		close(s);
		return (-1);
	}
#endif
	close(s);
	
	if (ichan == 0) {
#ifdef __OpenBSD__
		if (channel.i_channel < 1000)
			return (channel.i_channel);
#endif
#ifdef __FreeBSD__
		if (channel.i_val < 1000)
			return (channel.i_val);
#endif
	}

	return (0);
}

/*
 * get_signal()
 *	get signal strength
 * Return:
 *	<signal strength> = success, 0 = no data, -1 = error
 */
int
get_signal(const char *interface, const char *network)
{
	
#ifdef __OpenBSD__
	int				i = 0, s, len;
	struct ieee80211_nodereq_all	na;
	struct ieee80211_nodereq	nr[8];
#endif
#ifdef __FreeBSD__
	int				s, len;
	uint8_t				buf[24 * 1024], *cp;
	struct ieee80211req		na;
#endif
	char				network_id[IEEE80211_NWID_LEN + 1];

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		return (-1);

	memset(&na, 0, sizeof(na));
#ifdef __OpenBSD__
	memset(&nr, 0, sizeof(nr));
	na.na_node = nr;
	na.na_size = sizeof(nr);
	strlcpy(na.na_ifname, interface, sizeof(na.na_ifname));
#endif
#ifdef __FreeBSD__
	strlcpy(na.i_name, interface, sizeof(na.i_name));
	na.i_type = IEEE80211_IOC_SCAN_RESULTS;
	na.i_data = buf;
	na.i_len = sizeof(buf);
#endif
#ifdef __OpenBSD__
	if (ioctl(s, SIOCG80211ALLNODES, &na) == -1) {
		close(s);
		return (-1);
	}
#endif
#ifdef __FreeBSD__
	if (ioctl(s, SIOCG80211, (caddr_t)&na) == -1) {
		close(s);
		return (-1);
	}
#endif
	close(s);

#ifdef __OpenBSD__
	for (i = 0; i < na.na_nodes; i++) {
		if (nr[i].nr_nwid_len < sizeof(network_id))
			len = nr[i].nr_nwid_len + 1;
		else
			len = sizeof(network_id);
		strlcpy(network_id, (const char *)nr[i].nr_nwid, len);
		if (!strcmp(network_id, network))
			return (nr[i].nr_rssi);
	}
#endif
#ifdef __FreeBSD__
	/* This is how ifconfig does it */
	len = na.i_len;
	cp = buf;
	do {
		struct ieee80211req_scan_result *sr;
		uint8_t *vp;
		sr = (struct ieee80211req_scan_result *)cp;
		vp = (u_int8_t *)(sr + 1);
		strlcpy(network_id, (const char *)vp, sr->isr_ssid_len + 1);
		if (!strcmp(network_id, network))
			return (sr->isr_rssi);
		cp += sr->isr_len;
		len -= sr->isr_len;
		if (sr->isr_len <= 0) /* Some weird lockup */
			break;
	} while (len >= sizeof(struct ieee80211req_scan_result));
#endif
	
	return (0);
}

/*
 * get_wi_signal()
 *	get signal strength for wi interfaces
 * Return:
 *	<signal strength> = success, -1 = error
 */
int
get_wi_signal(const char *interface)
{
	int		s;
	struct ifreq	ifr;
	struct wi_req	wreq;
#ifdef __OpenBSD__
	float		link;
#endif

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		return (-1);

	bzero((char *)&wreq, sizeof(wreq));
	bzero((char *)&ifr, sizeof(ifr));

	wreq.wi_len = WI_MAX_DATALEN;
	wreq.wi_type = WI_RID_COMMS_QUALITY;
	ifr.ifr_data = (caddr_t)&wreq;
	strlcpy(ifr.ifr_name, interface, sizeof(ifr.ifr_name));

	if (ioctl(s, SIOCGWAVELAN, &ifr) == -1) {
		close(s);
		return (-1);
	}
	close(s);

#ifdef __OpenBSD__
	link = wreq.wi_val[0];
	/*
	 * for future reference ...
	 * level = wreq.wi_val[1];
	 * noise = wreq.wi_val[2];
	 */
	return (letoh16(link));
#endif
#ifdef __FreeBSD__
	return (wreq.wi_val[1]);
#endif
}

/*
 * get_speed()
 *	get media speed
 * Return:
 *	<pointer to speed> = success, NULL = no data / error
 */
char *
get_speed(const char *interface)
{
	int					s, mword;
	struct ifmediareq			ifmr;
	const struct ifmedia_description	*desc;
#ifdef __OpenBSD__
	const struct ifmedia_description	ifm_subtype_descriptions[] =
	    IFM_SUBTYPE_DESCRIPTIONS;
#endif
#ifdef __FreeBSD__
	const struct ifmedia_description	ifm_subtype_descriptions[] =
	    IFM_SUBTYPE_IEEE80211_DESCRIPTIONS;
#endif

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		return (NULL);

	memset(&ifmr, 0, sizeof(ifmr));
	strlcpy(ifmr.ifm_name, interface, sizeof(ifmr.ifm_name));
	if (ioctl(s, SIOCGIFMEDIA, (caddr_t)&ifmr) == -1) {
		close(s);
		return (NULL);
	}
	close(s);

	mword = ifmr.ifm_active;
	for (desc = ifm_subtype_descriptions; desc->ifmt_string != NULL;
	    desc++) {
		if (IFM_TYPE_MATCH(desc->ifmt_word, mword) &&
		    IFM_SUBTYPE(desc->ifmt_word) == IFM_SUBTYPE(mword))
			return (translate_speed(desc->ifmt_string));
	}
	
	return (NULL);
}

/*
 * get_status()
 *	get network status
 * Return:
 *	<pointer to status> = success, NULL = no data / error
 */
char *
get_status(const char *interface)
{
#ifdef __OpenBSD__
	int					s, bitno;
	static char				status[64];
	const struct ifmedia_status_description	*ifms;
	const struct ifmedia_status_description	ifm_status_descriptions[] =
	    IFM_STATUS_DESCRIPTIONS;
	const int				ifm_status_valid_list[] =
	    IFM_STATUS_VALID_LIST;
#endif
#ifdef __FreeBSD__
	int					s;
#endif
	struct ifmediareq			ifmr;

	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		return (NULL);

	memset(&ifmr, 0, sizeof(ifmr));
	strlcpy(ifmr.ifm_name, interface, sizeof(ifmr.ifm_name));
	if (ioctl(s, SIOCGIFMEDIA, (caddr_t)&ifmr) == -1) {
		close(s);
		return (NULL);
	}
	close(s);

#ifdef __OpenBSD__
	for (bitno = 0; ifm_status_valid_list[bitno] != 0; bitno++) {
		for (ifms = ifm_status_descriptions; ifms->ifms_valid != 0;
		    ifms++) {
			if (ifms->ifms_type != IFM_TYPE(ifmr.ifm_current) ||
			    ifms->ifms_valid != ifm_status_valid_list[bitno])
				continue;
			strlcpy(status, IFM_STATUS_DESC(ifms, ifmr.ifm_status),
				sizeof(status));
			return (status);
		}
		
	}

	return (NULL);
#endif
#ifdef __FreeBSD__
	if (ifmr.ifm_status & IFM_ACTIVE)
		return ("active");
	return ("no carrier");
#endif
}

/*
 * get_nwid()
 *	get wireless network id
 * Return:
 * 	pointer to network id = success, NULL = no data / error
 */
char *
get_nwid(const char *interface)
{
	int			s, len, inwid;
	static char		network_id[IEEE80211_NWID_LEN + 1];
#ifdef __OpenBSD__
	struct ifreq		ifr;
	struct ieee80211_nwid	nwid;
#endif
#ifdef __FreeBSD__
	struct ieee80211req	nwid;
#endif
	
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		return (NULL);

#ifdef __OpenBSD__
	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_data = (caddr_t)&nwid;
	strlcpy(ifr.ifr_name, interface, sizeof(ifr.ifr_name));
#endif
#ifdef __FreeBSD__
	memset(&nwid, 0, sizeof(nwid));
	strlcpy(nwid.i_name, interface, sizeof(nwid.i_name));
	nwid.i_type = IEEE80211_IOC_SSID;
	nwid.i_data = network_id;
	nwid.i_len = sizeof(network_id);
#endif
#ifdef __OpenBSD__
	if ((inwid = ioctl(s, SIOCG80211NWID, (caddr_t)&ifr)) == -1) {
		close(s);
		return (NULL);
	}
#endif
#ifdef __FreeBSD__
	if ((inwid = ioctl(s, SIOCG80211, (caddr_t)&nwid)) == -1) {
		close(s);
		return (NULL);
	}
#endif
	close(s);

	if (inwid == 0) {
		if (nwid.i_len < sizeof(network_id))
			len = nwid.i_len + 1;
		else
			len = sizeof(network_id);
#ifdef __OpenBSD__
		strlcpy(network_id, (const char *)nwid.i_nwid, len);
#endif
#ifdef __FreeBSD__
		network_id[len - 1] = '\0';
#endif

		return (network_id);
	}

	return (NULL);
}

/*
 * get_first_wnic()
 *	scans interfaces and returns the first found wireless interface
 * Return:
 *	<pointer to wnic> = success, NULL = no nic found
 */
char *
get_first_wnic(void)
{
	char		*r = NULL;
	char		nic[IF_NAMESIZE];
	struct ifaddrs	*ifap = NULL, *ifa = NULL;

	memset(nic, 0, sizeof(nic));

	if (getifaddrs(&ifap) != 0)
		errx(1, "getifaddrs");

	for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
		if (strcmp(nic, ifa->ifa_name)) {
			if (get_wep(ifa->ifa_name) != -1) {
				r = strdup(ifa->ifa_name);
				break;
			}
		}
		strlcpy(nic, ifa->ifa_name, sizeof(nic));
	}

	freeifaddrs(ifap);

	return (r);
}

/*
 * check_nic()
 *	check if the monitored interface still exists
 * Return:
 *	0 = interface gone, 1 = interface exists
 */
int
check_nic(const char *interface)
{
	int		r = 0;
	char		nic[IF_NAMESIZE];
	struct ifaddrs	*ifap = NULL, *ifa = NULL;

	memset(nic, 0, sizeof(nic));

	if (getifaddrs(&ifap) != 0)
		errx(1, "getifaddrs");

	for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
		if (strcmp(nic, ifa->ifa_name)) {
			if (!strcmp(ifa->ifa_name, interface)) {
				r = 1;
				break;
			}
		}
		strlcpy(nic, ifa->ifa_name, sizeof(nic));
	}

	freeifaddrs(ifap);

	return (r);
}

/*
 * translate_speed()
 *	translate the result of media speed to human readable string
 * Return:
 *	<pointer to speed> = success, NULL = no data / error 
 */
char *
translate_speed(const char *mode)
{
	int	i;

	for (i = 0; speed[i] != NULL; i++) {
		if (!strcmp(mode, speed[i]))
			return (speed[i + 1]);
	}

	return (NULL);
}
