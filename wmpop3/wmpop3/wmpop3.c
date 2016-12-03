/*
 *  wmpop3.c
 *  pop3 mail checker. 
 *  by Scott Holden (scotth@thezone.net)
 *
 *  Contains some code from WMInet Dockable app.
 *             - BlitNum and BlitString
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

#include "../wmgeneral/misc.h"
#include "../wmgeneral/wmgeneral.h"
#include "Pop3Client.h"

#include "wmpop3.xpm"

char wminet_mask_bits[64*64];
int  wminet_mask_width = 64;
int  wminet_mask_height = 64;


#define WMPOP3_VERSION "0.5.6a"

#define CHAR_WIDTH 5
#define CHAR_HEIGHT 7
#define SEC_IN_MIN 60
#define YES 1
#define NO 0

char *ProgName;

char mailclient[32]   = "pine";
char password[32];
char username[32];
char popserver[128];
int  serverport       = 110;
int  mailCheckDelay   = 10;  /* default */
int  autoChecking     = YES; /* default */
int  newMessagesOnly  = YES; /* default */
char config_file[256] = "not-defined";

void usage(void);
void printversion(void);
void wmCheckMail_routine(int, char **);
int  readConfigFile( char *filename );

void BlitString(char *name, int x, int y);
void BlitNum(int num, int x, int y);


/********************************
 *           Main Program
 ********************************/
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

/*
 * wmCheckMail_routine : This function is used to set up the X display
 *           for wmpop3 and check for mail every n number of minutes.
 */

void wmCheckMail_routine(int argc, char **argv){

    int    mailWaiting    = 0;
    int    totalMessages  = 0;
    long   startTime      = 0;
    long   nextCheckTime  = 0;
    int    forcedCheck    = NO;
	int	   buttonStatus   =-1;
    int	   i;
	XEvent Event;
    Pop3   pc;

    if( !strcmp( config_file, "not-defined") )
        sprintf(config_file, "%s/.wmpop3rc", getenv("HOME"));

    if( readConfigFile(config_file) == -1){
        exit(0);
    }
    pc =pop3Create();     /* initialize Pop3 ADT */

    /* Set up timer for checking mail */
    startTime     = time(0);  
    nextCheckTime = 0; /* Make 0, so it checks for mail on start */

    
	createXBMfromXPM(wminet_mask_bits, wmpop3_xpm
               , wminet_mask_width, wminet_mask_height);
   
    openXwindow(argc, argv, wmpop3_xpm, wminet_mask_bits
               , wminet_mask_width, wminet_mask_height);

	AddMouseRegion(0, 18, 49, 45, 59 ); /* middle button */
	AddMouseRegion(1, 5 , 49, 17, 59 ); /* left button   */
	AddMouseRegion(2, 46, 49, 59, 59 ); /* right button  */
	AddMouseRegion(3, 2, 2, 58, 33);    /* main area     */
	AddMouseRegion(4, 5, 46, 58, 56);

    /* Check if Autochecking is on or off */
    if(autoChecking == NO ){
        copyXPMArea(67, 7 ,4 ,4 ,52 ,7 ); 
    }

    RedrawWindow();

    
    while (1){
        if( (((time(0) > nextCheckTime) || (nextCheckTime == 0))
                            && ( autoChecking == YES)) 
                            || (forcedCheck == YES)){ 
            if(pop3MakeConnection(pc,popserver,serverport) == -1){
                mailWaiting = -1;
            }
            else if( pop3Login(pc, username,password) == -1 ){
                mailWaiting = -1;
            }

            else if( (pop3CheckMail(pc)) == -1){
                mailWaiting = -1;
            }
            else{
                mailWaiting = pop3GetNumberOfUnreadMessages(pc);
                totalMessages = pop3GetTotalNumberOfMessages(pc);
            }
            if( forcedCheck == YES )
                forcedCheck = NO;
            pop3Quit(pc);
            nextCheckTime = time(0) + (mailCheckDelay * SEC_IN_MIN);
        }

		waitpid(0, NULL, WNOHANG);

        if( mailWaiting == -1 ){
            /* Error connecting to pop server */
            BlitString("::error::", 5, (11*(4-1)) + 5);
        }
        else{
            if( newMessagesOnly == YES ){
                /* Show unread messages only */
                BlitString("mesg :   ", 5, (11*(4-1)) + 5);
                BlitNum(mailWaiting, 45, (11*(4-1)) + 5);
            }else{
                /* Show unread Mesaages and Read messages */
                BlitString("    /    ", 5, (11*(4-1)) + 5);
                BlitNum(mailWaiting, 15, (11*(4-1)) + 5);
                BlitNum(totalMessages, 39, (11*(4-1)) + 5);
            }
        }
        if( mailWaiting == 0)
            copyXPMArea(72, 33, 45, 28, 4, 4 );
        else
            copyXPMArea(72, 2, 45, 28, 4, 4 );


        RedrawWindow();
        
        /* X Events */
        while (XPending(display)){
			XNextEvent(display, &Event);
            switch (Event.type)
            {
			case Expose:
				RedrawWindow();
				break;
			case DestroyNotify:
				XCloseDisplay(display);
				exit(0);
                break;
			case ButtonPress:
				i = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
				buttonStatus = i;
                if (buttonStatus == i && buttonStatus >= 0){
                    switch (buttonStatus){
                        case 0 : /* Middle button pressed */
                             copyXPMArea(128,29 ,27 ,8 ,18 ,49 );
                             nextCheckTime = 0;
                            break;
                        case 1 : /* Left button pressed */
                            copyXPMArea(128,16 ,11 ,8 ,6 ,49 );
                            break;
                        case 2: /* right button pressed */
                            copyXPMArea(128,2 ,11 ,8 ,46 ,49 );
                            /* change view on # of messages */
                            if( newMessagesOnly == YES )
                                newMessagesOnly = NO;
                            else
                                newMessagesOnly = YES;
                            break;
                        case 3:
                            execCommand(mailclient);
                            /* Recheck mail after 30 seconds,
                             * This gives time for the mail 
                             * browser to check the account */ 
                            nextCheckTime = time(0) + 30;
                            break;
					}
				}
                
				break;
			case ButtonRelease:
				i = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);

                if (buttonStatus == i && buttonStatus >= 0){
                    switch (buttonStatus){
                        case 0 :   /* Middle button */
                            copyXPMArea(128,39 ,27 ,8 ,18 ,49 );
                                forcedCheck = YES;
                            break;
                        case 1 :   /* Left Button */
                             copyXPMArea(144,16 ,11 ,8 ,5 ,49 );
                             if( autoChecking == YES){
                                 autoChecking = NO;
                                 /* Activate Red Led */
                                copyXPMArea(67, 7 ,4 ,4 ,52 ,7 ); 
                             }else{
                                 /* Activate Green Led */
                                 copyXPMArea(67, 2 ,4 ,4 ,52 ,7 ); 
                                 autoChecking = YES;
                             }
                             break;
                        case 2:  /* Right Button */
                            copyXPMArea(144,3 ,11 ,8 ,46 ,49 );
                            break;
                        case 3:
                            break;
					}
				}
				buttonStatus = -1;
				RedrawWindow();
				break;
			}
		}
		usleep(100000L);
	}
}


/*
 * usage : Prints proper command parameters of wmpop3.
 */
void usage(void)
{
    fprintf(stderr, "\nWMPop3 - Scott Holden <scotth@thezone.net>\n\n");
	fprintf(stderr, "usage:\n");
	fprintf(stderr, "    -display <display name>\n");
	fprintf(stderr, "    -geometry +XPOS+YPOS      initial window position\n");
    fprintf(stderr, "    -c <filename>             use specified config file\n");
    fprintf(stderr, "    -h                        this help screen\n");
	fprintf(stderr, "    -v                        print the version number\n");
	fprintf(stderr, "\n");
}

/*
 * printversion : This function is used to print the version info
 *                to standard output.
 */
void printversion(void)
{
	fprintf(stderr, "wmpop3 v%s\n", WMPOP3_VERSION);
}

// Blits a string at given co-ordinates
void BlitString(char *name, int x, int y)
{
    int		i;
	int		c;
    int		k;

	k = x;
    for (i=0; name[i]; i++)
    {
        c = toupper(name[i]); 
        if (c >= 'A' && c <= 'Z')
        {   /* its a letter */
			c -= 'A';
			copyXPMArea(c * 6, 74, 6, 8, k, y);
			k += 6;
        }
        else if( c == ' ' ){
			c = 14;
			copyXPMArea(73, 64, 6, 8, k, y);
			k += 6;
        }
        else if( c == '/' ){
			c = 14;
			copyXPMArea(68, 64, 6, 8, k, y);
			k += 6;
        }
        else
        {   /* its a number or symbol */
			c -= '0';
			copyXPMArea(c * 6, 64, 6, 8, k, y);
			k += 6;
		}
	}

}


 /* Blits number to give coordinates.. two 0's, right justified */


void BlitNum(int num, int x, int y)
{
    char buf[1024];
    int newx=x;

    if (num > 99)
    {
        newx -= CHAR_WIDTH;
    }

    if (num > 999)
    {
        newx -= CHAR_WIDTH;
    }

    sprintf(buf, "%02i", num);

    BlitString(buf, newx, y);
}
    

int  readConfigFile( char *filename ){
    char buf[256];
    char temp[32];
    char *ptr = 0;
    FILE *fp;

    if( (fp = fopen( filename, "r")) == 0 ){
        sprintf(config_file, "%s/.wmpop3rc", getenv("HOME"));
        printf("-Config file does not exit : %s\n",config_file);
        printf("+Trying to create new config file.\n");
        if((fp = fopen(config_file,"w")) == 0){
            printf("-Error creating new config file\n");
            return -1;
        }
        fprintf(fp,"# Replace all < > with appropriate data\n#\n");
        fprintf(fp,"popserver          < pop3 server name >\n");
        fprintf(fp,"port               110    # default port\n");
        fprintf(fp,"username           < pop3 login name  >\n");
        fprintf(fp,"password           < pop3 password   >\n");
        fprintf(fp,"autochecking       0      # 1 enables, 0 disables\n");
        fprintf(fp,"mailcheckdelay     10     # default mail check time in minutes\n");
        fprintf(fp,"viewallmessages    0      # 0 Shows both read and unread messages\n");
        fprintf(fp,"#                           and 1 shows only unread messages\n");
        fprintf(fp,"mailclient         pine  # default mail client\n");
        printf("+New config file created : ~/.wmpop3rc\n\n");
        printf("+ ~/.wmpop3rc must be configured before running wmpop3.\n");
        fclose(fp);
        return -1;
    }

    while( fgets(buf, 256, fp) != 0){

        ptr = strtok( buf," \n" );

        if( ( ptr != 0) && (ptr[0] != '#') ){
            if( !strcmp(ptr, "username") ){
                ptr = strtok( 0, " \n");
                if( ptr == 0){
                    printf("Invalid UserName.\n");
                    return -1;
                }
                strcpy(username, ptr); 
            }
            else if( !strcmp( ptr, "password") ){
                ptr = strtok( 0, " \n");
                if( ptr == 0){
                    printf("Invalid password.\n");
                    return -1;
                }
                strcpy(password, ptr); 
            }
            else if( !strcmp( ptr, "popserver") ){
                ptr = strtok( 0, " \n");
                if( ptr == 0){
                    printf("Invalid popserver address.\n");
                    return -1;
                }
                strcpy(popserver, ptr); 
            }
            else if( !strcmp( ptr, "mailclient") ){
                ptr = strtok( 0, " \n");
                if( ptr == 0){
                    printf("Invalid mailclient.\n");
                    return -1;
                }
                strcpy(mailclient, ptr); 
            }
            else if( !strcmp( ptr, "port") ){
                ptr = strtok( 0, " \n");
                if( ptr == 0){
                    printf("Invalid popserver port number.\n");
                    return -1;
                }
                if( sscanf(ptr,"%[0123456789]",temp) == 0)
                    serverport = 110;
                else
                    serverport = atoi(temp);
            }
            else if( !strcmp( ptr, "viewallmessages") ){
                ptr = strtok( 0, " \n");
                if( ptr == 0){
                    printf("Invalid number. ( viewallmessages )\n");
                    return -1;
                }
                if( sscanf(ptr,"%[0123456789]",temp) != 0)
                    newMessagesOnly = atoi(temp);
            }
            else if( !strcmp( ptr, "mailcheckdelay") ){
                ptr = strtok( 0, " \n");
                if( ptr == 0){
                    printf("Invalid delay time.\n");
                    return -1;
                }
                if( sscanf(ptr,"%[0123456789]",temp) != 0)
                    mailCheckDelay = atoi(temp);
            }
            else if( !strcmp( ptr, "autochecking") ){
                ptr = strtok( 0, " \n");
                if( ptr == 0){
                    printf("Invalid value.\n");
                    return -1;
                }
                if( sscanf(ptr,"%[0123456789]",temp) != 0)
                    autoChecking = atoi(temp);
            }
            else
                printf("Unknown indenifier : %s\n",ptr);
        }
    }
    fclose(fp);
    return 0;
}

