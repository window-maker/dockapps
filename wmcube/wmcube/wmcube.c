/*

 wmcube.c

 Version 1.0.1  (2015-02-19)
 Cezary M. Kruk <c.kruk@bigfoot.com>
 http://linux-bsd-unix.strefa.pl

 Contributions:
	A few patches, three new objects, and other updates by Doug Torrance
	<dtorrance@monmouthcollege.edu> (2015-02-19)

 Versions 0.98  (2000-10-23)
 Robert Kling   <robkli-8@student.luth.se>
 http://boombox.campus.luth.se/projects.php

 Contributions:
	-n option patch by Thorsten Jens <thodi@et-inf.fho-emden.de> (2000-05-12)
	Various bugfixes and optimizations by Jakob Borg (2000-05-13)
	Solaris Port by Dan Price <dp@rampant.org> (2000-07-16)
	OpenBSD Port by Brian Joseph Czapiga <rys@godsey.net> (2000-07-19)
	FreeBSD Port by Tai-hwa Liang <avatar@mmlab.cse.yzu.edu.tw> (2000-07-20)
	NetBSD Port by Jared Smolens <jsmolens+@andrew.cmu.edu> (2000-09-23)

 This software is licensed through the GNU General Public Licence.

 See http://www.BenSinclair.com/dockapp/ for more Window Maker dockapps.

 If you want to port wmcube to another OS the system specific code is
 sectioned the bottom of this file. See instructions there.

*/

#define CK_WMCUBE_VERSION "1.0.1"
#define CK_REV_YEAR "2014-2015"
#define CK_REV_DATE "2015-02-19"
#define DT_REV_YEAR "2015"
#define RK_WMCUBE_VERSION "0.98"
#define RK_REV_YEAR "2000"
#define RK_REV_DATE "2000-10-23"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>

#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <utmp.h>
#include <dirent.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#ifdef FREEBSD
#include <sys/resource.h>
#include <sys/sysctl.h>
#include <errno.h>
#endif

#include "../wmgeneral/wmgeneral.h"
#include "../wmgeneral/misc.h"

#include "wmcube.xpm"
#include "wmcubered.xpm"
#include "wmcubegreen.xpm"
#include "wmcubeblue.xpm"
#include "wmcubecyan.xpm"
#include "wmcubemagenta.xpm"
#include "wmcubeyellow.xpm"
char wmcube_mask_bits[64*64];
int  wmcube_mask_width = 64;
int  wmcube_mask_height = 64;

#define CHAR_WIDTH 5
#define CHAR_HEIGHT 7
#define PI 3.1415926535

//**** Graphics ***********************************

void putpixel(int x, int y, int c);
void line(int x1, int y1, int x2, int y2, int c);
void hline(int x1, int x2, int y, int c);
void triangle(int x1, int y1, int x2, int y2, int x3, int y3, int c);
void BlitString(char *name, int x, int y);
void BlitNum(int num, int x, int y);
void clearscr();
void draw();
void set_plugin();
void startup_seq();
int  red = 0;
int  green = 0;
int  blue = 0;

//**** 3d specific ********************************

void setupobj(char *filename) ;
void setUpAngles();
void rotate(int xang, int yang, int zang);
int  normal(float p1[], float p2[], float p3[]);
int  luminate(float p1[], float p2[], float p3[]);
void sortz(int nofelements);

//**** Application Management, I/O etc. ***********

void print_help();
int  loadobj(char *filename);
void mem_alloc_error(void *block);
int	 scan4objects(char *dir);
int  next_object();
void die();

//**** System specific functions ******************

int  init_calc_cpu();
int  calc_cpu_total();

//**** Global variables ***************************

int		xcenter, ycenter, zoff;
double		cost[361], sint[361];
double		acost[100];
float		**matrix;
float		**rmatrix;
int		**planes;
int		*plane_color;
int		*zorder;
int		*cline;
int		nofcoords, noflines, nofplanes;
char		*objects[1000];
int		nof_objects = 0;
int		show_load = 1;
int		use_nice = 1;
int		which_cpu = -1;
int		planesORlines = 1;
char		*pname;
int		font = 0;

int		but_stat = -1;

float	lum_vector[3] = { 0, 0, 100 };  // Lightsource vector

	char	obj_filename[256];
	char	*plugin = {""};

int main(int argc, char **argv)
{
	int		j, i = 0, rot_speed = 0, cpu_usage = 0, rot_step = 1;
	long	screen_speed = 10000;    // microseconds between screen updates (approx.)
	long	cpu_update = 490000;     // microseconds between cpu update (approx.)
	int		loop = 0;
	XEvent	Event;

	char	*rotdiv = {"25"};
	char	*rotstep = {"1"};
	int		rot;
	int 	cube_color = 1;
	int 	c = 0;
	int		invert_speed = 0;

	pname = strrchr(argv[0], '/');
	if (pname == NULL) pname = argv[0];

	srand((unsigned)time(NULL));
	opterr = 0;
	
	while ((c = getopt (argc, argv, "o:d:r:c:fnbipRGBh")) != -1) {
		switch (c)
		{
			case 'o':
				plugin = optarg;
				break;
			case 'd':
				rotstep = optarg;
				break;
			case 'r':
				rotdiv = optarg;
				break;
			case 'c':
				which_cpu = atoi(optarg);
				break;
			case 'f':
				font = 40;
				break;
			case 'n':
				use_nice = 0;
				break;
			case 'b':
				cube_color = 2;
				break;
			case 'i':
				invert_speed = 1;
				break;
			case 'p':
				show_load = 0;
				break;
			case 'R':
				red = 1;
				break;
			case 'G':
				green = 1;
				break;
			case 'B':
				blue = 1;
				break;
			case 'h':
				print_help();
				return 1;
			case '?':
					print_help();
					return 1;
			default:
				abort();
		}
	}

	set_plugin();

	/*
	 * Validate that wmcube can run on this system given the parameters,
	 * then setup the statistics gathering subsystem.
	 */
	 
	if (init_calc_cpu() != 0) die();

	/*
	 * Scan directory for .wmc files and choose one randomly. If the user 
	 * specified a particular file, load that one.
	 */

#ifndef SOLARIS  // scan4objects doesnt work on Solaris, load object immediatly
	scan4objects(obj_filename);
	
	if (nof_objects != 0)
		next_object();
	else
#endif
		setupobj(obj_filename);
	 	
	/*
	 * Various initializion stuff for the 3d-engine etc.
	 */

	setUpAngles();

	rot = atoi(rotdiv);
	if ((rot >= 1) && (rot <=100)) ; else rot = 25;

	rot_step = atoi(rotstep);
	if (rot_step < 0) rot_step = -rot_step;	

	if (calc_cpu_total() == -1)	die();

	cpu_update /= screen_speed;

	if ((red == 0) && (green == 0) && (blue == 0)) {
		createXBMfromXPM(wmcube_mask_bits, wmcube_xpm, wmcube_mask_width, wmcube_mask_height);
		openXwindow(argc, argv, wmcube_xpm, wmcube_mask_bits, wmcube_mask_width, wmcube_mask_height);
	} else if ((red == 1) && (green == 0) && (blue == 0)) {
		createXBMfromXPM(wmcube_mask_bits, wmcubered_xpm, wmcube_mask_width, wmcube_mask_height);
		openXwindow(argc, argv, wmcubered_xpm, wmcube_mask_bits, wmcube_mask_width, wmcube_mask_height);
	} else if ((red == 0) && (green == 1) && (blue == 0)) {
		createXBMfromXPM(wmcube_mask_bits, wmcubegreen_xpm, wmcube_mask_width, wmcube_mask_height);
		openXwindow(argc, argv, wmcubegreen_xpm, wmcube_mask_bits, wmcube_mask_width, wmcube_mask_height);
	} else if ((red == 0) && (green == 0) && (blue == 1)) {
		createXBMfromXPM(wmcube_mask_bits, wmcubeblue_xpm, wmcube_mask_width, wmcube_mask_height);
		openXwindow(argc, argv, wmcubeblue_xpm, wmcube_mask_bits, wmcube_mask_width, wmcube_mask_height);
	} else if ((red == 1) && (green == 1) && (blue == 0)) {
		createXBMfromXPM(wmcube_mask_bits, wmcubeyellow_xpm, wmcube_mask_width, wmcube_mask_height);
		openXwindow(argc, argv, wmcubeyellow_xpm, wmcube_mask_bits, wmcube_mask_width, wmcube_mask_height);
	} else if ((red == 0) && (green == 1) && (blue == 1)) {
		createXBMfromXPM(wmcube_mask_bits, wmcubecyan_xpm, wmcube_mask_width, wmcube_mask_height);
		openXwindow(argc, argv, wmcubecyan_xpm, wmcube_mask_bits, wmcube_mask_width, wmcube_mask_height);
	} else if ((red == 1) && (green == 0) && (blue == 1)) {
		createXBMfromXPM(wmcube_mask_bits, wmcubemagenta_xpm, wmcube_mask_width, wmcube_mask_height);
		openXwindow(argc, argv, wmcubemagenta_xpm, wmcube_mask_bits, wmcube_mask_width, wmcube_mask_height);
	} else if ((red == 1) && (green == 1) && (blue == 1)) {
		createXBMfromXPM(wmcube_mask_bits, wmcube_xpm, wmcube_mask_width, wmcube_mask_height);
		openXwindow(argc, argv, wmcube_xpm, wmcube_mask_bits, wmcube_mask_width, wmcube_mask_height);
	}

	startup_seq();
	
	if (calc_cpu_total() == -1) die();

	// index, left, top, right, bottom
	AddMouseRegion(1,  45,  45, 58, 58);	  // + Zoom In
	AddMouseRegion(5,  5, 45, 20, 58);	  // - Zoom Out
	AddMouseRegion(3,  21, 45, 45, 58);	  // Show cpu-load
	AddMouseRegion(2,  5, 5, 55, 45);	  // Everywhere else (almost) to change object

	/*
	 * Main loop begins here
	 */

	while (1)
	{
		i = (i+rot_speed+rot_step) % 360;
		
		clearscr();

		rotate(i,i,i);
		draw(cube_color);

		if (show_load) {
			BlitNum(cpu_usage,24,49);
			BlitString("%",38,49);
		}

		RedrawWindow();
		
		if (loop++ == cpu_update) { 
			loop = 0;

			/*
			 * call calc_cpu_total to update statistics.  If some
			 * sort of bad event occurs, calc_cpu_total will return
			 * -1, and we exit.
			 */
			 
			if ((cpu_usage = calc_cpu_total()) == -1) {
				die();
			}
			rot_speed =  abs( invert_speed*(100 / rot) - cpu_usage / rot);
		}
		
      // X Events
		
		while (XPending(display))
		{

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
					j = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
					but_stat = j;
					
					break;
				case ButtonRelease:
					j = CheckMouseRegion(Event.xbutton.x, Event.xbutton.y);
					
					switch(j)
					{
						case 1:
							if (zoff > 750) {
								BlitString("ö",48,48);
								RedrawWindow();
								zoff -= 150;
							}
							break;

						case 2:
							next_object();
							break;

						case 3:
							if (show_load == 1) show_load = 0; else show_load = 1;						
							ycenter = 15 - 2*show_load;
							break;

						case 5:
							BlitString("ä",11,49);
							RedrawWindow();
							zoff += 150;
							break;
					}
			}	
			break;
		}
		usleep(screen_speed);
	}

	/*
	 * Free up memory used by the object (dirty...)
	 */

	free(matrix);
	free(rmatrix);
	free(cline);	

	return 1;
}

//**** Graphics ***********************************
//*************************************************

void set_plugin()
{
	strcpy(obj_filename,plugin);
	if (access (obj_filename, R_OK) == -1) {
		strcpy(obj_filename,"/usr/share/wmcube/");
		strcat(obj_filename,plugin);
	}
}

void startup_seq() 
{
	char *tmp = malloc(32);
	int oldzoff = 3600;

	sprintf(tmp,"%s",CK_WMCUBE_VERSION);
	
	RedrawWindow();
	BlitString("WMCUBE",13,22);
	BlitString(tmp,15,31);
	RedrawWindow();
	RedrawWindow();
	usleep(3000000);
	RedrawWindow();
	
	zoff = 1200;

	for (;zoff < oldzoff; zoff += 35)
	{
		rotate((zoff-1200)/8,0,0);
		clearscr();
		BlitString("WMCUBE",13,22);
		BlitString(tmp,15,31);
		draw(1);
		RedrawWindow();
		usleep(9000);
	}

	free(tmp);
	zoff = 3600;
}	

void draw(int color)
{
	int i;

	if (planesORlines) {

		sortz(nofplanes);
		for (i = 0; i < nofplanes; i++) {
	    	if (normal(rmatrix[planes[zorder[i]][0]], rmatrix[planes[zorder[i]][1]], rmatrix[planes[zorder[i]][2]]) > 0) {	
			
			triangle(xcenter+rmatrix[planes[zorder[i]][0]][0], ycenter+rmatrix[planes[zorder[i]][0]][1],
				 	 xcenter+rmatrix[planes[zorder[i]][1]][0], ycenter+rmatrix[planes[zorder[i]][1]][1],
					 xcenter+rmatrix[planes[zorder[i]][2]][0], ycenter+rmatrix[planes[zorder[i]][2]][1], plane_color[zorder[i]]);
			}
		}
		
	} else {
		for (i = 0; i < noflines; i += 2)
			line(xcenter+rmatrix[cline[i  ]-1][0], ycenter+rmatrix[cline[i  ]-1][1],
		 	 	 xcenter+rmatrix[cline[i+1]-1][0], ycenter+rmatrix[cline[i+1]-1][1],color);
	}
}

void putpixel(int x, int y,int c)
{
	if ((x > 4) && (x < 59) && (y > 4) && (y < 59))
		copyXPMArea(160-c,0,1,1,x,y);
}

void hline(int x1, int x2, int y, int c)
{
	if ((y > 4) && (y < 59)) {
		if (x1 <= 4) x1 = 5;  else if (x1 > 57) return; 
		if (x2 > 57) x2 = 57; else if (x2 <= 4) return; 

		copyXPMArea(105, 56+c + 9*(c/18), x2-x1, 1, x1, y);
	}
}

void triangle(int x1, int y1, int x2, int y2, int x3, int y3, int c) // Draws a filled triangle
{
    int k,k2,x,x_2,i, tmp1; 

    int x1t, x2t;
			    
    if (y3<y2)
    {
		tmp1=y2;
		y2=y3;
		y3=tmp1;
		tmp1=x2;
		x2=x3;
		x3=tmp1;
    }

    if (y2<y1)
    {
		tmp1=y1;
		y1=y2;
		y2=tmp1;
		tmp1=x1;
		x1=x2;
		x2=tmp1;
    }

    if (y3<y2)
    {
		tmp1=y2;
		y2=y3;
		y3=tmp1;
		tmp1=x2;
		x2=x3;
		x3=tmp1;
    }

    if (y1!=y3)	k=((x1-x3) << 6) / (y1-y3);
    else k=(x1-x3) << 6;

	if (y1!=y2)	k2=((x1-x2) << 6) / (y1-y2);
    else k2=(x1-x2) << 6;

    x=x1 << 6;
    x_2=x;
    i=y1;

    if (i!=y2)
	do
	{
	    x+=k;
	    x_2+=k2;
	    i++;
	  
		if ((x1t = x >> 6) >  (x2t = x_2 >> 6))
			hline(x2t, x1t, i, c);
		else
			hline(x1t, x2t, i, c);
	}
	while (i!=y2);

    if (i==y3) return;

    if (y2!=y3) k2=((x2-x3) << 6) / (y2-y3);
    else k2=((x2-x3) << 6);

    x_2=x2 << 6;
    i=y2;
    do
    {
		x+=k;
		x_2+=k2;
		i++;

		if ((x1t = x >> 6) >  (x2t = x_2 >> 6))
			hline(x2t, x1t, i, c);
		else
			hline(x1t, x2t, i, c);
    }
    while (i!=y3);
}

void clearscr()
{
	copyXPMArea(78,0,56,56,4,4);
}

// Blits a string at given co-ordinates
void BlitString(char *name, int x, int y)
{
	int		i;
	int		c;
	int		k;
	
	k = x;
	
	copyXPMArea(73,64,1,8,k-1,y);
	
	for (i=0; name[i]; i++)
	{  
		c = toupper(name[i]); 
      //printf("%c",c);
		
		if (c >= '0' && c<= '9')
		{   // its a number or symbol
			c -= '0';
			if ( k > -2) copyXPMArea(c * 6, 64 + font, 6, 9, k, y);
			k += 6;
		} else
		if (c >= 'A' && c <= 'Z')
		{   // its a letter
			c -= 'A';
			if ( k > -2) copyXPMArea(c * 6, 74 + font, 6, 9, k, y);
			k += 6;
		} else
		if (c == '.') {
			if ( k > -2) copyXPMArea(60, 64 + font, 6, 9, k, y);
			k += 6;
		} else
		if (c == '%') {
			if ( k > -2) copyXPMArea(66, 64 + font, 6, 9, k, y);
			k += 6;
		} else
		if (c == 246) {
			if ( k > -2) copyXPMArea(0, 84 + font, 6, 9, k, y);
			k += 6;
		} else
		if (c == 228) {
			if ( k > -2) copyXPMArea(6, 84 + font, 6, 9, k, y);
			k += 6;
		} else
		{   // its a blank or something else
			if ( k > -2) copyXPMArea(73, 64 + font, 6, 9, k, y);
			k += 6;
		}
		if (k >= 58) break;
	}
	copyXPMArea(73, 64 + font, 1, 9, k, y);
	
}

// Blits number to given coordinates.. two 0's, right justified

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

void line(int x1, int y1, int x2, int y2, int c)
{
	int i, deltax, deltay, numpixels,
	d, dinc1, dinc2,
	x, xinc1, xinc2,
	y, yinc1, yinc2;
	
	deltax = abs(x2 - x1);
	deltay = abs(y2 - y1);
	if (deltax >= deltay)
	{
		numpixels = deltax + 1;
		d = (deltay << 1) - deltax;
		dinc1 = deltay << 1;
		dinc2 = (deltay - deltax) << 1;
		xinc1 = 1;
		xinc2 = 1;
		yinc1 = 0;
		yinc2 = 1;
	}
	else
	{
		numpixels = deltay + 1;
		d = (deltax << 1) - deltay;
		dinc1 = deltax << 1;
		dinc2 = (deltax - deltay) << 1;
		xinc1 = 0;
		xinc2 = 1;
		yinc1 = 1;
		yinc2 = 1;
	}
	if (x1 > x2)
	{
		xinc1 = - xinc1;
		xinc2 = - xinc2;
	}
	if (y1 > y2)
	{
		yinc1 = - yinc1;
		yinc2 = - yinc2;
	}
	x = x1;
	y = y1;
	for (i=1; i<numpixels; i++)
	{
		putpixel(x, y, c);
		if (d < 0)
		{
			d = d + dinc1;
			x = x + xinc1;
			y = y + yinc1;
		}
		else
		{
			d = d + dinc2;
			x = x + xinc2;
			y = y + yinc2;
		}
	}
}


//**** 3d specific ********************************
//*************************************************

void rotate(int xang, int yang, int zang)
{
	float tx, ty, tz;
	int i;
	
	for (i = 0; i < nofcoords; i++) 
	{
		tx = cost[yang]*matrix[i][0]-sint[yang]*matrix[i][2];
		tz = sint[yang]*matrix[i][0]+cost[yang]*matrix[i][2];
		ty = cost[zang]*matrix[i][1]-sint[zang]*tx;

		rmatrix[i][0] = (cost[zang]*tx+sint[zang]*matrix[i][1]);
		rmatrix[i][1] = (sint[xang]*tz+cost[xang]*ty);
		rmatrix[i][2] = (cost[xang]*tz-sint[xang]*ty);
	}

	if (planesORlines)
		for (i = 0; i < nofplanes; i++)
			if (normal(rmatrix[planes[i][0]], rmatrix[planes[i][1]], rmatrix[planes[i][2]]) > 0)
				plane_color[i] = luminate(rmatrix[planes[i][0]], rmatrix[planes[i][1]], rmatrix[planes[i][2]]);

	for (i = 0; i < nofcoords; i++) {	
		// Perspective correcting lines...
		rmatrix[i][0] = (rmatrix[i][0] *256) / (2*rmatrix[i][2] - zoff) + xcenter;
		rmatrix[i][1] = (rmatrix[i][1] *256) / (2*rmatrix[i][2] - zoff) + ycenter;
	}
}


void sortz(int nofelements) {   // Insertion-sort the planes in increasing z-distance

	int i, j, k;
	float key;
	float temparr[nofelements];

	for (i = 0; i < nofelements; i++) 
    {
   		zorder[i] = i;
   		temparr[i] = rmatrix[planes[i][0]][2]+rmatrix[planes[i][1]][2]+rmatrix[planes[i][2]][2];
    }

	for (j = 1; j < nofelements; j++) {
		
		key = temparr[j];
		k = zorder[j];
		i = j - 1;
			
		while ((i > -1) && (temparr[i] > key)) {
			temparr[i+1] = temparr[i];
			zorder[i+1] = zorder[i];
			i--;
		}

		zorder[i+1] = k;										
		temparr[i+1] = key;
	}
}

int normal(float p1[], float p2[], float p3[])
{
		return ((p1[0]-p3[0])*(p2[1]-p3[1])-(p2[0]-p3[0])*(p1[1]-p3[1]));
}

int luminate(float p1[], float p2[], float p3[])
{
      double x1 = (float)(p1[0]-p3[0]), y1 = (float)(p1[1]-p3[1]),  z1 = (float)(p1[2]-p3[2]);
      double x2 = (float)(p2[0]-p3[0]), y2 = (float)(p2[1]-p3[1]),  z2 = (float)(p2[2]-p3[2]);
      double nx = y1*z2-y2*z1, ny =-(x1*z2-x2*z1),nz = x1*y2-y1*x2;

	  return (int)(53 * (acost[(int)(50 + 50*(nx*lum_vector[0]+ny*lum_vector[1]+nz*lum_vector[2])/
				  					(sqrt(nx*nx+ny*ny+nz*nz)*
				   					sqrt(lum_vector[0]*lum_vector[0]+lum_vector[1]*lum_vector[1]+
				   					lum_vector[2]*lum_vector[2])))] / PI));	

	  // Do I smell optimization? :-)				   						
}

void setUpAngles()
{
	int i;
	for (i = 0; i < 361; i++) {
		cost[i] = cos((double)i*(2*PI/(double)360));
		sint[i] = sin((double)i*(2*PI/(double)360));
	}

	for (i = 0; i < 100; i++) acost[i] = acos((double)(-50+i)/50);
}

void setupobj(char *filename) 
{
	int i, j = 0;
	int biggest = 0;
	float scale = 1;

	xcenter = 16;
	ycenter = 15 - 2*show_load;
	
	if (strcmp(filename,"") != 0)
		loadobj(filename);
	else
	{
		nofcoords = 8;
		noflines = 24;
		nofplanes = 12;
		planesORlines = 1;

		matrix = 		(float **)malloc(nofcoords*sizeof(float *)); mem_alloc_error(matrix);
		planes = 		(int **)malloc(nofplanes*sizeof(int *)); mem_alloc_error(planes);
		plane_color =	(int *)malloc(nofplanes*sizeof(int)); mem_alloc_error(plane_color);
		zorder = 		(int *)malloc(nofplanes*sizeof(int)); mem_alloc_error(zorder);

		for (i = 0; i < nofplanes; i++) zorder[i] = i;
		
		for (i = 0; i < nofcoords; i++) { 
			matrix[i] = (float *)malloc(3*sizeof(float));
			mem_alloc_error(matrix[i]);
		}

		for (i = 0; i < nofplanes; i++) { 
			planes[i] = (int *)malloc(3*sizeof(int));
			mem_alloc_error(planes[i]);
		}

		cline = (int *)malloc((noflines+1)*sizeof(int)); mem_alloc_error(cline);
	
		matrix[0][0] = -180; matrix[0][1] = -180; matrix[0][2] =  180; // 0
		matrix[1][0] =  180; matrix[1][1] = -180; matrix[1][2] =  180; // 1
		matrix[2][0] =  180; matrix[2][1] =  180; matrix[2][2] =  180; // 2
		matrix[3][0] = -180; matrix[3][1] =  180; matrix[3][2] =  180; // 3
		matrix[4][0] = -180; matrix[4][1] = -180; matrix[4][2] = -180; // 4
		matrix[5][0] =  180; matrix[5][1] = -180; matrix[5][2] = -180; // 5
		matrix[6][0] =  180; matrix[6][1] =  180; matrix[6][2] = -180; // 6
		matrix[7][0] = -180; matrix[7][1] =  180; matrix[7][2] = -180; // 7

		cline[0] = 1;  cline[1] = 2;
		cline[2] = 2;  cline[3] = 3;
		cline[4] = 3;  cline[5] = 4;
		cline[6] = 4;  cline[7] = 1;
		cline[8] = 5;  cline[9] = 6;
		cline[10] = 6; cline[11] = 7;
		cline[12] = 7; cline[13] = 8;
		cline[14] = 8; cline[15] = 5;
		cline[16] = 1; cline[17] = 5;
		cline[18] = 2; cline[19] = 6;
		cline[20] = 3; cline[21] = 7;
		cline[22] = 4; cline[23] = 8;

		planes[0][0] = 0; planes[0][1] = 1; planes[0][2] = 3;
		planes[1][0] = 1; planes[1][1] = 2; planes[1][2] = 3;		
		planes[2][0] = 1; planes[2][1] = 5; planes[2][2] = 6;
		planes[3][0] = 1; planes[3][1] = 6; planes[3][2] = 2;

		planes[4][0] = 4; planes[4][1] = 0; planes[4][2] = 3;
		planes[5][0] = 4; planes[5][1] = 3; planes[5][2] = 7;		
		planes[6][0] = 3; planes[6][1] = 2; planes[6][2] = 7;
		planes[7][0] = 7; planes[7][1] = 2; planes[7][2] = 6;

		planes[8][0] = 4;  planes[8][1] = 1;  planes[8][2] = 0;
		planes[9][0] = 4;  planes[9][1] = 5;  planes[9][2] = 1;		
		planes[10][0] = 5; planes[10][1] = 4; planes[10][2] = 7;
		planes[11][0] = 5; planes[11][1] = 7; planes[11][2] = 6;
	}

	rmatrix = (float **)realloc(rmatrix,nofcoords*sizeof(float *)); mem_alloc_error(rmatrix);
	for (i = 0; i < nofcoords; i++) { 
		rmatrix[i] = (float *)malloc(3*sizeof(float));
		mem_alloc_error(rmatrix[i]);
	}

	/*
	 * Find the longest discance between all coordinates relative to the origin
	 */

	for (i = 0; i < nofcoords; i++) {
		j = (int)sqrt((pow(matrix[i][0],2)+pow(matrix[i][1],2)+pow(matrix[i][2],2)));
		if (j > biggest) biggest = j;
	}

	/*
	 * Scale every coordinate using the calculated factor
	 */

	scale = 280 / (float)biggest;	

	for (i = 0; i < nofcoords; i++) {
		matrix[i][0] *= scale;
		matrix[i][1] *= scale;
		matrix[i][2] *= scale;
	}
}




//**** Application Management, I/O etc. ***********
//*************************************************

void print_help() {
	printf("\nwmcube %s  (C) %s Cezary M. Kruk (%s)\n", CK_WMCUBE_VERSION, CK_REV_YEAR, CK_REV_DATE);
	printf("              (C)      %s Doug Torrance\n", DT_REV_YEAR);
	printf("wmCube %s   (C)      %s Robert Kling   (%s)\n\n", RK_WMCUBE_VERSION, RK_REV_YEAR, RK_REV_DATE);
	
	printf("  Usage: wmcube [-o <filename>] [-drcfnbipRGBh]\n\n");
	
	printf("  -o <filename> : load external 3D object (plugin).\n\n");
	printf("  -d x : rotate x degrees/step when the CPU is idle (default 1).\n");
	printf("  -r x : rotate 1 degree faster every x percent of the CPU usage (default 25).\n");

#ifdef LINUX
	printf("  -c x : which CPU (0, 1, 2...) to monitor (default average over all).\n");
	printf("  -f   : use smooth font (default OFF).\n");
	printf("  -n   : exclude \"nice\" processes (default OFF).\n");
#endif

#ifdef SOLARIS
	printf("  -c x : which CPU (0, 1, 2...) to monitor (default average over all).\n");
	printf("  -f   : use smooth font (default OFF).\n");
#endif

#ifdef FREEBSD
	printf("  -f   : use smooth font (default OFF).\n");
	printf("  -n   : exclude \"nice\" processes (default OFF).\n");
#endif

#ifdef NETBSD
	printf("  -f   : use smooth font (default OFF).\n");
	printf("  -n   : exclude \"nice\" processes (default OFF).\n");
#endif

	printf("  -b   : draw the cube in a brighter color (default OFF).\n");
	printf("  -i   : invert cube speed (default OFF).\n");
	printf("  -p   : do not display the CPU load (default OFF).\n");
	printf("  -R   : use red image.\n");
	printf("  -G   : use green image.\n");
	printf("  -B   : use blue image.\n");
	printf("  -h   : display this help.\n\n");	
}

void die()
{
	fprintf(stderr, "%s: exiting", pname);
	exit (1);
}

#ifndef SOLARIS 	// scan4objects doesnt work on Solaris because of alphasort
int scan4objects(char *dir) 
{
	struct dirent **names;
	int n;

	n = scandir(dir,&names,0,alphasort);
		
	while (n-- > 0) 
		if (strstr(names[n]->d_name,".wmc") != NULL)
		{			
			objects[nof_objects] = (char *)malloc(strlen(dir)+strlen(names[n]->d_name)+2);
			strcpy(objects[nof_objects],dir);
			if (dir[strlen(dir)] != '/') strcat(objects[nof_objects],"/");
			strcat(objects[nof_objects++],names[n]->d_name);
		}

	return nof_objects;
}
#endif

int next_object()
{
	if (nof_objects == 0) return -1;
	setupobj(objects[rand() % (nof_objects )]);
	
	return 0;
}


int loadobj(char *filename) {
	
	FILE *fp;
	char tmp[64] = {""};
	int i = 0;

	//printf("\nLoading file %s...",filename); fflush(stdout);
	
	if ((fp = fopen(filename,"rt")) == NULL) {
		printf("\nERROR: wmcube object-file not found (%s).\n\n",filename);
		exit(0);
	}

	if (fscanf(fp,"%s",tmp) < 1)
		printf("WARNING: Could not read first line of object-file (%s).\n",filename);
	
	if (strcmp(tmp,"WMCUBE_COORDINATES") != 0) {
		printf("\nError in objectfile: it must start with WMCUBE_COORDINATES\n\n");
		fclose(fp);
		exit(0);
	}

	if (fscanf(fp,"%s",tmp) < 1)
		printf("WARNING: Could not read second line of object-file (%s).\n",filename);

	while ((strcmp(tmp,"WMCUBE_LINES") != 0) && (strcmp(tmp,"WMCUBE_PLANES") != 0)) {		

	 	matrix = (float **)realloc(matrix,(i+1)*sizeof(float *)); mem_alloc_error(matrix);
		matrix[i] = (float *)malloc(3*sizeof(float)); mem_alloc_error(matrix[i]);
		if (fscanf(fp,"%f %f %f",&matrix[i][0],&matrix[i][1],&matrix[i][2]) < 3 )
			printf("WARNING: Could not read coordinates in object-file (%s).\n",filename);
		//printf("\n%d: %f %f %f",atoi(tmp), matrix[i][0],matrix[i][1],matrix[i][2]);

		if (atoi(tmp) != (++i)) { 
			
			printf("\nError in objectfile (WMCUBE_COORDINATES section):\n"
					 "the coordinates must be listed in order 1..n\n\n");
			fclose(fp);
			exit(0);
		}
		if (fscanf(fp,"%s",tmp) < 1)
			printf("WARNING: Could not read next line of object-file (%s).\n",filename);

		if (feof(fp)) {
			printf("\nError in objectfile: you must have a section WMCUBE_LINES or WMCUBE_PLANES\n\n");
			fclose(fp);
			exit(0);
		}
	}
 
	nofcoords = i;
	i = 0;

	if (strcmp(tmp,"WMCUBE_LINES") == 0) {

		planesORlines = 0;
		while (1) {
		
			cline = (int *)realloc(cline,(i+2)*sizeof(int)); mem_alloc_error(cline);
			if (fscanf(fp,"%d %d",&cline[i],&cline[i+1]) < 2) {
				if (feof(fp))
					break;
				else
					printf("WARNING: Could not read line coordinates in object-file (%s).\n",filename);
			}
			i += 2;
			//printf("\n%d %d",cline[i-2],cline[i-1]);
		
			if (cline[i-2] > nofcoords || cline[i-1] > nofcoords) { 
				printf("\nError in objectfile (WMCUBE_LINES section):\n"
					   "coordinates %d and/or %d don't exist\n\n",cline[i-2],cline[i-1]);
				fclose(fp);
				exit(0);
			}
		}
		noflines = i-2;
	}
	else if (strcmp(tmp,"WMCUBE_PLANES") == 0) {
	
		planesORlines = 1;
		while (1) {	
			planes = (int **)realloc(planes,(i+1)*sizeof(int *)); mem_alloc_error(planes);
			planes[i] = (int *)malloc(3*sizeof(int)); mem_alloc_error(planes[i]);
			if (fscanf(fp,"%d %d %d",&planes[i][0],&planes[i][1],&planes[i][2]) < 3) {
				if (feof(fp))
					break;
				else
					printf("WARNING: Could not read plane coordinates in object-file (%s).\n",filename);
			}
			//printf("\n%d: %d %d %d",i,planes[i][0],planes[i][1],planes[i][2]);

			planes[i][0]--; planes[i][1]--; planes[i][2]--;
			//printf("\n%d: %d %d %d\n",i,planes[i][0],planes[i][1],planes[i][2]);
			
			if (planes[i][0] > nofcoords || planes[i][1] > nofcoords || planes[i][2] > nofcoords) { 
				printf("\nError in objectfile (WMCUBE_PLANES section):\n"
				 	   "coordinates %d and/or %d and/or %d don't exist\n\n",planes[i][0],planes[i][1],planes[i][2]);
				fclose(fp);
				exit(0);
			}
			i++;		
		}
		nofplanes = i;
		plane_color =	(int *)malloc(nofplanes*sizeof(int)); mem_alloc_error(plane_color);
		zorder =		(int *)malloc(nofplanes*sizeof(int)); mem_alloc_error(zorder);
		for (i = 0; i < nofplanes; i++) zorder[i] = i;
		
	} else {
		printf("\nError in objectfile: you must have a section WMCUBE_LINES or WMCUBE_PLANES\n\n");
		fclose(fp);
		exit(0);
	}

	fclose(fp);
	return 1;
}

void mem_alloc_error(void *block) {
	if (block == NULL) {
		printf("\nError allocating memory!\n\n");
		exit(0);
	}
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 * Begin System Specific Code.  If you wish to port wmcube to a new platform,
 * you'll need to implement the following operations:
 *
 * int init_calc_cpu();
 * 	Perform feature tests to determine whether wmcube can run, and set up
 * 	any files/data structures/etc. to gather statistics.
 *
 * int calc_cpu_total();
 * 	return an integer reflecting the current CPU load
 */

#if defined LINUX

/*
 * init_calc_cpu doesn't have much to do on Linux, but it can check to see if
 * /proc/stat is available; if the user selected to monitor a particular CPU,
 * it can check it's existence.
 */
int init_calc_cpu()
{
	FILE *fp;
	int i;
	char cpuid[6];
	char check_cpu[6];
	char tmp[32];

	sprintf(check_cpu, "cpu%d", which_cpu);

	if ((fp = fopen("/proc/stat","rb")) == NULL) {
		perror("/proc/stat required for this system");
		return -1;
	}

	if (which_cpu == -1)
		return 0;

	for (i = -2; i < which_cpu; i++) {
		if (fscanf(fp, "%5s %31s %31s %31s %31s %31s %31s %31s %31s", cpuid, tmp, tmp, tmp, tmp, tmp, tmp, tmp, tmp) < 1)
			fprintf(stderr, "WARNING: could not read cpuid from /proc/stat\n");
        }

	if (strcmp(check_cpu,cpuid) != 0) {
                fprintf(stderr, "ERROR: could not read cpu-load on %s. Are you "
		    "sure you have an SMP system?\n",check_cpu);
                return -1;
        }
	fclose(fp);
	return (0); 
}

int calc_cpu_total() {
	unsigned int total, used;
	int t=0, i;
	static int previous_total = 0, previous_used = 0;
	char cpuid[6];
	unsigned int cpu,nice,system,idle;
	char tmp[32];
	FILE *fp;
	
	fp = fopen("/proc/stat","rt");

	for (i = -2; i < which_cpu; i++) {
		if (fscanf(fp,"%5s %u %u %u %u %31s %31s %31s %31s", cpuid, &cpu, &nice, &system, &idle, tmp, tmp, tmp, tmp) < 5)
			fprintf(stderr, "WARNING: could not read statistics from /proc/stat\n");
	}

	fclose(fp);

	used = cpu + system + use_nice*nice;
	total = used + idle + (1-use_nice)*nice;

	t = 100 * (double)(used - previous_used) / (double)(total - previous_total);
	previous_total = total;
	previous_used = used;
	
	return t;
}

#elif defined SOLARIS
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <kstat.h>

static kstat_ctl_t *kc;
static kstat_t **cpu_ksp_list;
static kstat_t *the_cpu;
static int ncpus;

/*
 * The biggest subtlety of the Solaris port is that init_calc_cpu can be called
 * after the initial program setup.  This occurs when a 'kstat state change'
 * occurs.  Usually this means that a CPU has been taken on or off-line using
 * the psradm command.  Another possibility is that on server systems, a new
 * CPU might have been hot-added to a running system.
 *
 * As a result, init_calc_cpu frees any resources it might have setup if needed,
 * and reinitializes everything.
 */
int init_calc_cpu()
{
	kstat_t *ksp;
	int i = 0;

	if (kc == NULL) {
		if ((kc = kstat_open()) == NULL) {
			fprintf(stderr, "wmcube: can't open /dev/kstat\n");
			return -1;
		}
	}

	if (which_cpu != -1) {
		/*
		 * User selected to monitor a particlur CPU.  find it...
		 */
		for (ksp = kc->kc_chain; ksp; ksp = ksp->ks_next) {
			if ((strcmp(ksp->ks_module, "cpu_stat") == 0) &&
			    (ksp->ks_instance == which_cpu)) {
				the_cpu = ksp;
				break;
			}
		}
		if (the_cpu == NULL) {
			fprintf(stderr, "CPU %d not found\n", which_cpu);
			return -1;
		}
	} else {
		/*
		 * User selected to monitor all CPUs.  First, count them.
		 */
		for (ksp = kc->kc_chain; ksp; ksp = ksp->ks_next) {
			if (strcmp(ksp->ks_module, "cpu_stat") == 0)
				i++;
		}

		if (cpu_ksp_list) {
			free(cpu_ksp_list);
		}
		cpu_ksp_list = (kstat_t **) calloc(i * sizeof (kstat_t *), 1); 
		ncpus = i;

		/*
		 * stash the ksp for each CPU.
		 */
		i = 0;
		for (ksp = kc->kc_chain; ksp; ksp = ksp->ks_next) {
			if (strcmp(ksp->ks_module, "cpu_stat") == 0) {
				cpu_ksp_list[i] = ksp;
				i++;
			}
		}
	}
	return 0;
}

int calc_cpu_total()
{
	int i;
	cpu_stat_t stat;
	static int previous_total = 0, previous_used = 0;
	int used, total, t, user = 0, wait = 0, kern = 0, idle = 0;

	/*
	 * Read each cpu's data.  If the kstat chain has changed (a state change
	 * has happened, maybe a new cpu was added to the system or one went
	 * away), then reinitialize everything with init_calc_cpu().  Finally,
	 * recursively call calc_cpu_total.
	 *
	 * We'll need to do a little better than this in the future, since we
	 * could recurse too much in the pathological case here.
	 */
	if (which_cpu == -1) {
		for (i = 0; i < ncpus; i++) {
			if (kstat_read(kc, cpu_ksp_list[i],
			    (void *)&stat) == -1) {
				if (init_calc_cpu() != 0) {
					fprintf(stderr, "failed to "
					    "reinitialize following state "
					    "change\n");
					return (-1);
				}
				return (calc_cpu_total());
			}
			user += stat.cpu_sysinfo.cpu[CPU_USER];   /* user */
			wait += stat.cpu_sysinfo.cpu[CPU_WAIT];   /* io wait */
			kern += stat.cpu_sysinfo.cpu[CPU_KERNEL]; /* sys */
			idle += stat.cpu_sysinfo.cpu[CPU_IDLE]; /*idle("free")*/
		}
	} else {
		if (kstat_read(kc, the_cpu, (void *)&stat) == -1) {
			if (init_calc_cpu() != 0) {
				fprintf(stderr, "failed to reinitialize "
				    "following state change\n");
				return (-1);
			}
			return (calc_cpu_total());
		}
		user += stat.cpu_sysinfo.cpu[CPU_USER];    /* user */
		wait += stat.cpu_sysinfo.cpu[CPU_WAIT];    /* io wait */
		kern += stat.cpu_sysinfo.cpu[CPU_KERNEL];  /* sys */
		idle += stat.cpu_sysinfo.cpu[CPU_IDLE];    /* idle("free") */
	}

	used = user + wait + kern;
	total = used + idle;
	t = 100 * (double)(used - previous_used) /
	    (double)(total - previous_total);
	previous_total = total;
	previous_used = used;
	return (t);
}

#elif defined FREEBSD

int init_calc_cpu()
{
        return 0;
}

#define GETSYSCTL(name, var) getsysctl(name, &(var), sizeof(var))

static void getsysctl(const char *name, void *ptr, size_t len)
{
	size_t nlen = len;

	if (sysctlbyname(name, ptr, &nlen, NULL, 0) == -1) {
		fprintf(stderr, "sysctl(%s...) failed: %s\n", name,
			strerror(errno));
		exit(1);
	}
	if (nlen != len) {
		fprintf(stderr, "sysctl(%s...) expected %lu, got %lu\n",
			name, (unsigned long)len, (unsigned long)nlen);
		exit(1);
	}
}

int calc_cpu_total() {
        int total, used, t=0;
        static int previous_total = 0, previous_used = 0;
        int cpu,nice,system,idle;
        unsigned long int cpu_time[CPUSTATES];

        GETSYSCTL("kern.cp_time", cpu_time);

        cpu = cpu_time[CP_USER];
        nice = cpu_time[CP_NICE];
        system = cpu_time[CP_SYS];
        idle = cpu_time[CP_IDLE];

        used = cpu + system + use_nice*nice;
        total = used + idle + (1-use_nice)*nice;

        t = 100 * (double)(used - previous_used) / (double)(total - previous_total);
        previous_total = total;
        previous_used = used;

        return t;
}

#elif defined OPENBSD

int init_calc_cpu()
{
	return 0;
}

int calc_cpu_total() {
        double avenrun[3];

        (void) getloadavg(avenrun, sizeof(avenrun) / sizeof(avenrun[0]));
        return(((5.0*avenrun[0] + 0.5) > 50) ? 50 : (5.0*avenrun[0] + 0.5))*2;
}


#elif defined NETBSD /* END OPENBSD */
#include <sys/sched.h>
#include <sys/sysctl.h>

int init_calc_cpu () 
{
        return 0;
}

int calc_cpu_total ()
{
        static u_int64_t last_cp_time[CPUSTATES] = { 0, 0, 0, 0, 0 };
        u_int64_t        curr_cp_time[CPUSTATES];
        u_int64_t        total_time = 0, idle_time = 0;
        int mib[2];
        int i;
        size_t ssize;
        const int IDLE_TIME = 4;
        const int NICE_TIME = 1;

        ssize = sizeof ( curr_cp_time );
        mib[0] = CTL_KERN;
        mib[1] = KERN_CP_TIME;
        if ( sysctl ( mib, 2, curr_cp_time, &ssize, NULL, 0 ) ) { 
                fprintf ( stderr, "wmcube: unable to read CP_TIME from sysctl()\n" );
                exit ( 0 );
        }
        if ( !use_nice ) 
                curr_cp_time[NICE_TIME] = 0;

        /* NetBSD gives 5 CPUSTATES - 
         * User, Nice, System, Interrupt, Idle 
         */
        idle_time = curr_cp_time[IDLE_TIME] - last_cp_time[IDLE_TIME];
        for ( i = 0; i < CPUSTATES; i++ ) {
                total_time += ( curr_cp_time[i] - last_cp_time[i] ); 
                last_cp_time[i] = curr_cp_time[i];
        }

        /* Calculate the % CPU usage as the User+Nice+System+Interrupt/Total
         * for the interval
         */
        return ( 100 * (int) ( total_time - idle_time ) / total_time );

}

#else /* END NETBSD */

/*
 * This is a stub which will compile for platforms other than LINUX or SOLARIS.
 * Use these to start your port to a new platform.
 */
int init_calc_cpu()
{
	return 0;
}

int calc_cpu_total()
{
	return 0;
}

#endif /* OS SPECIFIC CODE */
