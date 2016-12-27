
 /*
 *
 *  	wmdonkeymon 0.9 (C) 2002 Marcelo Burgos Morgade Cortizo (marcelomorgade@ig.com.br)
 * 
 *  		- Show status of edonkey downloads based on '*.part.met' files
 *  		
 * 
 *
 */





#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <X11/X.h>
#include <X11/xpm.h>
#include "../wmgeneral/wmgeneral.h"
#include "wmdonkeymon_master.xpm"
#include "wmdonkeymon_mask.xbm"
 

#define SLOT_SIZE 52
#define REF_RATE 5
#define VERSION "0.9"


int xpos[] = { 66, 71, 76, 81, 86, 91,	/* A B C D E F */
	66, 71, 76, 81, 86, 91,	/* G H I J K L */
	66, 71, 76, 81, 86, 91,	/* M N O P Q R */
	66, 71, 76, 81, 86, 91,	/* S T U V W X */
	66, 71, 76, 81, 86, 91,	/* Y Z / _ - . */
	96, 101,		/* 0 1 */
	96, 101,		/* 2 3 */
	96, 101,		/* 4 5 */
	96, 101,		/* 6 7 */
	96, 101
};				/* 8 9 */

int ypos[] = { 4, 4, 4, 4, 4, 4,
	9, 9, 9, 9, 9, 9,
	14, 14, 14, 14, 14, 14,
	19, 19, 19, 19, 19, 19,
	24, 24, 24, 24, 24, 24,
	4, 4,
	9, 9,
	14, 14,
	19, 19,
	24, 24
};


int but_stat;
XEvent Event;


struct downlinfo {
	char metname[30];
	char name[516];
	long int lastsize;
	long int size;
	char type[30];
	long int copied;
	int gappos[50][2];
	int firstgap;
	int t_miss;
	unsigned char gapnum;
	unsigned char status;
} files[4];

void usage();
void printversion();
void showString(char * buf, int row);
void pressEvent(XButtonEvent * xev);
void getStatus(int i,char * st);
void loadConfig(char * dir);
void splash();

int main(int argc, char *argv[])
{
	int i=0,debug=0,r,j=0,x=0,selected=-1;
	long int gi=0,gf=0;
        int colord=66,colorg=71;
        struct dirent **namelist;
        int ls=0,metnum=0;
        unsigned char buf[516] ;
	char * tmpdir = NULL;
        for (i=1; i<argc; i++) {
	        char *arg = argv[i];
                if (*arg=='-') {
                        switch (arg[1]) {
	                        case 't' :
					tmpdir = argv[i+1];
					printf("Using temp dir %s\n",tmpdir);
                                break;
	                        case 'w' :
				     if (argc>i+1){
					if (!strcasecmp(argv[i+1],"red")) colord=71;
					else if (!strcasecmp(argv[i+1],"blue")) colord=76;
					else if (!strcasecmp(argv[i+1],"yellow")) colord=81;
					else if (!strcasecmp(argv[i+1],"white")) colord=86;
					else if (!strcasecmp(argv[i+1],"cyan")) colord=91;
					else if (!strcasecmp(argv[i+1],"black")) colord=96;
					else if (!strcasecmp(argv[i+1],"blank")) colord=101;
					else printf("Invalid color %s\n",argv[i+1]);
				     }else {usage();exit(-1);}
                                break;
	                        case 'g' :
				     if (argc>i+1){
					if (!strcasecmp(argv[i+1],"green")) colorg=66;
					else if (!strcasecmp(argv[i+1],"blue")) colorg=76;
					else if (!strcasecmp(argv[i+1],"yellow")) colorg=81;
					else if (!strcasecmp(argv[i+1],"white")) colorg=86;
					else if (!strcasecmp(argv[i+1],"cyan")) colorg=91;
					else if (!strcasecmp(argv[i+1],"black")) colorg=96;
					else if (!strcasecmp(argv[i+1],"blank")) colorg=101;
					else printf("Invalid color %s\n",argv[i+1]);
				     }else {usage();exit(-1);}
                                break;
	                        case 'v' :
	                                printversion();
	                                exit(0);
                                break;
	                        case 'd' :
	                                debug=1;
	                                printf("Debuggin mode: \n");
                                break;
	                        default:
	                                usage();
	                                exit(0);
	                         break;
	               }
	       }
	}
	if (!tmpdir) { usage(); exit(-1);}
	
	openXwindow(argc, argv, wmdonkeymon_master_xpm, wmdonkeymon_mask_bits, wmdonkeymon_mask_width, wmdonkeymon_mask_height);
		copyXPMArea(5,60,52,54,5,3);
		RedrawWindow();
		splash();

	
	r = 0;
	while (1) {
         if (!r) {
	        FILE * met;
	        unsigned char type;
	        short int len=0,vlen=0;
	        int gaps=0, firstgap = 0x7fffffff,  miss=0, fsize=0,metcount=0;
	        long int fileSize=0, num =0;
	        char nvalue[516],value[516];
		j =0;
		metnum=0;	
		// Search for files in temp directory
		ls = scandir(tmpdir, &namelist, 0, alphasort);
		if (ls < 0){
		      printf("Can't find files in %s",tmpdir);
		      exit(-1);
		}
	        else {
		      while(ls-- && (metcount<4)) {
			    char * pt;
			    pt = strstr(namelist[ls]->d_name,".part.met");
			    if (pt && !strcmp(pt,".part.met")) {
				    if(debug)printf("File: %s\n",namelist[ls]->d_name);
				    strcpy(files[metcount].metname,namelist[ls]->d_name);
				    metcount++;
	  	    	    }
		           free(namelist[ls]);
		      }
		      free(namelist);
		}
		for (metnum=0; metnum < metcount; metnum++)	{
			i=0;
			sprintf(buf,"%s%s",tmpdir,files[metnum].metname);
			files[metnum].t_miss=0;
			if(debug)printf("opening %s\n",buf);
		        if ( (met = fopen(buf,"rb")) != NULL) {}
		        else {printf("Nada\n");};
		
		        // Version
		        i += fread(buf,1,1,met);
		        if(debug){ printf("Version: %x\n",buf[0]); }
		
			// Date ??
		        i += fread(buf,1,4,met);
		        if(debug) { printf("Date: %x %x %x %x \n",buf[0],buf[1],buf[2],buf[3]); }
		
			// Hash
		        i += fread(buf,1,16,met);
		        if(debug){ printf("Hash: ");  for (j=0;j<16;j++)  printf("%x ",buf[j]); 	 printf("\n");  }
	
		
			// Partial Hashes
		        i += fread(buf,1,2,met);
		        memcpy(&j,buf,2);
		        if(debug)  printf("Num of Hashes: %d\n",j);
		
			// Hashes
		        for (i=0;i<j;i++){
		              fread(buf,1,16,met);
		              if(debug){printf("Hash %d: ",i+1);    for (x=0;x<16;x++) printf("%x ",buf[x]);     printf("\n");}
		       }
		
		        // Num of Meta Tags
		        i = fread(buf,1,4,met);
		        memcpy(&num,buf,4);
		        if(debug){printf("Num of Meta Tags: %ld\n",num);}
		        x = 0;
		
			// Meta Tags
			for (i=0;i<num;i++){
				fread(&type,1,1,met);
				fread(&len,2,1,met);
				fread(nvalue,1,len,met);
				if (type==2){
				    // String Tag
				    fread(&vlen,1,2,met);
				    fread(value,1,vlen,met);
				    value[vlen] = '\0';
				    if (len==1){
					    // Special Tag
					     switch (nvalue[0]){
					       case 1:
						    strcpy(files[metnum].name,value);
						    if(debug){printf("File Name: %s\n",value);}
					       break;
					       case 3:
						    if(debug){printf("File Type: %s\n",value);}
						    strcpy(files[metnum].type,value);
					       break;
					       case 4:
						    if(debug){printf("File Format: %s\n",value);}
					       break;
					       case 18:
						    if(debug){printf("Temp file: %s\n",value);}
					       break;
		
					     }
		
				    } else if (debug)printf("Unknow String Tag %d: %s",nvalue[0],value);
				}else	if (type==3){
				    fread(&fsize,1,4,met);
				    if (len==1){
				    	// Special Tag
					    switch (nvalue[0]){
					       case 2:
					      	    if(debug)printf("File Size: %d KB\n",fsize/(1024));
						    fileSize = fsize;
					       break;
					       case 8:
					      	    if(debug)printf("Copied so Far: %d KB\n",fsize/(1024));
						    files[metnum].copied=fsize;
					       break;
					       case 19:
					      	   if(debug) printf("Priority: %d\n",fsize);
					       break;
					       case 20:
					      	    if(debug)printf("Status: %d\n",fsize);
						    files[metnum].status = fsize;
					       break;
					       default:
					       	   if(debug)printf("Unknow Tag %d: %d\n",nvalue[0],fsize);
					       break;
					    }
				    } else {
				    	    if (nvalue[0]==9){
						    nvalue[len]='\0';
						    if(debug)printf("gap %3s from %10d", &nvalue[1], fsize);
						    gaps = fsize;
						    if (gaps < firstgap) firstgap = gaps;
					    }else if (nvalue[0]==10){
						   miss = fsize - gaps;
						   gi = (long int)((SLOT_SIZE * (gaps/1024)) / (fileSize/1024));
						   gf = (long int)((SLOT_SIZE * (fsize/1024)) / (fileSize/1024));
						   if (gf>=gi){
						   	files[metnum].gappos[x][0] = gi;
							files[metnum].gappos[x][1] = gf;
							x++;
						   }
						   files[metnum].t_miss += miss;
					    	   if(debug)printf(" to %10d = %10d  Size(%d) Gaprel: %d-%d\n",fsize,miss,9728000,files[metnum].gappos[x][0],files[metnum].gappos[x][1]);
					    }
		                   }
	
			     }
		        }
	
		files[metnum].gapnum = x;	
		files[metnum].firstgap = firstgap;
		files[metnum].lastsize = files[metnum].copied;
		files[metnum].size = fileSize;
	
	// sort gaps
	/*	don'n needed
	 * 	for (i=0; i<x; i++)
			 for (j=0; j<x; j++) {
			 	if (files[0].gappos[i][0]<files[0].gappos[j][0]) {
					z = files[0].gappos[i][0];
					files[0].gappos[i][0] = files[0].gappos[j][0];
					files[0].gappos[j][0] = z;
					z = files[0].gappos[i][1];
					files[0].gappos[i][1] = files[0].gappos[j][1];
					files[0].gappos[j][1] = z;
				}
			 }
		if(debug)for (i=0; i<x; i++){	printf("[%d-%d]",files[0].gappos[i][0],files[0].gappos[i][1]);	}
	*/
	
		if(debug)printf("%d bytes = %.2f mb missing\n", files[metnum].t_miss, (double)files[metnum].t_miss/(1024*1024));
		if(debug)if (firstgap < 0x7fffffff) printf("first gap starts at %d (%d blocks are complete)\n", firstgap, firstgap/(9500*1024));
	
	
		fclose(met);
	     }
	}



		while (XPending(display)) {
			XNextEvent(display, &Event);
			switch (Event.type) {
			case Expose:
				RedrawWindow();
				break;
                        case ButtonPress:
                                but_stat = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
                        	break;
	                case ButtonRelease:
	                        i = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
	                        if (but_stat == i && but_stat >= 0) {
					if (selected>-1) selected=-1;
					else selected=i;
	                         }
				break;
			}
		}
		
		for (j=0;j<15;j++) DelMouseRegion(j);

		if (metnum==0){
			splash();
			showString("NO FILES",8);
			showString("FOUND IN",9);
			showString("TEMP DIR",10);
		}else if (selected>-1){
			char out[30];
			char unit[4] = " KMG";
			long int s,c;
			int sk=0,ck=0;
			copyXPMArea(5,60,52,54,5,3);
			AddMouseRegion(0,5,5,54,54);
			showString(files[selected].name,1);
			
			s=files[selected].size;
			while (s>1024){s/=1024;sk++;}
			c=files[selected].copied;
			while (c>1024){c/=1024;ck++;}
			
			sprintf(out,"%ld%c/%ld%c",c,unit[ck],s,unit[sk]);
			showString(out,4);

			sprintf(out,"%s",files[selected].type);
			showString(out,5);
		
			// **************
			// Donwload Rate
			// Don't work unless edonkey update met files more frequently
			// 
			//  s = (files[selected].copied) - (files[selected].lastsize);
			//  sprintf(out,"%ld B/S",(s/REF_RATE));
			//  showString(out,8);

			//************************
			//STATUS 
			//status tag is always "Looking..."      :(
			//
			//getStatus(files[selected].status,out);	
			//showString(out,9);
			sprintf(out,"%.3f%%", (( 1.0 *  files[selected].copied / files[selected].size))*100);
			showString(out,8);
			
			copyXPMArea(66,colord,52,5,5,11);
			for (i=0; i < files[selected].gapnum ; i++) {
				copyXPMArea(66,colorg,files[selected].gappos[i][1]-files[selected].gappos[i][0],5,files[selected].gappos[i][0]+5 ,11);
			}
		}else{
			copyXPMArea(5,60,52,54,5,3);
			for (j=0; j<metnum; j++){
			        showString(files[j].name,(j*2)+1+j);
				copyXPMArea(66,colord,52,5,5,(j+1)*10+(j*5) );
//				printf("top: %d\n",((j+1)*10+(j*5)));
				for (i=0; i < files[j].gapnum ; i++) {
					copyXPMArea(66,colorg,files[j].gappos[i][1]-files[j].gappos[i][0],5,files[j].gappos[i][0]+5 ,(j+1)*10+(j*5));
				}
				AddMouseRegion(j,5,(j+1)*5,52,(j+1)*10+(j*5)+5);
			}
		}
		RedrawWindow();

		sleep(1);
      		r++;
		if (r==REF_RATE) r=0;
      
    }
}



void
usage()
{

	printf("\nwmdonkeymon %s: \n",VERSION);
	printf("\nusage: wmdonkeymon -t tmpdir [-w color] [-g color]\n");
	printf("\t-t\t\tPath to edonkey temp dir.\n");
	printf("\t-w\t\tColor for downloaded parts\n");
	printf("\t-g\t\tColor for gaps\n");
	printf("\t-d\t\tDump a lot of debug messages\n");
	printf("\t-h\t\tDisplay help screen.\n");
	printf("\t\tColors: green,red,blue,yellow,white,cyan,black,blank\n");
}

void printversion(){
	printf("\nwmfsm version: \n");

}
void pressEvent(XButtonEvent * xev)
{
	return;
}

void strcaseup(char * str){
	int i=0;
	while(str[i]){
	    if (str[i]>='a' && str[i]<='z') {  str[i]-=32;}
	    i++;
	}
}

void showString(char * buf, int row){
	int i; 	
	strcaseup(buf);
	for (i=0; buf[i] && i<10;i++){
		if (buf[i]>='0' && buf[i]<='9') copyXPMArea(xpos[buf[i]-18],ypos[buf[i]-18],5,5,(i+1)*5,(row*5));
		else if((buf[i]>='A' && buf[i]<='Z')) copyXPMArea(xpos[buf[i]-65],ypos[buf[i]-65],5,5,(i+1)*5,(row*5));
		else if((buf[i]==' ')) copyXPMArea(66,44,5,5,(i+1)*5,(row*5));
		else if((buf[i]=='/')) copyXPMArea(76,24,5,5,(i+1)*5,(row*5)); 
		else if((buf[i]=='.')) copyXPMArea(91,24,5,5,(i+1)*5,(row*5)); 
		else if((buf[i]=='%')) copyXPMArea(106,24,5,5,(i+1)*5,(row*5)); 
	 	else copyXPMArea(xpos[28],ypos[28],5,5,(i+1)*5,(row*5));
	}
}

void getStatus(int i,char * st){
	if (i==0) strcpy(st,"Looking...");
	else if(i==1) strcpy(st,"Paused");
	else strcpy(st," ");
}

void splash(){
	// Splash
	int i=0;
	char * s= ".WMDONKEY.";
	while (i<4){
		RedrawWindow();
		showString(s,1);					     
		copyXPMArea(70,47,47,19,9,20);
		RedrawWindow();
		sleep(1);
		i++;
	}
	
}
