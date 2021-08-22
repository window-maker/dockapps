
/* 
	Code based on wmppp
	File ring.wav got from ktalkd

*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <unistd.h>

#include <sys/wait.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>


#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include "../wmgeneral/wmgeneral.h"
#include "../wmgeneral/misc.h"

#include "wmisdnmon-master.xpm"
#include "wmisdnmon-mask.xbm"


#define WMISDNMON_VERSION "0.9b"
#define LOG_SIZE 5
#define LCD_SIZE 9
#define FIELDS 6

#define FROM 0
#define MSN 1
#define SI 2
#define HORA 3
#define MES 4
#define DIA 5


char	*ProgName;
char	*host="127.0.0.1";
int	port=6105;
char    log[LOG_SIZE][FIELDS][LCD_SIZE+1];
int	cur_size;
int 	debug=0;
int blink_time = 0;
int blink_index = -1;
int dt_time=0;
int dt_index=-1;
int ch1=0,ch2=0;

/* functions */
void usage(void);
void printversion(void);
void wmisdn_routine(int, char **);
void addLog(char*);
void drawLog(void);
void drawDetails(void);
void clear(void);
void changedStatus(char *line);
void drawLeds();

int main(int argc, char *argv[]) {

	int		i;
	

	/* Parse Command Line */

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
			case 'v' :
				printversion();
				exit(0);
				break;
			case 'b' :
				debug=1;
				printf("Debuggin mode: \n");
				break;
			case 'i' :
				host = arg + 1;
				break;
			case 'p' :
				port = atoi(arg + 1);
				break;
			default:
				usage();
				exit(0);
				break;
			}
		}
	}

	wmisdn_routine(argc, argv);
	return 0;
}


void wmisdn_routine(int argc, char **argv) {

	unsigned int i;
	XEvent		Event;
	int but_stat = -1;

	int xpm_X = 0, xpm_Y = 0;
	char tmp[20][255];


	int isdnInfoSock;
	struct sockaddr_in isdnInfoAddr;
	char buffer[4096];
	int r,lines;

        fd_set rfds;
        struct timeval tv;
        int retval,c,l;


	cur_size = 0;
	
	
	isdnInfoSock = socket(AF_INET,SOCK_STREAM,0);
	isdnInfoAddr.sin_family = AF_INET;
	isdnInfoAddr.sin_port = htons(port);
	isdnInfoAddr.sin_addr.s_addr = inet_addr(host);
	bzero(&(isdnInfoAddr.sin_zero),8);

	
	openXwindow(argc, argv, wmisdnmon_master_xpm, wmisdnmon_mask_bits, wmisdnmon_mask_width, wmisdnmon_mask_height);
	
	while (connect(isdnInfoSock,(struct sockaddr * )&isdnInfoAddr, sizeof(isdnInfoAddr)) < 0){
        	copyXPMArea(0, 84, 56, 40, 4, 18 );
		RedrawWindowXY(0,0);
		usleep(5000000L);
	} 

	r=1;
	lines=0;
//	while (lines<6){
//		r = recv(isdnInfoSock, buffer, sizeof(buffer), 0);
//	        for (i=0; i<r; i++){
//		  if (buffer[i]=='\n') {
//			   lines++;
//		  }		  
//		}
//		if (r==0) exit(1);
//	}
	bzero(buffer,sizeof(buffer));








	/* add mouse region */
	AddMouseRegion(0, 5, 19, 59, 25);
	AddMouseRegion(1, 5, 27, 59, 33);
	AddMouseRegion(2, 5, 35, 59, 41);
	AddMouseRegion(3, 5, 43, 59, 49);
	AddMouseRegion(4, 5, 51, 59, 57);
	while (1) {
		bzero(buffer,sizeof(buffer));

		FD_ZERO(&rfds);
	        FD_SET(isdnInfoSock, &rfds);
                tv.tv_sec = 0;
                tv.tv_usec = 5000;
                retval = select(isdnInfoSock+1, &rfds, NULL, NULL, &tv);

                if (retval){
			r = recv(isdnInfoSock, buffer, sizeof(buffer), 0);
			if (debug) printf("Received %d bytes\n",r);
			if (debug) printf("Received from IsdnInfo: %s\n",buffer);
			c=0;
			l=0;
		        for (i=0; i<r && l<20; i++){
		           if (buffer[i]!='\n' && c<254){
		                        tmp[l][c]=buffer[i];
		                        c++;
	                   } else  {
                               tmp[l][c]='\0';
	                       l++;
                               c=0;
                           }
                        }		
			if (debug) printf("Parse OK\n");
		       for (i=0; i<l ; i++){
			        if (tmp[i][0]=='1')
				        addLog(tmp[i]);
				else if (tmp[i][0]=='0')
					changedStatus(tmp[i]);
		          
		       } 	
		       
		}

		clear();
		if (!dt_time) drawLog();
		else {
			dt_time--;
			drawDetails();
		}
	       
		RedrawWindowXY(0,0);
 
		while (XPending(display)) {
			XNextEvent(display, &Event);
			switch (Event.type) {
			case Expose:
				RedrawWindowXY(xpm_X, xpm_Y);
				break;
			case DestroyNotify:
				XCloseDisplay(display);
				exit(0);
				break;
			case ButtonPress:
				but_stat = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
				break;
			case ButtonRelease:
				i = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
				if (but_stat == i && but_stat >= 0 && i < cur_size) {
					dt_index=i;
					dt_time=12;
					drawDetails();
				}
				break;
			}
		}

		usleep(250000L);
	}
	close(isdnInfoSock);
	
}


/*******************************************************************************\
|* addLog									*|
\*******************************************************************************/

void addLog(char *n) {
	int j,i,c;
	char fields[FIELDS][30];
        char *aux;	


       	sscanf(n,"%s %s %s %s %s %s",fields[0],fields[1],fields[2],fields[3],fields[4],fields[5]);
      
        for (i = 0 ; i < FIELDS ; i++){	
	       	c = strlen(fields[i]);
       
		if (c>LCD_SIZE) aux = fields[i] + ( c - LCD_SIZE);
	        else aux = fields[i] + (i==FROM?1:0);
		
		if (cur_size == LOG_SIZE){
			for ( j=0; j<LOG_SIZE-1;  j++)
			       	strcpy(log[j][i],log[j+1][i]);
			
			strcpy(log[LOG_SIZE - 1][i], aux);
		} else {
			strcpy(log[cur_size][i], aux);
		}
	}
	if (cur_size == LOG_SIZE) blink_index = LOG_SIZE -1;
	else blink_index =  cur_size++;
	blink_time = 18;
	
	system("play /usr/share/ring.wav &");
}

/*******************************************************************************\
|* drawLog								       *|
\*******************************************************************************/

void drawLog() {
	int n,i,j,s;
       	copyXPMArea(5,19,54,40,5,19);
	for ( i=0; i<cur_size; i++){
	       j=strlen(log[i][FROM])-1;
	       s=j+1;
	       while(j>-1){
		n = log[i][FROM][j] - 48; //convert to int
        	copyXPMArea(1 + n*6, (blink_index==i&&blink_time%2==0?75:66), 6, 8, 5 + (j*6) +((LCD_SIZE-s)*6), 19 +(i*8));
		j--;
	       }
	}
	if (blink_index>-1){
        	copyXPMArea( 65 , (blink_time%2==0?4:12) ,19 , 8 , 37 , 6 );
		if (!blink_time--) {
			 blink_index=-1;
        		drawLeds();
		}
	}
	
}

/*******************************************************************************\
|* drawDetails								       *|
\*******************************************************************************/

void drawDetails(void) {
	int n,j,s;
	int i=dt_index;
	j=strlen(log[i][FROM])-1;
	s=j+1;
	while(j>-1){
		n = log[i][FROM][j] - 48; //convert to int
        	copyXPMArea(1 + n*6, 75, 6, 8, 5 + (j*6) +((LCD_SIZE-s)*6), 19);
		j--;
       }
       	copyXPMArea(57,95,37,8 ,15, 33 );
       	
	copyXPMArea(1 + (log[i][HORA][0]-48)*6 , 66, 6, 8,  17, 45 );
       	copyXPMArea(1 + (log[i][HORA][1]-48)*6 , 66, 6, 8, 23 , 45 );
       	copyXPMArea(60, 66, 3, 8, 29, 45);
       	copyXPMArea(1 + (log[i][HORA][3]-48)*6 , 66, 6, 8, 32, 45 );
       	copyXPMArea(1 + (log[i][HORA][4]-48)*6 , 66, 6, 8, 38, 45 );
       	copyXPMArea(60, 66, 3, 8, 44, 45);
       	copyXPMArea(1 + (log[i][HORA][6]-48)*6 , 66, 6, 8, 47, 45 );
       	copyXPMArea(1 + (log[i][HORA][7]-48)*6 , 66, 6, 8, 53 , 45 );
	
}

/*******************************************************************************\
|* changedStatus							       *|
\*******************************************************************************/

void drawLeds(){
	if (ch1 && ch2) copyXPMArea( 65 , 28 ,19 , 8 , 37 , 6 );
	else if (ch1) copyXPMArea( 65 , 4 ,19 , 8 , 37 , 6 );
	else if (ch2) copyXPMArea( 65 , 12 ,19 , 8 , 37 , 6 );
	else copyXPMArea( 65 , 20 ,19 , 8 , 37 , 6 );
}

/*******************************************************************************\
|* changedStatus							       *|
\*******************************************************************************/

void changedStatus(char *line){
	char cmd[50],info1[50],info2[50];
	if (strlen(line)>12){
		sscanf(line,"%s %s %s",cmd,info1,info2);
	        if (strcmp(cmd,"0usage:")==0) {
	              if (atoi(info1)==0)  ch1=0;
	              else   ch1=1;  
	              if (atoi(info2)==0)  ch2=0;
	              else   ch2=1;      
		      drawLeds();
		      if (debug) printf("Channel status changed: %d %d\n",ch1,ch2);
	       }
	}
}

/*******************************************************************************\
|* clear								       *|
\*******************************************************************************/

void clear(void) {
	int i,j;
	for (i=0;i<LCD_SIZE; i++)
		for (j=0;j<LOG_SIZE;j++)
       			copyXPMArea(58,84,6,8, 5+(i*6) ,19+(j*8));

}

/*******************************************************************************\
|* usage								       *|
\*******************************************************************************/

void usage(void) {

	printf("\nwmisdnmon - programming: Marcelo Morgade (LA_MANO)\n\n");
	printf("usage:\n");
	printf("\t-display <display name>\n");
	printf("\t-i ip.addr\tip number of isdninfo daemon (default: 127.0.0.1)\n");
	printf("\t-p port.number\tport of isdninfo daemon (default: 6105)\n");
	printf("\t-b\t\tprint console debug messages\n");
	printf("\t-v\t\tprint version\n");
	printf("\t-h\t\tthis help screen\n");
	printf("\n");
}

/*******************************************************************************\
|* printversion								       *|
\*******************************************************************************/

void printversion(void) {

	printf("wmisdnmon %s\n", WMISDNMON_VERSION);
	
}
