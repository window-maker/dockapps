/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <stdio.h>
#include <X11/X.h>
#include <X11/xpm.h>
#include <X11/Xlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "wmgeneral.h"
#include "pixmaps.h"

/* Prototypes */
static void print_usage(void);
static void ParseCMDLine(int argc, char *argv[]);

int zindex = 0;		/* zipcode index */
size_t delay = 60;	/* delay for update, in seconds */

/**
 * Displays parameter usage information
 */
static void print_usage(void)
{
	printf("wmWeatherApp version: %s\n", VERSION);
	printf("Usage: wmWeatherApp -zip zipcode [-delay seconds]\n\n");
}

/**
 * Parse command line arguments entered by the user
 *
 * @param	argc	Number of arguments in argv
 * @param	argv	Array containing the arguments entered
 */
void ParseCMDLine(int argc, char *argv[])
{
	int i;

	for(i = 1; i < argc; i++)
	{
		if(!strcmp(argv[i], "-display"))
		{
			++i;
		}
		else if(!strcmp(argv[i], "-zip") || !strcmp(argv[i], "-z")
			|| !strcmp(argv[i], "-zipcode"))
		{
			++i;
			zindex = i;
		}
		else if(!strcmp(argv[i], "-d") || !strcmp(argv[i], "-delay"))
		{
			++i;

			if(atoi(argv[i]) != 0)
				delay = atoi(argv[i]);

			if(delay < 30)
			{
				printf("wmWeatherApp: delay of %d second%sis too short, "
					"using a sane value of 60 instead. (Use -df to force)\n",
					delay, delay > 1 ? "s " : " ");

				delay = 60;
			}
		}
		else if(!strcmp(argv[i], "-df"))
		{
			++i;

			if(atoi(argv[i]) != 0)
				delay = atoi(argv[i]);

			printf("wmWeatherApp: forcing delay to %d second%sIf this value is too short "
				"wmWeatherApp will continually fetch and display the current "
				"weather which will use more bandwidth, cpu, and memory than "
				"needed. I trust this is what you want.\n", delay,
				delay > 1 ? " " : ". ");
		}
		else
		{
			print_usage();
			_exit(1);
		}
	}

	if(zindex == 0 || zindex >= argc || atoi(argv[zindex]) == 0 || strlen(argv[zindex]) != 5)
	{
		print_usage();
		_exit(1);
	}
}

/**
 * Update the background image of the dockapp using
 * the image pulled from www.wunderground.com
 */
void draw_image(void)
{
	XpmAttributes Attributes;
	int Width;
	int Height;
	Colormap cmap;
	Pixmap NewPixmap, NewShapeMask;

	Attributes.valuemask = XpmExactColors | XpmCloseness | XpmReturnAllocPixels;
	Attributes.exactColors = 0;
	Attributes.closeness   = 40000;
	cmap = DefaultColormap(display, DefaultScreen(display));

	if(XpmReadFileToPixmap(display, Root, "/tmp/weather.xpm", &NewPixmap, &NewShapeMask, &Attributes) >= 0)
	{
		Height = Attributes.height;
		Width = Attributes.width;

		XCopyArea(display, NewPixmap, wmgen.pixmap, NormalGC, 0, 0, Width, Height, 5, 5);
		XFreeColors(display, cmap, Attributes.alloc_pixels, Attributes.nalloc_pixels, 0);
    }

	RedrawWindow();
}

int main(int argc, char *argv[])
{
	XEvent	event;
	char buffer[1024];
	char buffer2[512];
	char *tmp;
	FILE *fp;
	size_t i;
	time_t lastupdate = 0;
	int status;

	ParseCMDLine(argc, argv);
	openXwindow(argc, argv, xpm_master, xpm_mask_bits, xpm_mask_width, xpm_mask_height);

	while(1)
	{
		/* first handle X events */
		while(XPending(display))
		{
			XNextEvent(display, &event);
			switch(event.type)
			{
				case Expose:
					RedrawWindow();
					break;
				case ButtonPress:
					switch(event.xbutton.button)
					{
						case 1:		/* left mouse button */
							if(getenv("BROWSER") == NULL)
								setenv("BROWSER", "mozilla", 0);

							snprintf(buffer2, 1024, "http://www.wunderground.com/cgi-bin/findweather/getForecast?query=%s", argv[zindex]);

							if(!fork())
								execl("/bin/sh", "/bin/sh", getenv("BROWSER"), buffer2, NULL);
							else
								wait(&status);

							break;
						case 2:		/* middle mouse button */
							break;
						case 3:		/* right mouse button */
							break;
					}
					break;
				case ButtonRelease:
					break;
			}
		}

		/* at least delay seconds have passed since the last update
		 * so fetch the latest image and then update the display */
		if(time(NULL) - lastupdate >= delay)
		{
			/* TODO
			 * This could be ported to a configuration file which would
			 * make extending it to work with other sites. This is more
			 * of a quick hack to get everything working together and I
			 * plan on improving it sometime in the very near future.
			 *
			 * Also, I don't like relying on external binaries therefore
			 * in another version I will link with the curl c library */
			snprintf(buffer2, 1024, "curl \"http://www.wunderground.com/cgi-bin/findweather/getForecast?query=%s\" 2>/dev/null | grep \"wunderground.com/graphics/conds/\" | head -1", argv[zindex]);
			fp = popen(buffer2, "r");
			fgets(buffer, 1024, fp);
			pclose(fp);
			if((tmp = strstr(buffer, "src")) != NULL)
			{
				/* get past the src=" part */
				tmp += 5;

				/* find out where our url ends (right before the next " encountered) */
				for(i = 0; i < strlen(tmp) && tmp[i] != '"'; i++);

				memset(buffer2, '\0', 512);
				strncpy(buffer2, tmp, i);

				/* we have our url; grab the image */
				memset(buffer, '\0', 1024);
				sprintf(buffer, "wget --proxy=off --passive-ftp --tries 0 -q -O /tmp/weather.gif \"%s\"", buffer2);
				fp = popen(buffer, "r");
				pclose(fp);

				/* now we need to convert the image... */
				fp = popen("convert -geometry 64x64 /tmp/weather.gif /tmp/weather.xpm", "r");
				pclose(fp);

				/* display it... */
				draw_image();

				/* and finally remove our temp files */
				unlink("/tmp/weather.gif");
				unlink("/tmp/weather.xpm");

				/* this is to avoid thrashing the cpu by only updating
				 * after the timeout (specified with -delay on the command
				 * line) has expired */
				lastupdate = time(NULL);
			}
		}

		/* since we aren't doing anything mission-critical we should
		 * sleep for some duration; I find that 10000 works just fine */
		usleep(10000);
	}

	/* we should never get here */
	return(0);
}

