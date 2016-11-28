/*
 *  wmpop3.c
 *  multi pop3 mail checker. 
 *  written by Louis-Benoit JOURDAIN (lb@jourdain.org)
 *  based on the original work by Scott Holden (scotth@thezone.net)
 *
*/


#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

#include <X11/xpm.h>
#include <X11/Xlib.h>
#include <X11/extensions/shape.h>

#include <dirent.h>

#include "../wmgeneral/misc.h"
#include "../wmgeneral/wmgeneral.h"
#include "Pop3Client.h"

#include "wmpop3.xpm"

char wminet_mask_bits[64*64];
int  wminet_mask_width = 64;
int  wminet_mask_height = 64;

char *ProgName;

char mailclient[32]   = "pine";
char password[32];
char username[32];
char popserver[128];
int  serverport       = 110;
int  mailCheckDelay   = 10;  /* default */
int  autoChecking     = YES; /* default */
int  newMessagesOnly  = YES; /* default */
int  displaydelay = 1;
char	tempdir[1024];
char config_file[256] = "not-defined";
int	scrollspeed = 100;

Pop3	conf[6];
int	nb_conf;	/* number of configured pop servers */
int	summess;	/* total number of messages */
int	color;

#ifdef _DEBUG
int	haspassed = 0;
#endif

t_scrollbar	scrollbar;

void usage(void);
void printversion(void);
void wmCheckMail_routine(int, char **);
int  readConfigFile( char *filename );

void BlitString(char *name, int x, int y, int fragment);
void BlitNum(int num, int x, int y, int todelete);

int	pop3DeleteMail(int num, Pop3 pc);
int	pop3VerifStats(Pop3 pc);

/********************************
 *           Main Program
 ********************************/
void	deleteoldtmpfiles() 
{
  char	buf[1024];
  DIR	*dir;
  struct dirent	*dp;

  if (tempdir) {
    if ((dir = opendir(tempdir))) {
      chdir(tempdir);
      while((dp = readdir(dir))) {
	if (!strncmp(dp->d_name, TMPPREFIX, TMPPREFIXLEN)) {
	  sprintf(buf, "%s/%s", tempdir, dp->d_name);
#ifdef _DEBUG
	  printf("  delete old tmp file: %s\n", buf);
#endif
	  unlink(buf);
	}
	}
      closedir(dir);
    }
  }
}

int main(int argc, char *argv[]) {

	int		i;

	ProgName = argv[0];
	if (strlen(ProgName) >= 5)
		ProgName += (strlen(ProgName) - 5);
	
	for (i=1; i<argc; i++) {
		char *arg = argv[i];

		if (*arg=='-') {
			switch (arg[1]) {
			case 'd' :
				if (strcmp(arg+1, "display")) {
					usage();
					exit(1);
				}
				break;
			case 'g' :
				if (strcmp(arg+1, "geometry")) {
					usage();
					exit(1);
				}
				break;
			case 'v' :
				printversion();
				exit(0);
				break;
            case 'c' :
                if (argc > (i+1))
                {
                    strcpy(config_file, argv[i+1]);
                    i++;
                }
                break;
            default:
				usage();
				exit(0);
				break;
			}
		}
	}
	wmCheckMail_routine(argc, argv);

	return 0;
}

void		build_line(char *buf, int num, int index, Pop3 pc) 
{
  int		len;
  int		pos;
  char		tmp[256];
  
  /* display spaces in case alias is less than 3 char long */
  memset(tmp, ' ', sizeof(char) * 3);
  memset(tmp + 3, 0, sizeof(char) * (256 - 3));
  memcpy(tmp, pc->alias, strlen(pc->alias));
  tmp[3] = ':';
  pos = 4;
  for (len = 0; len < MAIL_BUFLEN && pc->mails[num].from[len]; len++);
  memcpy(tmp + pos, pc->mails[num].from, len);
  tmp[pos + len] = '/';
  pos += len + 1;
  for (len = 0; len < MAIL_BUFLEN && pc->mails[num].subject[len]; len++);
  memcpy(tmp + pos, pc->mails[num].subject, len);
  len += pos;
  tmp[len++] = ' ';
  tmp[len++] = '*';
  tmp[len++] = ' ';
  tmp[len] = '\0';
  pos = index % len;
  if (len - pos < (NB_DISP + EXTRA)) { /* We are at the end of the string */
    memcpy(buf, tmp + pos, len - pos);
    memcpy(buf + len - pos, tmp, (NB_DISP + EXTRA) - len + pos);
  }
  else {
    memcpy(buf, tmp + pos, NB_DISP + EXTRA);
  }
}

void		display_index(int mail_index, int line, Pop3 pc)
{
#ifdef _DEBUG
  if (haspassed) {
    printf("  mail_index: %d, line: %d, todelete: %d\n",
	   mail_index, line, pc->mails[mail_index].todelete);
  }
#endif
  if (pc->mails[mail_index].todelete) {
    copyXPMArea(67, 12, 2, 5, 7, (line * 6) + TOP_MARGIN);
  }
  else {
    copyXPMArea(69, 11, 2, 6, 7, (line * 6) + TOP_MARGIN - 1);
  }
}

void		ClearDisplay() {
  int		i;

  copyXPMArea(72, 4, 3, 42, LEFT_MARGIN, 4);
  copyXPMArea(72, 4, 50, 42, 9, 4 );
  for (i = 0; i < NB_LINE; i++) {
    copyXPMArea(69, 11, 2, 6, 7, (i * 6) + TOP_MARGIN - 1);
  }
}

/* return < 0 if error
   0 if no pb
   else nb of mails which couldn't be deleted */
int	DeleteMail(Pop3 pc)
{
  int	etat = 0;
  int	old_sizeOfAllMessages;
  int	i;
  
  old_sizeOfAllMessages = pc->sizeOfAllMessages;
  BlitString(pc->alias, 10, TOP_MARGIN, 0);
  BlitString("connect", 10, 6*2 + TOP_MARGIN, 0);
  RedrawWindow();
  etat = pop3MakeConnection(pc, pc->popserver, pc->serverport);
  if (-1 != etat) {
    BlitString("login", 10, (6*3) + TOP_MARGIN, 0);
    RedrawWindow();
    etat = pop3Login(pc, pc->username, pc->password);
  }
  if (-1 != etat) {
    BlitString("chk cont", 10, (6*4) + TOP_MARGIN, 0);
    RedrawWindow();
    etat = pop3VerifStats(pc);
  }
  if (-1 != etat) {
    if (etat != old_sizeOfAllMessages) {
      BlitString("mailbox", 10, TOP_MARGIN, 0);
      BlitString("content", 10, 6 + TOP_MARGIN, 0);
      BlitString("changed", 10, 6*2 + TOP_MARGIN, 0);
      BlitString("between", 10, 6*3 + TOP_MARGIN, 0);
      BlitString("updates", 10, 6*4 + TOP_MARGIN, 0);
      BlitString("del can.", 10, 6*6 + TOP_MARGIN, 0);
      RedrawWindow();
      sleep(displaydelay);
      for (i = 0, etat = 0; i < pc->numOfMessages; i++) {
	etat += pc->mails[i].todelete;
      }
    }
    else {
      etat = 0;
      for (i = 0; i < pc->numOfMessages; i++) {
	if (pc->mails[i].todelete) {
	  etat += pop3DeleteMail(i + 1, pc);
	}
      }
    }
  }
  pop3Quit(pc);
  return (etat);
}  

/*
void		launch_mail_client(int iS)
{
  int		i;
  int		j;
  char		str[16];
  char		**args;

  ClearDisplay();
  if ((args = conf[iS]->mailclient)) {
    BlitString("starting", 10, TOP_MARGIN, 0);
    for (i = 0; i < 5 && args[i]; i++) {
      for (j = 0; j < 8 && args[i][j]; j++);
      memcpy(str, args[i], j);
      str[j] = '\0';
      BlitString(str, 10, (i + 2)*6 + TOP_MARGIN, 0);
    }
    if (0 == fork()) {
      if (execvp(args[0], args)) {
	perror("execvp");
	copyXPMArea(72, 4, 3, 42, LEFT_MARGIN, 4);
	copyXPMArea(72, 4, 50, 6, 9, 4 );
	copyXPMArea(69, 11, 2, 6, 7, TOP_MARGIN - 1);
	BlitString("Error", 10, TOP_MARGIN, 0);
      }
    }
    else {
      conf[iS]->nextCheckTime = time(0) + 30;
    }
  }
  else {
    BlitString("  mail", 10, TOP_MARGIN, 0);
    BlitString(" client", 10, 6 + TOP_MARGIN, 0);
    BlitString("not  set", 10, 6*2 + TOP_MARGIN, 0);
    BlitString(" check", 15, 6*5 + TOP_MARGIN, 0);
    BlitString("wmpop3rc", 10, 6*6 + TOP_MARGIN, 0);
  }
  RedrawWindow();
  sleep(displaydelay);
}
*/

void		display_scrollbar(int index, int totalmessages, 
				  int maxlines, int scrollmode)
{
  int		delta;
  int		scrollen;
  int		vertpos;
  
  if (totalmessages <= maxlines) {
#ifdef _DEBUG
    if (haspassed)
      printf("  full scrollbar\n");
#endif
    copyXPMArea((scrollmode) ? 68 : 65, 18, SCROLL_W, 
		SCROLL_H + (2 * SCROLL_EXT), SCROLL_LX, SCROLL_TY);
    scrollbar.top = SCROLL_TY;
    scrollbar.bottom = SCROLL_TY + SCROLL_H;
    scrollbar.allowedspace = SCROLL_H;
  }
  else {
    delta = totalmessages - maxlines;
    /* determine the size of the scrollbar */
    if (0 >= (scrollen = SCROLL_H - ((SCROLL_H / maxlines) * delta)))
      scrollen = 1;
    /* determine the position */
    scrollbar.allowedspace = SCROLL_H - scrollen;
    vertpos = scrollbar.allowedspace * index / delta;
    copyXPMArea((scrollmode) ? 68 : 65, 18, SCROLL_W, SCROLL_EXT,
		SCROLL_LX, vertpos + SCROLL_TY);
    copyXPMArea((scrollmode) ? 68 : 65, 18 + SCROLL_EXT, SCROLL_W, scrollen,
		SCROLL_LX, vertpos + SCROLL_TY + SCROLL_EXT);
    copyXPMArea((scrollmode) ? 68 : 65, 18 + SCROLL_EXT + SCROLL_H, 
		SCROLL_W, SCROLL_EXT,
		SCROLL_LX, vertpos + SCROLL_TY + SCROLL_EXT + scrollen);
    scrollbar.top = vertpos + SCROLL_TY;
    scrollbar.bottom = vertpos + SCROLL_TY + (SCROLL_EXT * 2) + scrollen;

#ifdef _DEBUG
    if (haspassed) {
      printf("  index: %d, totalmess:%d, mxli: %d, delta: %d, vertpos: %d, allowedspace: %d, sl:%d\n",
	     index, totalmessages, maxlines, delta, vertpos, 
	     scrollbar.allowedspace, scrollen);
    }
#endif
  }
  /*  RedrawWindow(); */
}

int		is_for_each_mail(char **command) 
{
  int		i;
  
  if (command) {
    for (i = 0; command[i]; i++) {
      if ('%' == command[i][0]) {
	if (('f' == command[i][1]) ||
	    ('s' == command[i][1]) ||
	    ('m' == command[i][1]) ||
	    ('n' == command[i][1]))
	  return (1);
      }
    }
  }
  return (0);
}

char		*quotify(char *str) 
{
  char		*retour;
  int		len;

  len = strlen(str);
  if (NULL != (retour = (char *) malloc(sizeof(char) * len + 3))) {
    retour[0] = '\'';
    memcpy(retour + 1, str, len);
    retour[len + 1] = '\'';
    retour[len + 2] = '\0';
  }
  return (retour);
}

/* nb: number of the mail on the server - 1 (if -1: for all mails)
   path: allocated buffer to store path of tmp file
   newonly: only for new messages
   onlyselected: only for selected messages
   pc: main struct.
   on success return 0 */
int		writemailstofile(int nb, char *path, int newonly, 
				 int onlyselected, Pop3 pc) 
{
  int		fd;
  int		len;
  int		i;
  int		retour = 0;
  int		goforwritemail = 0;
  int		mustdisconnect = 0;
  
  len = strlen(tempdir);
  if (len) {
    path[0] = '\0';
    strcat(path, tempdir);
    if ('/' != tempdir[len - 1])
      strcat(path, "/");
    strcat(path, TMPPREFIX);
    strcat(path, "XXXXXX");
#ifdef _DEBUG
    printf("  creating temporary file [%s]\n", path);
    printf("  nb=%d, newonly=%d, onlyselected=%d\n",
	   nb, newonly, onlyselected);
#endif
    if (-1 == (fd = mkstemp(path))) {
      fprintf(stderr, "Error: writing mail to file\n");
      perror(path);
      retour++;
    }
    else {
      /* to prevent connecting many times, do it now. */
      if (NOT_CONNECTED == pc->connected) {
#ifdef _DEBUG
	printf("  writemailstofile not connected, connecting\n");
#endif
	if (pop3MakeConnection(pc,pc->popserver,pc->serverport) == -1){
	  return (1);
	}
	if (pop3Login(pc, pc->username, pc->password) == -1) {
	  return (1);
	}
	mustdisconnect = 1;
      }

      if (nb < 0) {
	/* concatenate all the mails into 1 file */
	for (i = 0; i < pc->numOfMessages; i++) {
	  goforwritemail = 0;
#ifdef _DEBUG
	  printf("  mail %d:", i);
#endif
	  if (!newonly && !onlyselected) {
#ifdef _DEBUG
	    printf("  !newonly && !onlyselected\n");
#endif	   
	    goforwritemail = 1;
	  }
	  else if (newonly) {
	    if (pc->mails[i].new && 
		(!onlyselected || (onlyselected && pc->mails[i].todelete))) {
#ifdef _DEBUG
	      printf("  newonly\n");
#endif	   
	      goforwritemail = 1;
	    }
	  }
	  else if (onlyselected && pc->mails[i].todelete) {
#ifdef _DEBUG
	    printf("  onlyselected\n");
#endif	   
	    goforwritemail = 1;
	  }
	  if (goforwritemail) {
	    /* put the mail separator */
	    if (strlen(pc->mailseparator)) {
#ifdef _DEBUG
	      printf("  pc->mailseparator: [%s]\n", pc->mailseparator);
#endif
	      write(fd, pc->mailseparator, strlen(pc->mailseparator));
	    }
	    else {
#ifdef _DEBUG
	      printf("  TXT_SEPARATOR: [%s]\n", TXT_SEPARATOR);
#endif      
	      write(fd, TXT_SEPARATOR, strlen(TXT_SEPARATOR));
	    }
	    if (pop3WriteOneMail(i + 1, fd, pc)) {
	      retour++;
	    }
	  }
	}

      }
      else {
	if (pop3WriteOneMail(nb + 1, fd, pc))
	  retour++;
      }
      close(fd);
      /* close the connection if we've opened it */
      if (mustdisconnect) {
#ifdef _DEBUG
	printf("  writemailstofile disconnecting\n");
#endif
	pop3Quit(pc);
      }
    }
  }
  else {
    fprintf(stderr, "no tempdir configured, can't create file\n");
    retour++;
  }
  return (retour);
}

/* 
 * do_command_completion
 * returned array should be freed by calling function
 * if nbnewmail == 0, for %c option concatenate _all_ the messages 
 */
char		**do_command_completion(int num, char **command, 
					int nbnewmail, Pop3 pc) 
{
  char		**retour;
  int		i;
  int		yaerreur = 0;
  char		path[1024];
  
  for (i = 0; command[i]; i++);
#ifdef _DEBUG
  printf("  %d args to the command, mail num %d\n", i, num);
#endif
  if (NULL == (retour = (char **) malloc (sizeof(char *) * i + 1))) {
    return (NULL);
  }
  memset(retour, 0, sizeof(char *) * i + 1);
  for (i = 0; command[i]; i++) {
#ifdef _DEBUG
    printf("  arg %d: [%s]\n", i, command[i]);
#endif
    if ('%' == command[i][0]) {
      switch (command[i][1]) {
      case 'T': /* total nb of mails */
	if (NULL != (retour[i] = (char *) malloc(sizeof(char) * 8)))
	  sprintf(retour[i], "%d", pc->numOfMessages);
	else 
	  yaerreur = 1;
	break;
      case 't': /* total nb of new mails */
	if (NULL != (retour[i] = (char *) malloc(sizeof(char) * 8)))
	  sprintf(retour[i], "%d", (nbnewmail < 0) ? 0 : 1);
	else 
	  yaerreur = 1;
	break;
      case 'C': /* concatenate all the selected mails in 1 file */
	if (writemailstofile(-1, path, 0, SELECTONLY, pc))
	  yaerreur = 1;
	else
	  if (NULL == (retour[i] = strdup(path))) {
	    yaerreur = 1;
	  }
	break;
      case 'c': /* concatenate all the mails in 1 file */
	if (writemailstofile(-1, path, (nbnewmail > 0), NOSELECTONLY, pc)) 
	  yaerreur = 1;
	else 
	  if (NULL == (retour[i] = strdup(path))) {
	    yaerreur = 1;
	  }
	break;
      case 'f': /* from field */
#ifdef _DEBUG
	printf("  from field: [%s]\n", pc->mails[num].from);
#endif
	if (NULL == (retour[i] = quotify(pc->mails[num].from)))
	  yaerreur = 1;
	break;
      case 's': /* subject of the mail */
#ifdef _DEBUG
	printf("  subject field: [%s]\n", pc->mails[num].subject);
#endif	
	if (NULL == (retour[i] = quotify(pc->mails[num].subject)))
	  yaerreur = 1;
	break;
      case 'm': /* copy the mail to tmp file */
	if (0 <= num) {
	  if (writemailstofile(num, path, (nbnewmail > 0), NOSELECTONLY, pc)) 
	    yaerreur = 1;
	  else 
	    if (NULL == (retour[i] = strdup(path))) {
	      yaerreur = 1;
	  }
	}
	break;
      case 'n': /* number of the message on the mail server */
	if (NULL != (retour[i] = (char *) malloc(sizeof(char) * 8)))
	  sprintf(retour[i], "%d", num + 1);
	else
	  yaerreur = 1;
	break;
      case 'S': /* server name */
	if (NULL == (retour[i] = strdup(pc->popserver)))
	  yaerreur = 1;
	break;
      case 'a': /* server alias */
	if (NULL == (retour[i] = strdup(pc->alias)))
	  yaerreur = 1;
	break;
      case 'u': /* user name */
	if (NULL == (retour[i] = strdup(pc->username)))
	  yaerreur = 1;
	break;
      case 'w': /* user password */
	if (NULL == (retour[i] = strdup(pc->password)))
	  yaerreur = 1;
	break;
      case 'P': /* server port */
	if (NULL != (retour[i] = (char *) malloc(sizeof(char) * 8)))
	  sprintf(retour[i], "%d", pc->serverport);
	else
	  yaerreur = 1;
	break;
      case '%': /* % character, leave in place */
	 if (NULL == (retour[i] = strdup(command[i]))) 
	   yaerreur = 1;
	break;
      default: 
	break;
      }
    }
    else {
#ifdef _DEBUG
      printf("    just copying arg [%s]\n", command[i]);
#endif
      if (NULL == (retour[i] = strdup(command[i]))) 
	yaerreur = 1;
    }
  }
  retour[i] = NULL;
#ifdef _DEBUG
  printf("  retour: %ld\n", (long) retour);
#endif
  if (!yaerreur)
    return (retour);
  else
    return (NULL);
}

/* num is the index of the mail in the mail array 
   if num == -1, means that the command isn't for each mail */


void		spawn_command(int num, char **command, int nbnewmail, 
			      char *display, Pop3 pc)
{
  char		**tmpcommand;
  char		str[16];
  int		i, j;
  
  if (display) {
    ClearDisplay();
    if (!command) {
      BlitString(display, 10, 6 + TOP_MARGIN, 0);
      BlitString("not  set", 10, 6*2 + TOP_MARGIN, 0);
      BlitString(" check", 15, 6*5 + TOP_MARGIN, 0);
      BlitString("wmpop3rc", 10, 6*6 + TOP_MARGIN, 0);
    }
    else {
      BlitString("starting", 10, TOP_MARGIN, 0);
      for (i = 0; i < 5 && command[i]; i++) {
	for (j = 0; j < 8 && command[i][j]; j++);
	memcpy(str, command[i], j);
	str[j] = '\0';
	BlitString(str, 10, (i + 2)*6 + TOP_MARGIN, 0);
      }
      RedrawWindow();
    }
  }
  if (command) {
    /* check for any '%' sign in the command and complete it */
    tmpcommand = do_command_completion(num, command, nbnewmail, pc);
    if (tmpcommand) {
      /* launch the command in a new process */
      if (0 == fork()) {
#ifdef _DEBUG
	printf("  spawning command: [");
	for (i = 0; tmpcommand[i]; i++) {
	  printf("  %s ", tmpcommand[i]);
	}
	printf("]\n");
#endif
	if (execvp(tmpcommand[0], tmpcommand)) {
	  perror("execvp");
	}
      }
      else {
	if (display) {
	  /* set the check delay to 30 seconds */
	  pc->nextCheckTime = time(0) + 30;
	}
	/* free the memory (in the parent process...) */
	for (i = 0; tmpcommand[i]; i++) {
	  free (tmpcommand[i]);
	}
	free (tmpcommand);
      }
    }
    else {
      if (display) {
	ClearDisplay();
	BlitString(" Error", 15, TOP_MARGIN, 0);
	BlitString(" While", 15, 6*2 + TOP_MARGIN, 0);
	BlitString("parsing", 15, 6*3 + TOP_MARGIN, 0);
	BlitString("command", 15, 6*4 +TOP_MARGIN, 0);
	BlitString(display, 10, 6*6 + TOP_MARGIN, 0);
	fprintf(stderr, "Error while parsing %s\n", display);
      }
      else 
	fprintf(stderr, "Error while parsing command\n");
    }
  }
  if (display) {
    RedrawWindow();
    sleep(displaydelay);
  }
}

/*
 * wmCheckMail_routine : This function is used to set up the X display
 *           for wmpop3 and check for mail every n number of minutes.
 */

void		wmCheckMail_routine(int argc, char **argv){
  
  int		buttonStatus   = -1;
  int		iS;
  Pop3		pc;
  int		i, j, k;
  XEvent	Event;
  char		str[256];
  int		index = 0;
  int		fragment = 0;
  int		index_vert = 0;
  int		oldnbmess = 0;
  int		nbsel;
  int		selectedmess;
  int		justreloaded;
  long		thistime;
  char		*linestodel[7];
  int		unreachable;
  long		sleeplenght;
  int		scrollmode = 0;
  int		nbnewmail;
  char		separ;
    
  if( !strcmp( config_file, "not-defined") )
    sprintf(config_file, "%s/.wmpop3rc", getenv("HOME"));
  
  if( readConfigFile(config_file) == -1){
    exit(0);
  }
  /* Set up timer for checking mail */
  createXBMfromXPM(wminet_mask_bits, wmpop3_xpm
		   , wminet_mask_width, wminet_mask_height);
  
  openXwindow(argc, argv, wmpop3_xpm, wminet_mask_bits
	      , wminet_mask_width, wminet_mask_height);
  
  AddMouseRegion(0, 19, 49, 30, 58); /* check button */
  AddMouseRegion(1, 46, 49, 50, 58 ); /* autocheck button   */
  AddMouseRegion(2, 53, 49, 57, 58 ); /* switch display button  */
  AddMouseRegion(3, 5, 49, 16, 58 ); /* delete button */ 
  AddMouseRegion(4, 33, 49, 44, 53); /* top arrow */
  AddMouseRegion(5, 33, 54, 44, 58); /* botton arrow */
  /* add the mouse regions for each line */
  for (i = 0; i < NB_LINE; i++) {
    AddMouseRegion(6 + i, 9, (i*6) + TOP_MARGIN, 
		   58, (i*6) + TOP_MARGIN + CHAR_HEIGHT);
  }
  
  
  /* Check if Autochecking is on or off */
  if(autoChecking == NO ){
    copyXPMArea(142, 38, 7, 11, 45, 48);
  }
  else {
    copyXPMArea(142, 49, 7, 11, 45, 48);
  }
  
  RedrawWindow();
  
  summess = 0;

  deleteoldtmpfiles();

  sleeplenght = (long) 20000L + (20000L - (20000L * scrollspeed / 100));
  while (1) {
    RSET_COLOR;
    for (iS = 0; iS < nb_conf; iS++) {
      pc = conf[iS];
      if( (((time(0) > pc->nextCheckTime) || 
	    (pc->nextCheckTime == 0)) && ( autoChecking == YES)) 
	  || (pc->forcedCheck == YES)){ 
	justreloaded = 1;
	ClearDisplay();
	BlitString(pc->alias, 10, TOP_MARGIN, 0);
	BlitString("connect", 10, (6*2) + TOP_MARGIN, 0);
	RedrawWindow();
	pc->status = 0;
	if(pop3MakeConnection(pc,pc->popserver,pc->serverport) == -1){
	  pc->status = 1;
	}
	if (!pc->status) {
	  BlitString("login", 10, (6*3) + TOP_MARGIN, 0);
	  RedrawWindow();
	  if (pop3Login(pc, pc->username, pc->password) == -1 ) 
	    pc->status = 2;
	}
	if (!pc->status) {
	  BlitString("get mail", 10, (6*4) + TOP_MARGIN, 0);
	  RedrawWindow();
	  if (pop3CheckMail(pc) == -1)
	    pc->status = 3;
	}
	pc->nextCheckTime = time(0) + (pc->mailCheckDelay * SEC_IN_MIN);
	index = 0;
	pc->forcedCheck = NO;
	/* check if new mail has arrived */
	for (i = 0, nbnewmail = 0; i < pc->numOfMessages; i++)
	  if (pc->mails[i].new)
	    nbnewmail++;
	/* launch the new mail command */
	if (pc->newmailcommand && (-1 != pc->sizechanged)) {
	  if (nbnewmail) {
#ifdef _DEBUG
	    printf("  %d New mail(s) ha(s)(ve) arrived!\n", nbnewmail);
#endif
	    if (is_for_each_mail(pc->newmailcommand)) {
	      for (i = 0; i < pc->numOfMessages; i++) 
		if (pc->mails[i].new)
		  spawn_command(i, pc->newmailcommand, nbnewmail, NULL, pc);
	    }
	    else {
	      spawn_command(-1, pc->newmailcommand, nbnewmail, NULL, pc);
	    }
	  }
	}
	/* close the connection */
	pop3Quit(pc);
	pc->sizechanged = 0;
      }
    }
    waitpid(0, NULL, WNOHANG);
    /* reset the number of messages */
    unreachable = 1;
    for (summess = 0, iS = 0; iS < nb_conf; iS++) {
      pc = conf[iS];
      if (!pc->status) {
	unreachable = 0;
	summess += pc->numOfMessages;
      }
    }
    /* set the offset from the beginning of the list */
    if (summess != oldnbmess) {
      oldnbmess = summess;
      if (summess < NB_LINE)
	index_vert = 0;
      else {
	index_vert = summess - NB_LINE;
      }
    }
    memset(str, 0, 128);
    /* clear the display */
    ClearDisplay();
    nbsel = 0;
    for (iS = 0; iS < nb_conf; iS++) {
      pc = conf[iS];
      for (i = 0; i < summess && i < pc->numOfMessages; i++) {
#ifdef _DEBUG
	if (haspassed) {
	  printf("  %s, mails[%d], todelete=%d\n", 
	       pc->alias, i, pc->mails[i].todelete);
	}
#endif
	nbsel += pc->mails[i].todelete;
      }   
    } 
    if (justreloaded) {
      /* make sure we display the correct buttons */
      justreloaded = 0;
      if ((NO == newMessagesOnly) && nbsel)
	copyXPMArea(128,49 ,14 ,11 ,18 ,48 );
      else
	copyXPMArea(128,27 ,14 ,11 ,18 ,48 );
    }
    if (newMessagesOnly == YES ){
      /* Show messages waiting */
      RSET_COLOR;
      for (color = 0, iS = 0; iS < nb_conf; iS++) {
	pc = conf[iS];
	if (pc->countunreadonly)
	  separ = '-';
	else 
	  separ = ':';
	switch(pc->status) {
	case 1:
	  sprintf(str, "%-3s%cC*ER", pc->alias, separ);
	  break;
	case 2:
	  sprintf(str, "%-3s%cL*ER", pc->alias, separ);
	  break;
	case 3:
	  sprintf(str, "%-3s%cM*ER", pc->alias, separ);
	  break;
	default:
	  sprintf(str, "%-3s%c %3d", pc->alias, separ,
		  (pc->countunreadonly) ? pc->numOfUnreadMessages : 
		  pc->numOfMessages);
	  break;
	}
	BlitString(str, 10, (int) (6*iS) + TOP_MARGIN, 0);
	SWAP_COLOR;
      }
      RSET_COLOR;
      sprintf(str, "sel.: %2d", nbsel);
      BlitString(str, 10, (int) (6*6) + TOP_MARGIN, 0);	  
    }
    else {
      RSET_COLOR;
      if (0 == summess) {
	if (unreachable) {
	  BlitString(" error", 10, TOP_MARGIN, 0);
	}
	else {
	  BlitString("No  Mesg", 10, TOP_MARGIN, 0);
	}
	if (autoChecking == YES) {
	  BlitString("  next", 10, 6*2 + TOP_MARGIN, 0);
	  BlitString(" update", 10, 6*3 + TOP_MARGIN, 0);
	  j = SEC_IN_MIN * 1000; 
	  thistime = time(0);
	  for (i = 0, k = 0; i < nb_conf; i++) {
	    if ((conf[i]->nextCheckTime - thistime) < j) {
	      j = conf[i]->nextCheckTime - thistime;
	      k = i;
	    }
	  }
	  sprintf(str, " is:%s", conf[k]->alias);
	  BlitString(str, 10, 6*4 + TOP_MARGIN, 0);
	  BlitString("   in", 10, 6*5 + TOP_MARGIN, 0);
	  sprintf(str, "%-5d s.", j);
	  BlitString(str, 10, 6*6 + TOP_MARGIN, 0);
	}
      }
      else {
	RSET_COLOR;
	/* iS = index in the conf struct
	   i = index of the mail 
	   j = line nb on display */
	memset(linestodel, 0, sizeof(char *));
	for (iS = 0, j = 0; iS < nb_conf && j < NB_LINE + index_vert; iS++) {
	  pc = conf[iS];
	  for (i = 0; (j < NB_LINE + index_vert) && i < pc->numOfMessages; 
	       j++, i++) {
	    if (j >= index_vert) {
	      build_line(str, i, index, pc);
	      BlitString(str, 10, ((j - index_vert) * 6) + TOP_MARGIN, 
			 fragment);
	      display_index(i, (j - index_vert), pc);
	      /* store the address of the delete flag, so that it will */
	      /* be easier to modify it afterwards */
	      linestodel[j - index_vert] = &(pc->mails[i].todelete);
	    }
	  }
	  SWAP_COLOR;
	}
	display_scrollbar(index_vert, summess, NB_LINE, scrollmode);
      }
      fragment++;
      if (0 == (fragment % (CHAR_WIDTH + 1))) {
	index++;
	fragment = 0;
	/*	printf("index=%d, fragment=%d\n", index, fragment); */
      }
    }
#ifdef _DEBUG
    haspassed = 0;
#endif
    
    RedrawWindow();
    
    RSET_COLOR;
    /* X Events */
    while (XPending(display)){
      XNextEvent(display, &Event);
      switch (Event.type) {
      case Expose:
	RedrawWindow();
	break;
      case DestroyNotify:
	XCloseDisplay(display);
	exit(0);
	break;
      case MotionNotify:
	if (scrollmode) {
#ifdef _DEBUG
	  printf("  ca bouge... index_vert before = %d, %d x %d, allowedspace: %d, summess: %d\n",
		 index_vert,
		 Event.xbutton.x, Event.xbutton.y,
		 scrollbar.allowedspace,
		 summess);

#endif
	  if (summess > NB_LINE) {
	    index_vert = scrollbar.orig_index_vert + 
	      ((Event.xbutton.y  - scrollbar.orig_y) * (summess - NB_LINE) / 
	       scrollbar.allowedspace);
	    if (0 > index_vert)
	      index_vert = 0;
	    if (index_vert + NB_LINE > summess)
	      index_vert = summess - NB_LINE;
	  }
#ifdef _DEBUG
	  printf("  deplacement de %d pixels --> index_vert = %d\n", 
		 Event.xbutton.y  - scrollbar.orig_y, index_vert);
#endif
	}
	break;
      case ButtonPress:
	buttonStatus = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
	if (buttonStatus >= 0){
	  switch (buttonStatus){
	  case 0 : /* check / open button pressed */
	    if ((NO == newMessagesOnly) && nbsel) 
	      copyXPMArea(128, 60 ,14 ,11 ,18 ,48 );
	    else
	      copyXPMArea(128,38 ,14 ,11 ,18 ,48 );
	    break;
	  case 1 : /* autocheck button pressed */
	    break;
	  case 2: /* switch display button pressed */
	    break;
	  case 3: /* delete button pressed */
	    copyXPMArea(127, 15, 14, 11, 4, 48);
	    break;
	  case 4: /* top arrow button pressed */
	    break;
	  case 5: /* bottom arrow button pressed */
	    break;
	  default:
	    break;
	  }
	  RedrawWindow();
	}
	else if (SCROLL_LX <= Event.xbutton.x && SCROLL_RX >= Event.xbutton.x
		 && scrollbar.top <= Event.xbutton.y && 
		 scrollbar.bottom >= Event.xbutton.y) {
	  scrollbar.orig_y = Event.xbutton.y;
	  scrollbar.orig_index_vert = index_vert;
	  scrollmode = 1;
	}
	break;
      case ButtonRelease:
	i = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
	
	if (buttonStatus == i && buttonStatus >= 0){
	  switch (buttonStatus){
	  case 0 :   /* check button */
	    if (nbsel && !newMessagesOnly) {
	      copyXPMArea(128,49 ,14 ,11 ,18 ,48 );	
	      /* this is where you launch the open mail command */
	      for (iS = 0; iS < nb_conf; iS++) {
		pc = conf[iS];
		for (i = 0, selectedmess = 0; i < pc->numOfMessages; i++) {
		  if (pc->mails[i].todelete) {
		    selectedmess = 1;
		    break;
		  }
		}
		if (selectedmess) {
#ifdef _DEBUG
		  printf("  launching selectedmesgcommand command for conf %d\n",
			 iS);
#endif
		  
		  if (is_for_each_mail(pc->selectedmesgcommand)) {
#ifdef _DEBUG
		    if (!pc->numOfMessages) {
		      printf("  command is for each mail but there's no mail\n");
		    }
		    else
		      printf("  command is for each mail\n");
#endif
		    for (i = 0; i < pc->numOfMessages; i++) 
		      spawn_command(i, pc->selectedmesgcommand, 0, 
				    "selectm.", pc);
		  }
		  else {
		    spawn_command(-1, pc->selectedmesgcommand, 0, 
				  "selectm.", pc);
		  }
		}
	      }
	    }
	    else {
	      copyXPMArea(128,27 ,14 ,11 ,18 ,48 );	
	      for (iS = 0; iS < nb_conf; iS++)
		conf[iS]->forcedCheck = YES;
	    }
	    break;
	  case 1 :   /* autocheck Button */
	    if (autoChecking == YES){
	      autoChecking = NO;
	      copyXPMArea(142, 38, 7, 11, 45, 48);		  
	    }
	    else {
	      autoChecking = YES;
	      for (iS = 0; iS < nb_conf; iS++)
		conf[iS]->nextCheckTime = time(0) + 
		  (conf[iS]->mailCheckDelay * SEC_IN_MIN);
	      copyXPMArea(142, 49, 7, 11, 45, 48);
	    }
	    break;
 	  case 2:  /* switch display Button */
	    index = 0;
	    /* change view on # of messages */
	    if( newMessagesOnly == YES ) {
	      newMessagesOnly = NO;
	      copyXPMArea(149,38 , 7 , 11, 52, 48);
	      if (nbsel) {
		copyXPMArea(128,49,14,11,18,48);
	      }
	      else {
		copyXPMArea(128,27,14,11,18,48);
	      }
	    } 
	    else {
	      newMessagesOnly = YES;
	      copyXPMArea(149,49 , 7 , 11, 52, 48);
	      copyXPMArea(128,27,14,11,18,48);
	    }
#ifdef _DEBUG
	    haspassed = 1;
#endif
	    break;
	  case 3: /* delete button */
	    copyXPMArea(143, 15, 14, 11, 4, 48);
	    j = 0;
	    RSET_COLOR;
	    for (iS = 0; iS < nb_conf; iS++) {
	      pc = conf[iS];
	      for (i = 0, k = 0; i < pc->numOfMessages; i++) 
		k += pc->mails[i].todelete;
	      if (k) {
		/* clear the display */
		ClearDisplay();
		k = DeleteMail(pc);
		if (k < 0) {
		  sprintf(pc->delstatus, "%-3s: Err", pc->alias);
		}
		else if (k > 0) {
		  sprintf(pc->delstatus, "%-3s/D:%2d", pc->alias, k);
		}
		else 
		  sprintf(pc->delstatus, "%-3s:  ok", pc->alias);
		pc->forcedCheck = YES;
	      }
	      else {
		sprintf(pc->delstatus, "%-3s:none", pc->alias);
	      }
	    }
	    /* clear the display */
	    ClearDisplay();
	    RSET_COLOR;
	    for (iS = 0; iS < nb_conf; iS++) {
	      BlitString(conf[iS]->delstatus, 10, 6*iS + TOP_MARGIN, 0);
	      SWAP_COLOR;
	    }
	    RedrawWindow();
	    sleep(displaydelay);
	    break;
	  case 4: /* top arrow button pressed */
	    index_vert--;
	    if (0 > index_vert)
	      index_vert = 0;
#ifdef _DEBUG
	    haspassed = 1;
#endif
	    break;
	  case 5: /* bottom arrow button pressed */
	    if (summess > NB_LINE) {
	      index_vert++;
	      if (index_vert + NB_LINE > summess)
		index_vert = summess - NB_LINE;
	    }
#ifdef _DEBUG
	    haspassed = 1;
#endif
	    break;
	  default:
	    if (newMessagesOnly == NO) { /* message view mode */
	      if ((5 < buttonStatus) && (buttonStatus <= 5 + NB_LINE)) {
		if ((buttonStatus - 6 + index_vert) < summess) {
		  /* first update lines to del */
		  *(linestodel[buttonStatus - 6]) = 
		    1 - *(linestodel[buttonStatus - 6]);
#ifdef _DEBUG
		  printf("  button %d pressed, j'update lines to del\n", 
			 buttonStatus - 6);
		  haspassed = 1;
#endif
		  nbsel = 0;
		  for (iS = 0; iS < nb_conf; iS++) {
		    pc = conf[iS];
		    for (i = 0; i < pc->numOfMessages; i++) {
		      nbsel += pc->mails[i].todelete;
#ifdef _DEBUG
		      printf("  conf %d, mail %d, todelete = %d\n",
			     iS, i, pc->mails[i].todelete);
#endif
		    }
		  }
		  /* display open or reload buttons */
		  if (nbsel) {
		    copyXPMArea(128,49,14,11,18,48);
		  }
		  else {
		    copyXPMArea(128,27,14,11,18,48);
		  }
		}
	      }
	    }
	    else { /* summary view mode */
	      if ((5 < buttonStatus) && (buttonStatus <= 5 + nb_conf)) {
		if ((Event.xbutton.x > 10) && 
		    (Event.xbutton.x < (10 + (4 * (CHAR_WIDTH + 1))))) {
#ifdef _DEBUG
		  printf("  launching command for conf %d\n",
			 buttonStatus - 6);
#endif
		  pc = conf[buttonStatus - 6];
		  if (is_for_each_mail(pc->mailclient)) {
#ifdef _DEBUG
		    if (!pc->numOfMessages) {
		      printf("  command is for each mail but there's no mail\n");
		    }
#endif
		    for (i = 0; i < pc->numOfMessages; i++) 
		      spawn_command(i, pc->mailclient, nbnewmail, 
				    "mailcli.", pc);
		  }
		  else {
		    spawn_command(-1, pc->mailclient, nbnewmail, 
				  "mailcli.", pc);
		  }
		}
		else if ((Event.xbutton.x > (10 + (4 * (CHAR_WIDTH + 1)))) && 
			 (Event.xbutton.x < (10 + (8 * (CHAR_WIDTH + 1))))) {
		  /* swap view mode */
		  conf[buttonStatus - 6]->countunreadonly = 
		    1 - conf[buttonStatus - 6]->countunreadonly;
#ifdef _DEBUG
		  printf("  swapping view mode for conf %d: %s\n",
			 buttonStatus - 6,
			 (conf[buttonStatus - 6]->countunreadonly) ?
			 "UnreadMessages" : "TotalMessages");
		  
#endif		  
		}
	      }
	      else if ((5 + NB_LINE) == buttonStatus) { 
		/* status summary line pressed */
		if ((Event.xbutton.x > 10) && 
		    (Event.xbutton.x < (10 + (4 * (CHAR_WIDTH + 1))))) {
		  /* select all messages */
		  for (iS = 0; iS < nb_conf; iS++) {
		    for (pc = conf[iS], i = 0; i < pc->numOfMessages; i++) {
		      pc->mails[i].todelete = 1;
		    }
		  }
		}
		else if ((Event.xbutton.x > (10 + (4 * (CHAR_WIDTH + 1)))) && 
			 (Event.xbutton.x < (10 + (8 * (CHAR_WIDTH + 1))))) {
		  /* unselect all messages */
		  for (iS = 0; iS < nb_conf; iS++) {
		    for (pc = conf[iS], i = 0; i < pc->numOfMessages; i++) {
		      pc->mails[i].todelete = 0;
		    }
		  }
		}
	      }
	    }
	    break;
	  }
	}
	else {
	  if (buttonStatus >= 0) {
	    /* button has been pressed correctly but released somewhere else */
	    switch(buttonStatus) {
	    case 0: /* check button was pressed */
	      if ((NO == newMessagesOnly) && nbsel)
		copyXPMArea(128,49 ,14 ,11 ,18 ,48 );
	      else
		copyXPMArea(128,27 ,14 ,11 ,18 ,48 );
	      break;
	    case 3: /* delete button was pressed */
	      copyXPMArea(143, 15, 14, 11, 4, 48);
	      break;
	    }
	  }
	}
	RedrawWindow();
	buttonStatus = -1;
	scrollmode = 0;
	break;
      }
    }
    usleep(sleeplenght);
  }
}


/*
 * usage : Prints proper command parameters of wmpop3.
 */
void usage(void)
{
  fprintf(stdout, "\nWMPop3LB - Louis-Benoit JOURDAIN (wmpop3lb@jourdain.org)\n");
  fprintf(stdout, "based on the work by Scott Holden <scotth@thezone.net>\n\n");
  fprintf(stdout, "usage:\n");
  fprintf(stdout, "    -display <display name>\n");
  fprintf(stdout, "    -geometry +XPOS+YPOS      initial window position\n");
  fprintf(stdout, "    -c <filename>             use specified config file\n");
  fprintf(stdout, "    -h                        this help screen\n");
  fprintf(stdout, "    -v                        print the version number\n");
  fprintf(stdout, "\n");
}

void printversion(void)
{
  fprintf(stdout, "wmpop3 v%s\n", WMPOP3_VERSION);
}

// Blits a string at given co-ordinates
void BlitString(char *name, int x, int y, int fragment)
{
  int		i;
  int		c;
  int		k;
  int		row;
  
  /*  printf("--name: [%s] \n", name); */
  for (i = 0, k = x; name[i]; i++) {
    c = toupper(name[i]); 
    if (c >= 'A' && c <= 'Z') {   /* its a letter */
      c -= 'A';
      row = LETTERS;
    }
    else if (c >= '0' && c <= '9') {   /* its a number */
      c -= '0';
      row = NUMBERS;
    }
    else switch(c) {
    case '-':
      c = 26;
      row = LETTERS;
      break;
    case '*':
      c = 27;
      row = LETTERS;
      break;
    case ':': 
      c = 10;
      row = NUMBERS;
      break;
    case '/':
      c = 11;
      row = NUMBERS;
      break;
    case '@':
      c = 12;
      row = NUMBERS;
      break;
    case '%':
      c = 15;
      row = NUMBERS;
      break;
    case ' ':
      c = 13;
      row = NUMBERS;
      break;
    case '.':
      c = 28;
      row = LETTERS;
      break;
    default:
      c = 14;
       row = NUMBERS;
       break;
    }
    /*    printf("c:%2d (%c), fragment: %d, i:%d, k=%d ", 
	  c, name[i], fragment, i, k); */
    if (i > 0 && i < NB_DISP) {
      copyXPMArea(c * CHAR_WIDTH, CH_COLOR(row), CHAR_WIDTH + 1, CHAR_HEIGHT, 
		  k, y);
      /*      printf(" - k1: %d += %d + 1", k, CHAR_WIDTH); */
      k += CHAR_WIDTH + 1;
    }
    else if (0 == i) {
      copyXPMArea(c * CHAR_WIDTH + fragment, CH_COLOR(row), 
		  CHAR_WIDTH + 1 - fragment, CHAR_HEIGHT, 
		  k, y);
      /*      printf(" - k2: %d += %d + 1 - %d", k, CHAR_WIDTH, fragment); */
      k += CHAR_WIDTH + 1 - fragment;
    }
    else if (fragment && (NB_DISP == i)) {
      copyXPMArea(c * CHAR_WIDTH, CH_COLOR(row), fragment + 1, CHAR_HEIGHT, 
		  k, y);
      /*       printf(" - k3: %d += %d ", k, fragment); */
      k += fragment;
    }
    /*    printf("  -- apres k=%d\n", k); */
  }
}


/* Blits number to given coordinates...*/

void BlitNum(int num, int x, int y, int todelete)
{
  if (todelete)
    num += 19;
  copyXPMArea(((num - 1) * (SN_CHAR_W + 1)) + 1, CH_COLOR(SMALL_NUM), 
	      SN_CHAR_W, CHAR_HEIGHT + 1, x, y);
}

int	parsestring(char *buf, char *data, int max, FILE *fp) 
{
  char	*deb;
  char	*end;
  char	*bal;
  int	go = 1;
  int	linelen = 0;

  /* trim the leading spaces */
  memset(data, 0, max);
  for (deb = buf; *deb && isspace(*deb); deb++);
  if (!*deb)
    return (-1);
  if ('"' == *deb) {
    ++deb;
    bal = data;
    while (go) {
#ifdef _DEBUG
      printf("  line to parse: [%s]\n", deb);
#endif
      /* get to the end of the line */
      for (end = deb; *end && ('"' != *end); end++);
      if (!*end) { /* this is a multiline entry */
	linelen += (int) (end - deb);
	if (linelen > max) {
#ifdef _DEBUG
	  printf("  maximum line length reached\n");
#endif
	  return (-1);
	}
	memcpy(bal, deb, (int) (end - deb));
	bal = data + linelen;
	if (fgets(buf, CONF_BUFLEN, fp)) {
	  deb = buf;
	}
	else { /* end of file reached */
	  return (-1);
	}
      }
      else {
	memcpy(bal, deb, end - deb);
	go = 0;
      }
    }
  }
  else {
    for (end = deb; *end && !isspace(*end); end++);
    memcpy(data, deb, ((end - deb) > max) ? max : end - deb);
  }
#ifdef _DEBUG
  printf("  parsed string (len=%d) : [%s]\n", strlen(data), data);
#endif
  return (0);
}

int	parsenum(char *buf, int *data)
{
  char	*deb;
  char	*end;
  char	temp[32];

  memset(temp, 0, 32);
  for (deb = buf; *deb && isspace(*deb); deb++);
  if (!*deb)
    return (-1);
  if ('"' == *deb) {
    for (end = ++deb; *end && ('"' != *end); end++);
    if (!*end)
      return (-1);
    memcpy(temp, deb, end - deb);
    *data = atoi(temp);
  }
  else {
    for (end = deb; *end && !isspace(*end); end++);
    memcpy(temp, deb, end - deb);
    *data = atoi(temp);
  }
  return (0);
}

char		**build_arg_list(char *buf, int len)
{
  int		espaces;
  int		i, j;
  char		**retour;

  /* count number of args */
  for (espaces = 0, i = 0; buf[i] && i < len; i++)
    if (isspace(buf[i]))
      espaces++;
  /* allocate space for the structure */
  if (NULL == (retour = (char **) malloc(sizeof(char *) * espaces + 2)))
    return (NULL);
  /* get each arg one by one */
  for (i = 0, j = 0; j < len && i < 256; i++) {
    /* get the end of the arg */
    for (espaces = j; espaces < len && !isspace(buf[espaces]); espaces++);
    /* allocate space for the arg */
    if (0 == (retour[i] = malloc(sizeof(char) * (espaces - j) + 1))) {
      /* free what has been allocated */
      for (j = 0; j < i; j++)
	free(retour[j]);
      return (NULL);
    }
    memcpy(retour[i], buf +  j, espaces - j);
    retour[i][espaces - j] = '\0';
    j = espaces + 1;
  }
  retour[i] = 0;
  return (retour);
}


int		readConfigFile( char *filename ){
    char	buf[CONF_BUFLEN];
    char	tmp[CONF_BUFLEN];
    FILE	*fp;
    char	*bal;

    if( (fp = fopen( filename, "r")) == 0 ){
        sprintf(config_file, "%s/.wmpop3rc", getenv("HOME"));
        fprintf(stderr, "-Config file does not exit : %s\n",config_file);
        fprintf(stderr, "+Trying to create new config file.\n");
        if((fp = fopen(config_file,"w")) == 0){
            fprintf(stderr, "-Error creating new config file\n");
            return -1;
        }
	fprintf(fp, "#####################################################\n");
	fprintf(fp, "#             wmpop3lb configuration file           #\n");
	fprintf(fp, "#                                                   #\n");
	fprintf(fp, "# for more information about wmpop3lb, please see:  #\n");
	fprintf(fp, "#          http://wmpop3lb.jourdain.org             #\n");
	fprintf(fp, "#               or send a mail to                   #\n");
	fprintf(fp, "#             wmpop3lb@jourdain.org                 #\n");
	fprintf(fp, "#####################################################\n");
        fprintf(fp, "autochecking         0   # 1 enables, 0 disables\n");
	fprintf(fp, "displaydelay         2   # nb of seconds error info is displayed\n");
	fprintf(fp, "scrollspeed          100 # percentage of original scrool speed\n");
	fprintf(fp, "tempdir              /tmp # directory for tmp files\n");
        fprintf(fp, "viewallmessages      0   # 0 Shows the from and subject\n");
        fprintf(fp, "#                          1 Shows the number of messages\n");
        fprintf(fp, "#\n# Replace all values with appropriate data\n#\n");
	fprintf(fp, "[Server]  # server section\n");
	fprintf(fp, "alias                \"3 alphanum. char. long alias\"\n");
        fprintf(fp, "popserver            \" pop3 server name \"\n");
        fprintf(fp, "port                 110 # default port\n");
        fprintf(fp, "username             \" pop3 login name  \"\n");
        fprintf(fp, "password             \" pop3 password   \"\n");
        fprintf(fp, "mailcheckdelay       10  # default mail check time in minutes\n");
	fprintf(fp, "countUnreadOnly      0   # count unread messages only\n");
	fprintf(fp, "mailclient           \"netscape -mail\" # for example...\n");
	fprintf(fp, "newmailcommand       \" specify new mail command \"\n");
	fprintf(fp, "selectedmesgcommand  \"specify command for selected mess\"\n");
	fprintf(fp, "mailseparator        \" separator when concatening messages\"\n");
	fprintf(fp, "maxdlsize		  -1 # (no limit)\n");
	fprintf(fp, "#\n# start new [server] section below (up to a total of 6)\n");
        fprintf(stderr, "+New config file created : ~/.wmpop3rc\n\n");
        fprintf(stderr, "+ ~/.wmpop3rc must be configured before running wmpop3.\n");
        fclose(fp);
        return -1;
    }
    
    nb_conf = 0;
    tempdir[0] = '\0';
    
    while ((nb_conf < 7) && fgets(buf, CONF_BUFLEN, fp) != 0) {
      if (buf[0] != '#') {
	if (!nb_conf && !strncasecmp( buf, "autochecking", 12) ){
	  if (parsenum(buf + 12, &autoChecking)) 
	    fprintf(stderr, "syntax error for parameter autochecking\n");
	}
	else if (!nb_conf && !strncasecmp( buf, "scrollspeed", 11) ){
	  if (parsenum(buf + 11, &scrollspeed)) 
	    fprintf(stderr, "syntax error for parameter scrollspeed\n");
	}
	else if (!nb_conf && !strncasecmp( buf, "displaydelay", 12) ){
	  if (parsenum(buf + 12, &displaydelay))
	    fprintf(stderr, "syntax error for parameter displaydelay\n");
	}
	else if (!nb_conf && !strncasecmp(buf, "tempdir", 7)) {
	  if (parsestring(buf + 7, tempdir, 1024, fp))
	    fprintf(stderr, "syntax error for parameter tempdir\n");
	}
	else if (!strncasecmp(buf, "[server]", 8)) {
	  nb_conf++;
	  if (!(conf[nb_conf - 1] = pop3Create(nb_conf))) {
	    fprintf(stderr, "Can't allocate memory for config structure\n");
	    fclose(fp);
	    return (-1);
	  }
	}
	else if (nb_conf && !strncasecmp(buf, "username", 8) ) {
	  if (parsestring(buf + 8, conf[nb_conf -1]->username, 256, fp))
	    fprintf(stderr, "section %d: invalid syntax for username\n", 
		    nb_conf);
	}
	else if (nb_conf && !strncasecmp( buf, "password", 8) ){
	  if (parsestring(buf + 8, conf[nb_conf - 1]->password, 256, fp))
	    fprintf(stderr, "section %d: invalid syntax for password\n", 
		    nb_conf);
	}
	else if (nb_conf && !strncasecmp(buf, "alias", 5) ) {
	  if (parsestring(buf + 5, conf[nb_conf -1]->alias, 3, fp))
	    fprintf(stderr, "section %d: invalid syntax for alias\n", nb_conf);
	}
	else if (nb_conf && !strncasecmp( buf, "popserver", 9) ){
	  if (parsestring(buf + 9, conf[nb_conf - 1]->popserver, 128, fp))
	    fprintf(stderr, "section %d: invalid syntax for popserver\n", 
		    nb_conf);
	}
	else if (nb_conf && !strncasecmp( buf, "port", 4) ){
	  if (parsenum(buf + 4, &(conf[nb_conf - 1]->serverport)))
	    fprintf(stderr, "section %d: Invalid popserver port number.\n", 
		    nb_conf);
	}
	else if (!nb_conf && !strncasecmp( buf, "viewallmessages", 15) ){
	  if (parsenum(buf + 15, &newMessagesOnly))
	    fprintf(stderr, "section %d: Invalid number ( viewallmessages )\n",
		    nb_conf);
	}
	else if (nb_conf && !strncasecmp(buf, "countunreadonly", 15)) {
	  if (parsenum(buf + 15, &(conf[nb_conf - 1]->countunreadonly)))
	    fprintf(stderr, "section %d: Invalid number ( countunreadonly )\n",
		    nb_conf);
	}
	else if (nb_conf && !strncasecmp( buf, "mailcheckdelay", 14) ){
	  if (parsenum(buf + 14, &(conf[nb_conf -1]->mailCheckDelay)))
	    fprintf(stderr, "section %d: Invalid delay time.\n", nb_conf);
	}
	else if (nb_conf && !strncasecmp(buf, "mailclient", 10)) {
	  if (parsestring(buf + 10, tmp, 256, fp))
	    fprintf(stderr, "section %d: Invalid syntax for mailclient.\n", 
		    nb_conf);
	  else
	    conf[nb_conf - 1]->mailclient = build_arg_list(tmp, strlen(tmp));
	}
	else if (nb_conf && !strncasecmp(buf, "newmailcommand", 14)) {
	  if (parsestring(buf + 14, tmp, 256, fp))
	    fprintf(stderr,"section %d: Invalid syntax for newmailcommand.\n", 
		    nb_conf);
	  else 
	    conf[nb_conf - 1]->newmailcommand = 
	      build_arg_list(tmp, strlen(tmp));
	}
	else if (nb_conf && !strncasecmp(buf, "selectedmesgcommand", 19)) {
	  if (parsestring(buf + 19, tmp, 256, fp))
	    fprintf(stderr,
		    "section %d: Invalid syntax for selectedmesgcommand.\n", 
		    nb_conf);
	  else
	    conf[nb_conf - 1]->selectedmesgcommand = 
	      build_arg_list(tmp, strlen(tmp));
	}
	else if (nb_conf && !strncasecmp(buf, "mailseparator", 13)) {
	  if (parsestring(buf + 13, conf[nb_conf - 1]->mailseparator, 256, fp))
	    fprintf(stderr, "section %d: Invalid syntax for mailseparator\n",
		    nb_conf);
	}
	else if (nb_conf && !strncasecmp( buf, "maxdlsize", 9) ){
	  if (parsenum(buf + 9, &(conf[nb_conf -1]->maxdlsize)))
	    fprintf(stderr, "section %d: Invalid maxdlsize.\n", nb_conf);
	}
	else if (nb_conf) { 
	  if (*buf && (isalpha(*buf) || isalnum(*buf)))
	    fprintf(stderr, "section %d: Unknown indentifier : [%s]\n", 
		    nb_conf, buf);
	}
	else {
	  for (bal = buf; *bal && !isalnum(*bal); bal++);
	  if (*bal)
	    fprintf(stderr, "identifier outside Server section: [%s]\n", buf);
	}
      } 
    }
    fclose(fp);
    return 0;
}







