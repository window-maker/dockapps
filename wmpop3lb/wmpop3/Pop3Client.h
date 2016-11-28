/* Author : Louis-Benoit JOURDAIN (lb@jourdain.org)
 * based on the original work by Scott Holden ( scotth@thezone.net )
 * 
 * multi Pop3 Email checker.
 *
 * Last Updated : Feb 7th, 2002
 *
 */


#ifndef POP3CLIENT
#define POP3CLIENT

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/poll.h>

#define	MAIL_BUFLEN	64
#define	CONF_BUFLEN	256

#define	RSET_COLOR	color = 0
#define	SWAP_COLOR	color = 1 - color
#define	CH_COLOR(a)	(color) ? a + 24 : a	

#define WMPOP3_VERSION "2.4.2"

#define CHAR_WIDTH 5
#define CHAR_HEIGHT 5
#define	TOP_MARGIN 5
#define	LEFT_MARGIN 4
#define	NUMBERS	93
#define	LETTERS	85
#define SMALL_NUM 77
#define	SN_CHAR_W 3
#define	NB_DISP	8
#define	EXTRA	1
#define	NB_LINE	7
#define SEC_IN_MIN 60
#define YES 1
#define NO 0
#define SELECTONLY 1
#define NOSELECTONLY 0

/* get mail progress bar */
#define	PROGRESSBAR_LEN	48
#define	PROGRESSBAR_HEI	6
#define PROGRESSBAR_HPOS	10
#define	PROGRESSBAR_VPOS	((6*6) + TOP_MARGIN)

#define	ORIG_PB_TX	72
#define	ORIG_PB_TY	65

#define	ORIG_PB_BARX	73	
#define	ORIG_PB_BARY	71

#define SCROLL_LX	4	/* left X coordinate */
#define SCROLL_RX	6	/* right X coordinate */
#define SCROLL_TY	4	/* top Y coordinate */
#define SCROLL_BY	45	/* botton Y coordinate */
#define SCROLL_W	3
#define SCROLL_H	38	/* this is without SCROLL_EXT at each end */
#define	SCROLL_EXT	2	

#define	TMPPREFIX	"Pop3tmp"
#define TMPPREFIXLEN	7
#define	TXT_MESSAGETOOBIG	"From: %s\n\
Subject: %s\n\n\
\
The size of this email (%d) is above the configured maxdlsize value (%d) and\
 was not downloaded\n"

#define	TXT_ERROR		"From: %s\n\
Subject: %s\n\n\
\
There was an error (%s) while getting this email from the server\n"

#define TXT_SEPARATOR "--------------------------------------------------------------------\n"

typedef struct s_scrollbar
{
  int		top;
  int		bottom;
  int		orig_y;
  int		orig_index_vert;
  int		allowedspace;
}		t_scrollbar;

typedef struct	s_mail
{
  char		from[MAIL_BUFLEN];
  char		subject[MAIL_BUFLEN];
  char		todelete;
  long		cksum;	/* checksum of the header */
  int		new;	/* mail is new */
}		t_mail;

struct pop3_struct{
  struct sockaddr_in	server;
  struct hostent	*hp;
  enum   {CONNECTED, NOT_CONNECTED} connected;
  char			**mailclient;		/* argv[] type of command */
  char			**newmailcommand;	/* argv[] type of command */
  char			**selectedmesgcommand;	/* argv[] type of command */
  char			username[256];
  char			popserver[128];
  char			password[256];
  char			mailseparator[256];
  char			inBuf[4096];
  char			outBuf[1024];
  char			alias[4];
  char			delstatus[9];
  int			s;			/* socket */
  int			serverport;
  int			localPort;
  int			numOfMessages;
  int			numOfUnreadMessages;
  int			countunreadonly;
  int			sizeOfAllMessages;
  int			sizechanged;	       
  int			mailCheckDelay;
  int			forcedCheck;
  int			status;
  int			maxdlsize;
  long			nextCheckTime;
  t_mail		*mails;
};
typedef struct pop3_struct *Pop3;


Pop3 pop3Create(int nb_conf);
int  pop3MakeConnection( Pop3 pc, char *serverName, int port);
int  pop3IsConnected(Pop3 pc);
int  pop3Login(Pop3 pc, char *name, char *pass);
int  pop3Quit(Pop3 pc);
int  pop3CheckMail(Pop3 pc);
int  pop3GetTotalNumberOfMessages( Pop3 pc );
int  pop3GetNumberOfUnreadMessages( Pop3 pc );
int  pop3WriteOneMail(int nb, int dest_fd, Pop3 pc);
#endif
