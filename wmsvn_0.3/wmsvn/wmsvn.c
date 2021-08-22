/*

    WMSVN by shpelda@seznam.cz

    (C) 2005

    based on WMLOG by Tom Pycke

*/


#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include "../wmgeneral/wmgeneral.h"
#include "../wmgeneral/misc.h"
#include "wmsvn.h"
#include "wmlog-master.xpm"
#include "wmlog-mask.xbm"



int main(int argc, char *argv[]) 
{
	unsigned int i;//temporary wide used counter

	
	{//init pipe
		int compipe[2];
		if(pipe(compipe))
		{//pipe failure
			perror("Pipe creation fail.");
			exit(1);
		}	
		
		//replace stdin and stdout by pipe
		for(i=0; i<2; i++){
			if(dup2(compipe[i],i) == -1)
			{
				perror("Streams redirection fail.");
				exit(2);
			}
		}
	}
	
	
	{//main loop
		char _bufData[BUFFERLENGTH+1];
		char* bufData = _bufData;
		char** buffer = (char **)&bufData;
		const char* commands[CMDCOUNT] = {command1,command2,command3};
		const int zero[CMDCOUNT] = {0,0,0};	
		const int verticalPos[CMDCOUNT] = {21,35,48};
		int horizontalPos[CMDCOUNT] = 	{0,0,0};
		char displayString[CMDCOUNT][BUFFERLENGTH];
		int counter = DATA_REFRESH_RATIO;
		int dataChanged[CMDCOUNT] = {0,0,0};
		int displayLength[CMDCOUNT];
		

		openXwindow(argc, argv, wmlog_master_xpm, bitmap_bits, bitmap_width, bitmap_height);
		while(1){
				if(counter % DATA_REFRESH_RATIO == 0)
				{//refresh data
					bcopy( zero, dataChanged, sizeof(int)*CMDCOUNT);
					for(i=0; i< CMDCOUNT; i++)
					{
						executeCommand( commands[i], buffer, BUFFERLENGTH);
						removeEolns( buffer );
						trim( buffer );
						if( strcmp( displayString[i], *buffer) != 0)
						{//data changed
							strcpy( displayString[i], *buffer);
							displayLength[i] = strlen(displayString[i]);
						}
					}
				}
				if(counter % DISPLAY_REFRESH_RATIO == 0)
				{//refresh display
					for(i=0; i<CMDCOUNT; i++)
					{
							horizontalPos[i] = horizontalPos[i] % displayLength[i];
 
					    DisplayScroll (displayString[i], horizontalPos[i]++, verticalPos[i]);
//					    if ( horizontalPos[i] > strlen( displayString[i] ) )
	//						horizontalPos[i]= 1;
					}
					RedrawWindow();
				}
		counter++;
		usleep(10000);
		}
	}
	return 0;
}
	
void executeCommand( const char* cmd, char**buffer, const unsigned int length ){

	//execute external command
	if(system( cmd ) == -1)
	{
		perror("Cmd launch fail.");
		exit(3);
	}
	
	//get its output
	fprintf(stdout, "%c",'\0');
	fflush(stdout);
	unsigned int a;
	unsigned int counter = 0;
	while( (a = fgetc(stdin)) != EOF){
		if( a == '\0')
			break; 
		if( counter >= (length-1) )
			break;
		(*buffer)[counter++]=(char)a;
	}
	(*buffer)[counter]='\0';
}

//remove all \n from given string
void removeEolns(char** buffer)
{
	char* t = strchr( *buffer, '\n');
	while(t != NULL ){
		*t='.';
		t = strchr( *buffer, '\n');
	}
}

void trim( char **buffer)
{
	const int len = strlen( *buffer );
	int b=0;
	int e=len-1;
	for(b=0; b< len; b++){//beginning
		if( ! (isWhitespace((*buffer)[b])) )
			break;
	}
	for(e=len-1; e>0; e++){//trailing
		if(! (isWhitespace((*buffer)[e])) )	
			break;
	}
	if((b != 0)||(e!=len-1)){//create result
		const unsigned int resultLength= e + 1 - b;
		memmove(*buffer, &((*buffer)[b]), resultLength );
		(*buffer)[resultLength]	= '\0';
	}

}

//	return 1 if character is whitespace, 0 otherwise
short unsigned int isWhitespace( char c )
{
	unsigned int i=0;
	const char whitespace[]={' ','\t'};
	const int whitespaceLength=2;

	for(i=0; i< whitespaceLength; i++){
		if(c == whitespace[i])
			return 1;
	}
	return 0;
}


//scroll string in a window
void DisplayScroll (const char *label, const int pos, const int level)
{
	const int length = strlen( label );
	char display[10];
	const int copy = min( length-pos, 9 );
	

	if( length < 10){//fits there without scrolling
		copyXPMArea(4, 84, 55, 10, 4, 49);
		BlitString (label, 4, level);
		return;
	}
	
	strncpy( display, label + pos, copy );	
	display[copy] = ' ';
	if( copy < 9 ){
		memmove(&(display[copy+1]), label, 9 - copy - 1);
	}
	display[10]='\0';

	copyXPMArea(4, 84, 55, 10, 4, 49);
	BlitString (display, 4, level);
}

// Blits a string at given co-ordinates
void BlitString(const char *name, int x, int y)
{
    unsigned int   i;
    unsigned int   c;

	for (i=0; name[i]!=0 && name[i]!='\n' && i<9; i++)
	{

		c = toupper(name[i]); 

		if (c=='=')
			c=':';
		if (c >= 'A' && c <= 'Z' && c!=0)
		{   // its a uppercase letter
			c -= 'A';
			copyXPMArea(c * 6, 74, 6, 8, x, y);
			x += 6;
		}
		else if (c>='0' && c<='=')
		{   // its a number or symbol
			c -= '0';
			copyXPMArea(c * 6, 64, 6, 8, x, y);
			x += 6;
		}
		else 
		{   
			copyXPMArea(c*6, 13, 6, 8, x, y);
			x += 6;
		}   
	}
}

