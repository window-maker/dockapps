/* Author : Louis-Benoit JOURDAIN (lb@jourdain.org)
 * based on the original work by Scott Holden ( scotth@thezone.net )
 *
 * multi Pop3 Email checker.
 *
 * Last Updated : Feb 7th, 2002
 *
 */


#include "Pop3Client.h"
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <X11/xpm.h>
#include <X11/Xlib.h>
#include <X11/extensions/shape.h>

#include <libdockapp/wmgeneral.h>

#include "Pop3Client.h"

/* return size if all goes well, -1 if not expected answer */
int	send_command(char *exp_answer, char **retour, Pop3 pc)
{
  int	size;

  send(pc->s, &pc->outBuf,strlen(pc->outBuf),0);
  size = recv(pc->s,pc->inBuf,1024,0);
  memset(*retour,0,1024);
  memcpy(*retour,pc->inBuf,size);
  if (strncmp(pc->inBuf, exp_answer, strlen(exp_answer))) {
    return (-1);
  }
  return (size);
}

Pop3 pop3Create(int num)
{
    Pop3 pc;

    pc = (Pop3)malloc( sizeof(*pc) );
    if( pc == 0)
        return 0;
    memset(pc, 0, sizeof(*pc));
    pc->connected     = NOT_CONNECTED;
    pc->serverport    = 110;
    pc->mailCheckDelay = 10;
    /* make sure we perform an action first time we receive mail */
    pc->forcedCheck = 1;
    pc->sizechanged = -1;
    pc->maxdlsize = -1;
    sprintf(pc->alias, "nb%d", num);
    return (pc);
}
int pop3MakeConnection(Pop3 pc, char *serverName, int port){

    pc->s = socket(AF_INET, SOCK_STREAM, 0 );
    memset( &(pc->server), 0 , sizeof(pc->server));
    pc->server.sin_family = AF_INET;
    pc->hp = gethostbyname(serverName);
    if( pc->hp == 0)
        return -1;
    memcpy( &(pc->server.sin_addr), pc->hp->h_addr, pc->hp->h_length);
    pc->server.sin_port = htons(port);
    if ( connect(pc->s, (struct sockaddr *)&(pc->server)
                               , sizeof(pc->server)) < 0 )
        return -1;
    pc->connected = CONNECTED;
   return 0;
}

int pop3IsConnected(Pop3 pc){

    if( pc->connected == CONNECTED)
        return 1;
    return 0;
}

int pop3Login(Pop3 pc, char *name, char *pass){

      int size;
      char temp[1024];

      if( pc->connected == NOT_CONNECTED ){
          perror("Not Connected To Server\n");
          return -1;
      }

      size = recv(pc->s,&pc->inBuf,1024,0);
      memset(temp,0,1024);
      memcpy(temp,pc->inBuf,size);
      if( temp[0] != '+' ){
          fprintf(stderr,"Error Logging in\n");
          return -1;
      }

      sprintf(pc->outBuf,"USER %s\r\n",name);
      send(pc->s, &pc->outBuf,strlen(pc->outBuf),0);
      size =recv(pc->s,pc->inBuf,1024,0);
      memset(temp,0,1024);
      memcpy(temp,pc->inBuf,size);
      if( temp[0] != '+' ){
          fprintf(stderr,"Invalid User Name\n");
          return -1;
      }

      memset(pc->outBuf,0,1024);
      sprintf(pc->outBuf,"PASS %s\r\n",pass);
      send(pc->s, pc->outBuf, strlen(pc->outBuf),0 );
      size =recv(pc->s,&pc->inBuf,1024,0);
      memset(temp,0,1024);
      memcpy(temp,pc->inBuf,size);
      if( temp[0] != '+' ){
          fprintf(stderr,"Incorrect Password\n");
          return -1;
      }

      return 0;
}

int	store_from(char *buf, int pos, int buflen, int num, Pop3 pc) {
  int	end;
  int	deb_addr;
  int	fin_addr;
  int	i;

  /* strip the white spaces */
  for (; (pos < buflen) && isspace(buf[pos]); pos++);
  /* get the end of the line */
  for (end = pos; (end + 1 < buflen) &&
	 (buf[end] != '\r') && (buf[end + 1] != '\n'); end++);
  /* try to find the '@' sign for the address */
  for (i = pos; (i < end) && buf[i] != '@'; i++);
  if (i < end) { /* we found a @ sign before the end of the line */
    for (deb_addr = i; deb_addr > pos && !isspace(buf[deb_addr]); deb_addr--);
    while (isspace(buf[deb_addr]) || ('<' == buf[deb_addr]))
      deb_addr++;
    for (fin_addr = i; fin_addr < end && !isspace(buf[fin_addr]); fin_addr++);
    if ('>' == buf[fin_addr - 1])
      fin_addr--;
    memcpy(pc->mails[num].from, buf + deb_addr,
	   (fin_addr - deb_addr > MAIL_BUFLEN) ?
	   MAIL_BUFLEN : fin_addr - deb_addr);
  }
  else {
    memcpy(pc->mails[num].from, buf + pos,
	   (end - pos > MAIL_BUFLEN) ? MAIL_BUFLEN : end - pos);
  }
  return (end + 2);
}

int	store_subject(char *buf, int pos, int buflen, int num, Pop3 pc) {
  int	end;
  int	i;

  /* get the end of the line */
  for (end = pos, i = 0; (i < MAIL_BUFLEN) && (end + 1 < buflen) &&
	 (buf[end] != '\r') && (buf[end + 1] != '\n'); end++, i++);
  for (; (pos < buflen) && (buf[pos] == ' '); pos++);
  memcpy(pc->mails[num].subject, buf + pos, end - pos);
  return (end + 2);
}

int	parse_header(char *buf, int buflen, int num, Pop3 pc) {
  int	pos = 0;
  int	retour = 1;

  /* parse the header for the from and subject fields */
#ifdef _DEBUG
  printf("  parse_header, buflen: %d, num %d, buf:\n%s\n",
	 buflen, num, buf);
#endif
  while (pos < buflen) {
    if ((buflen > (pos + 2)) && !strncmp(".\r\n", buf + pos, 3)) {
      /* this is the end */
      pos = buflen;
      retour = 0;
    }
    else if ((buflen > (pos + 4)) && !strncmp("From:", buf + pos, 5)) {
      pos += 5;
      pos = store_from(buf, pos, buflen, num, pc);
    }
    else if ((buflen > (pos + 7)) && !strncmp("Subject:", buf + pos, 8)) {
      pos += 8;
      pos = store_subject(buf, pos, buflen, num, pc);
    }
    else {
      for( ; (buflen > (pos + 2)) && (buf[pos] != '\r') &&
	     (buf[pos + 1] != '\n'); pos++);
      pos += 2;
    }
  }
  return (retour);
}

int	pop3VerifStats(Pop3 pc)
{
  int	retour = 0;
  char	*ptr;
  int	size;

  if( pc->connected == NOT_CONNECTED )
    return (-1);

  /* Find total number of messages in mail box */
  sprintf(pc->outBuf,"STAT\r\n");
  send(pc->s, pc->outBuf, strlen(pc->outBuf),0 );
  size = recv(pc->s,pc->inBuf,1024,0);
  if( pc->inBuf[0] != '+' ){
    perror("Error Receiving Stats");
    return (-1);
  }
  ptr = strtok(pc->inBuf, " ");
  ptr = strtok( 0," ");
  ptr = strtok( 0, " ");
  if (ptr)
    retour = atoi(ptr);
  return (retour);
}

long	calculate_cksum(char *buf, int len) {
  int	i;
  long	cumul;

  for (i = 0, cumul = 0; i < len; i++) {
    cumul += (long) buf[i];
  }
  return (cumul);
}

int		is_in_old_mails(long cksum, t_mail *oldmails, int numold) {
  int		i;

  for (i = 0; i < numold; i++) {
    if (oldmails[i].cksum == cksum)
      return (1);
  }
  return (0);
}

int		pop3CheckMail(Pop3 pc) {
  int		size;
  char		temp[4096];
  char		*ptr;
  int		i;
  int		pass;
  int		go;
  int		old_sizeOfAllMessages;
  int		old_numOfMessages;
  int		progressbarpos = 0;
  t_mail	*oldmails;

  if( pc->connected == NOT_CONNECTED )
    return -1;
  /* save the old values */
  old_sizeOfAllMessages = pc->sizeOfAllMessages;
  old_numOfMessages = pc->numOfMessages;
  /* Find total number of messages in mail box */
  sprintf(pc->outBuf,"STAT\r\n");
  send(pc->s, pc->outBuf, strlen(pc->outBuf),0 );
  size = recv(pc->s,pc->inBuf,1024,0);
  pc->inBuf[size] = '\0';
#ifdef _DEBUG
  printf("  pop3CheckMail, stat received buf (size=%d): [%s]\n",
	 size, pc->inBuf);
#endif
  memset(temp,0,1024);
  memcpy(temp,pc->inBuf,size);
  if( temp[0] != '+' ){
#ifdef _DEBUG
    printf("  Error Receiving Stats: [%s]\n", temp);
#endif
    return (-1);
  }

  ptr = strtok(temp, " ");
  ptr = strtok( 0," ");
  pc->numOfMessages = atoi(ptr);

  /* save the old messages */
  oldmails = pc->mails;
  pc->mails = NULL;
  /* allocate space for the new ones */
  if (pc->numOfMessages) {
    if (NULL ==
	(pc->mails = (t_mail *) malloc (sizeof(t_mail) * pc->numOfMessages))) {
      fprintf(stderr, "can't allocate memory for message structure\n");
      /* clear the old messages */
      if (oldmails) {
	free (oldmails);
      }
      return (-1);
    }
    /* Display the progress bar */
    copyXPMArea(ORIG_PB_TX, ORIG_PB_TY, PROGRESSBAR_LEN, PROGRESSBAR_HEI,
		PROGRESSBAR_HPOS, PROGRESSBAR_VPOS);
    RedrawWindow();
  }

  ptr = strtok( 0, " ");
  pc->sizeOfAllMessages = atoi(ptr);
  /* -1 means first time */
  if (-1 != pc->sizechanged)
    pc->sizechanged = (old_sizeOfAllMessages != pc->sizeOfAllMessages);

  sprintf(pc->outBuf,"LAST\r\n");
  send(pc->s, pc->outBuf, strlen(pc->outBuf),0 );
  size = recv(pc->s,pc->inBuf,1024,0);
  pc->inBuf[size] = '\0';
#ifdef _DEBUG
  printf("  pop3CheckMail, last received buf (size=%d): [%s]\n",
	 size, pc->inBuf);
#endif
  memset(temp,0,1024);
  memcpy(temp,pc->inBuf,size);
  if( temp[0] != '+' ){
#ifdef _DEBUG
    printf("  Error Receiving LAST: [%s]\n", temp);
#endif
    pc->numOfUnreadMessages = pc->numOfMessages;
  }
  else {
    ptr = strtok(temp, " ");
    ptr = strtok( 0," ");
    pc->numOfUnreadMessages = pc->numOfMessages - atoi(ptr);
  }
  /* get the subject and From fields of numOfMessages mails */
  for (i = 0; i < pc->numOfMessages; i++) {
#ifdef _DEBUG
    printf("  ---- message %d ---\n", i);
#endif
    /* increment the progress bar by 1 message */
    progressbarpos = (int) ((i + 1) * (PROGRESSBAR_LEN - 2)) /
      pc->numOfMessages;
    copyXPMArea(ORIG_PB_BARX, ORIG_PB_BARY, progressbarpos, 2,
		PROGRESSBAR_HPOS + 1, PROGRESSBAR_VPOS + 2);
    RedrawWindow();
    /* reset the from and subject fields for num */
    memset(&pc->mails[i], 0, sizeof(t_mail));
    sprintf(pc->outBuf, "TOP %d 0\r\n", i + 1);
    send(pc->s, pc->outBuf, strlen(pc->outBuf), 0);
    pass = 0;
    go = 1;
    while (go && (0 < (size = recv(pc->s,pc->inBuf,4096,0)))) {
      memset(temp, 0, 4096);
      memcpy(temp, pc->inBuf, size);
      if (!pass && (temp[0] != '+')) {
	fprintf(stderr, "Error while getting TOP %d 0\n", i + 1);
	go = 0;
	/* must set the from and subject fields to correct value */
	strncpy(pc->mails[i].from, "::error::", 9);
	continue;
      }
      /* calculate the checksum */
      pc->mails[i].cksum += calculate_cksum(temp, size);
      pass++;
      go = parse_header(temp, size, i, pc);
    }
    pc->mails[i].todelete = 0;
    /* verify if this message is new or not */
    pc->mails[i].new = !is_in_old_mails(pc->mails[i].cksum, oldmails,
					old_numOfMessages);
#ifdef _DEBUG
    printf("  mess %d, cksum: %ld, new: %d\n",
	   i, pc->mails[i].cksum, pc->mails[i].new);
#endif
  }
  /* clear the old messages */
  if (oldmails) {
    free (oldmails);
  }
  return 1;
}

int	pop3WriteOneMail(int nb, int dest_fd, Pop3 pc)
{
  int	len;
  int	retour = 0;
  char	buf[4096];
  char	temp[4096];
  int	size;
  int	pass = 0;
  int	go = 1;
  char	*deb, *fin;
  int	theemailsize = 0;
  int	mustdisconnect = 0;


  if (NOT_CONNECTED == pc->connected) {
#ifdef _DEBUG
    printf("  pop3WriteOneMail not connected, connecting\n");
#endif
    if (pop3MakeConnection(pc,pc->popserver,pc->serverport) == -1){
      return (1);
    }
    if (pop3Login(pc, pc->username, pc->password) == -1) {
      return (1);
    }
    mustdisconnect = 1;
  }
  /* check the size */
#ifdef _DEBUG
  printf("  pc->maxdlsize = [%d]\n", pc->maxdlsize);
#endif
  if (-1 != pc->maxdlsize) {
    sprintf(pc->outBuf, "LIST %d\r\n", nb);
#ifdef _DEBUG
    printf("  pop3WriteOneMail: %s", pc->outBuf);
#endif
    send(pc->s, pc->outBuf, strlen(pc->outBuf), 0);
    size = recv(pc->s, pc->inBuf, 4096, 0);
#ifdef _DEBUG
    printf("  received %d bytes\n", size);
#endif
    memset(temp, 0, 4096);
    memcpy(temp, pc->inBuf, size);
#ifdef _DEBUG
    printf("  pop3WriteOneMail: answer [%s]\n", temp);
#endif
    if (temp[0] != '+') {
      sprintf(buf, TXT_ERROR, pc->mails[nb - 1].from,
	      pc->mails[nb - 1].subject, pc->outBuf);
      write (dest_fd, buf, strlen(buf));
#ifdef _DEBUG
      printf("  bad answer: [%s]\n", pc->inBuf);
#endif
      return (0);
    }
    /* go get the size of the email, strip +OK */
    for (deb = temp; *deb && !isspace(*deb); deb++);
    /* strip the spaces */
    for (; *deb && isspace(*deb); deb++);
    /* strip the email number */
    for (; *deb && !isspace(*deb); deb++);
    /* strip the last spaces */
    for (; *deb && isspace(*deb); deb++);
    /* go to the end of the size */
    for (fin = deb; *fin && !isspace(*fin); fin++);
    if (*deb && *fin) {
      memcpy(buf, deb, fin - deb);
      buf[fin - deb] = '\0';
      theemailsize = atoi(buf);
#ifdef _DEBUG
      printf("  email retrieved size: %d\n", theemailsize);
#endif
      if (theemailsize > pc->maxdlsize) {
	sprintf(buf, TXT_MESSAGETOOBIG, pc->mails[nb - 1].from,
		pc->mails[nb - 1].subject, theemailsize, pc->maxdlsize);
	write (dest_fd, buf, strlen(buf));
#ifdef _DEBUG
	printf("  email too big, return\n");
#endif
	return (0);
      }
    }
    /* else {}
       the command is screwed up, never mind, continue to download the mail */
  }


  sprintf(pc->outBuf, "RETR %d\r\n", nb);
#ifdef _DEBUG
  printf("  pop3WriteOneMail: %s", pc->outBuf);
#endif
  send(pc->s, pc->outBuf, strlen(pc->outBuf), 0);
  while (go && (size = recv(pc->s, pc->inBuf, 4096, 0))) {
#ifdef _DEBUG
    printf("  received %d bytes\n", size);
#endif
    memset(temp, 0, 4096);
    memcpy(temp, pc->inBuf, size);
    deb = temp;
    if (!pass) {
      if (temp[0] != '+') {
	sprintf(buf, TXT_ERROR, pc->mails[nb - 1].from,
		pc->mails[nb - 1].subject, pc->outBuf);
	write (dest_fd, buf, strlen(buf));
	retour = 1;
	go = 0;
	continue;
      }
#ifdef _DEBUG
      printf("  first pass, skeep RETR answer ([%s])\n", deb);
#endif
      /* skip the RETR answer, go to the end of the line; */
      for (;*deb && ('\n' != *deb); deb++);
      deb++;
    }
    /* check if these are the final bits of the message */
    fin = temp + size - 5;
#ifdef _DEBUG
    printf("  size: %d, fin: [%s]\n", size, fin);
#endif
    if (size >= 5) {
      if (!strncmp(fin, "\r\n.\r\n", 5)) {
	go = 0;
	/* don't write the final '.' into the message, skip it*/
	size -= 3;
#ifdef _DEBUG
	printf("  skeeping final '.', new size=%d\n", size);
#endif
      }
    }
    else
      go = 0;
#ifdef _DEBUG
    printf("  buffer (long: %d):\n[%s]\n", (temp + size) - deb, deb);
#endif
    len = 0;
    while (deb < (temp + size)) {
      len = write(dest_fd, deb, (temp + size) - deb);
      deb += len;
    }
#ifdef _DEBUG
    printf("  wrote %d bytes\n", len);
#endif
    pass++;
  }
  if (mustdisconnect) {
#ifdef _DEBUG
    printf("  pop3WriteOneMail disconnecting\n");
#endif
    pop3Quit(pc);
  }
#ifdef _DEBUG
  printf("  pop3WriteOneMail, return\n");
#endif
  return (retour);
}

int	pop3DeleteMail(int num, Pop3 pc)
{
  int	size;

  sprintf(pc->outBuf, "DELE %d\r\n", num);
#ifdef _DEBUG
  printf("  %s\n", pc->outBuf);
#endif
  send(pc->s, pc->outBuf, strlen(pc->outBuf), 0);
  size = recv(pc->s, pc->inBuf, 4096, 0);
  if ('+' != pc->inBuf[0]) {
    perror("error while deleting mail");
    return (1);
  }
  return (0);
}




int  pop3GetTotalNumberOfMessages( Pop3 pc ){
     if( pc != 0 )
         return pc->numOfMessages;
     return -1;
}

int  pop3GetNumberOfUnreadMessages( Pop3 pc ){
    if( pc != 0)
        return pc->numOfUnreadMessages;
    return -1;
}

int pop3Quit(Pop3 pc){
    int size;

#ifdef _DEBUG
    printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
    printf("XXXXXX  pop3Quit -------- disconneting XXXXXXXXX\n");
    printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
#endif
    if( pc->connected == NOT_CONNECTED )
        return -1;
    send(pc->s, "quit\r\n", 6,0 );
    size =recv(pc->s,&pc->inBuf,1024,0);
    pc->connected = NOT_CONNECTED;
    if(pc->s != 0)
        close(pc->s);
    return 0;

}



