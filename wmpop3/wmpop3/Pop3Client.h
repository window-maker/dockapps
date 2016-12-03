/* Author : Scott Holden ( scotth@thezone.net )
 *
 * Pop3 Email Checker
 *
 * Last Updated : Mar 20, 1999
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


typedef struct pop3_struct *Pop3;

Pop3 pop3Create(void);
int  pop3MakeConnection( Pop3 pc, char *serverName, int port);
int  pop3IsConnected(Pop3 pc);
int  pop3Login(Pop3 pc, char *name, char *pass);
int  pop3Quit(Pop3 pc);
int  pop3CheckMail(Pop3 pc);
int  pop3GetTotalNumberOfMessages( Pop3 pc );
int  pop3GetNumberOfUnreadMessages( Pop3 pc );

#endif
