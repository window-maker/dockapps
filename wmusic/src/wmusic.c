/*  wmusic - a xmms remote-controlling DockApp
 *  Copyright (C) 2000-2001 Bastien Nocera <hadess@hadess.net>
 *  Maintained by John Chapin <john+wmusic@jtan.com>
 *
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

#define DBL_CLICK_INTERVAL 250	/* double click interval in milliseconds */
#define ARROW_INTERVAL 100	/* arrow update interval in milliseconds */
#define SCROLL_INTERVAL 300	/* scroll update interval in milliseconds */
#define SEPARATOR " ** "	/* The separator for the scrolling title */
#define DISPLAYSIZE 6		/* width of text to display (running title) */

#include <libdockapp/dockapp.h>
#include <playerctl/playerctl.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <locale.h>

#include "wmusic-master.xpm"
#include "wmusic-digits.xpm"

/*---------------------------------------------------------------------------*/
/*                             Prototypes                                    */
/*---------------------------------------------------------------------------*/

void copyNumArea(int x, int y, int sx, int sy, int dx, int dy);
void ActionPlay(int x, int y, DARect rect, void *data);
void ActionPause(int x, int y, DARect rect, void *data);
void ActionEject(int x, int y, DARect rect, void *data);
void ActionPrev(int x, int y, DARect rect, void *data);
void ActionNext(int x, int y, DARect rect, void *data);
void ActionStop(int x, int y, DARect rect, void *data);
void ActionFastr(int x, int y, DARect rect, void *data);
void ActionFastf(int x, int y, DARect rect, void *data);
void ToggleWins(int x, int y, DARect rect, void *data);
void ToggleVol(int x, int y, DARect rect, void *data);
void ChangeVol(int x, int y, DARect rect, void *data);
void ToggleTime(int x, int y, DARect rect, void *data);
void buttonPress(int button, int state, int x, int y);
void buttonRelease(int button, int state, int x, int y);
int PlayerConnect(void);
void DisplayRoutine();
void DrawPos (int pos);
void DrawTime(int time);
void DrawArrow(void);
void DrawVolume(void);
void DrawTitle(char *title);
void ExecuteXmms(void);

/*----------------------------------------------------------------------------*/
/*                             Variables                                      */
/*----------------------------------------------------------------------------*/

/* X11 variables */
char *displayName = "";
GC gc;
XEvent ev;

/* Dockapp variables */
PlayerctlPlayer *player;
char *xmms_cmd = "xmms";
Bool main_vis=0, pl_vis=0, eq_vis=0;
unsigned int volume_step = 5;
Bool run_excusive=0;

Bool t_time=0;
float title_pos = 0;
unsigned int arrow_pos = 0;
Bool pause_norotate = 0;
Time click_time=0;

Pixmap pixmap, mask;	/* Pixmap that is displayed */
Pixmap pixnum, masknum;	/* Pixmap source */

int left_pressed = 0; /* for pseudo drag callback */
int motion_event = 0; /* on motion events we do not want(too fast) display update */

static DAActionRect buttonRects[] = {
	{{5, 39, 14, 9}, ActionPrev},
	{{19, 39, 14, 9}, ActionNext},
	{{33, 39, 13, 9}, ActionFastr},
	{{46, 39, 13, 9}, ActionFastf},
	{{5, 48, 11, 11}, ActionEject},
	{{16, 48, 21, 11}, ActionPlay},
	{{37, 48, 11, 11}, ActionPause},
	{{48, 48, 11, 11}, ActionStop}
};

static DAActionRect toggleRect[] = {
	{{5, 5, 54, 30}, ToggleWins}
};

static DAActionRect globRect[] = {
	{{0, 0, 64, 64}, ToggleVol}
};

static DAActionRect displayRects[] = {
	{{5, 5, 54, 12}, ToggleTime},
};

static DAActionRect volumeRects[] = {
	{{5, 17, 38, 8 }, ChangeVol}
};

static DAProgramOption options[] = {
	{"-c", "--command", "Command to launch xmms", DOString, False,
		{&xmms_cmd} },
	{"-d", "--display", "Display to use", DOString, False, {&displayName} },
	{"-r", "--run", "Run xmms on startup", DONone, False, {NULL} },
	{"-V", "--volume", "Stepping of the wheel volume control (in percent)",
		DONatural, False, {&volume_step} },
	{"-a", "--rotate-arrow", "Do not rotate the arrow, when paused", 
		DONone, False, {NULL} },
	{"-l", "--time-left", "Show time left instead of time remaining by default",
	 DONone, False, {NULL} },
	{"-R", "--run-excusive", "Run xmms on startup, exit when xmms exits", DONone, False, {NULL} }
};

typedef struct
{
	wchar_t c;
	int x;
	int y;
} glyphdescr;

static glyphdescr glyphs[] = {
	{L'-', 67, 83}, {L'.', 73, 83}, {L'\x27', 79, 83},
	{L'(', 85, 83}, {L')', 91, 83}, {L'*', 97, 83}, {L'/', 103, 83},

	{L'0',  1, 83}, {L'1',  7, 83}, {L'2', 13, 83}, {L'3', 19, 83}, {L'4', 25, 83},
	{L'5', 31, 83}, {L'6', 37, 83}, {L'7', 43, 83}, {L'8', 49, 83}, {L'9', 55, 83},



	{L'A',  1, 73}, {L'a',  1, 73},
	{L'B',  7, 73}, {L'b',  7, 73},
	{L'C', 13, 73}, {L'c', 13, 73},
	{L'D', 19, 73}, {L'd', 19, 73},
	{L'E', 25, 73}, {L'e', 25, 73},

	{L'F', 31, 73}, {L'f', 31, 73},
	{L'G', 37, 73}, {L'g', 37, 73},
	{L'H', 43, 73}, {L'h', 43, 73},
	{L'I', 49, 73}, {L'i', 49, 73},
	{L'J', 55, 73}, {L'j', 55, 73},

	{L'K', 61, 73}, {L'k', 61, 73},
	{L'L', 67, 73}, {L'l', 67, 73},
	{L'M', 73, 73}, {L'm', 73, 73},
	{L'N', 79, 73}, {L'n', 79, 73},
	{L'O', 85, 73}, {L'o', 85, 73},

	{L'P', 91, 73}, {L'p', 91, 73},
	{L'Q', 97, 73}, {L'q', 97, 73},
	{L'R',103, 73}, {L'r',103, 73},
	{L'S',109, 73}, {L's',109, 73},
	{L'T',115, 73}, {L't',115, 73},

	{L'U',121, 73}, {L'u',121, 73},
	{L'V',127, 73}, {L'v',127, 73},
	{L'W',133, 73}, {L'w',133, 73},
	{L'X',139, 73}, {L'x',139, 73},
	{L'Y',145, 73}, {L'y',145, 73},

	{L'Z',151, 73}, {L'z',151, 73},


	{L'\x42e',  1, 93}, {L'\x44e',  1, 93}, /* cyrillic Yu */

	{L'\x410',  7, 93}, {L'\x430',  7, 93}, /* cyrillic A */
	{L'\x411', 13, 93}, {L'\x431', 13, 93}, /* cyrillic Be */
	{L'\x426', 19, 93}, {L'\x446', 19, 93}, /* cyrillic Ce */
	{L'\x414', 25, 93}, {L'\x434', 25, 93}, /* cyrillic De */
	{L'\x415', 31, 93}, {L'\x435', 31, 93}, /* cyrillic Ye */

	{L'\x424', 37, 93}, {L'\x444', 37, 93}, /* cyrillic eF */
	{L'\x413', 43, 93}, {L'\x433', 43, 93}, /* cyrillic Ge */
	{L'\x425', 49, 93}, {L'\x445', 49, 93}, /* cyrillic Ha */
	{L'\x418', 55, 93}, {L'\x438', 55, 93}, /* cyrillic I */
	{L'\x419', 61, 93}, {L'\x439', 61, 93}, /* cyrillic I-kratkoe */

	{L'\x41a', 67, 93}, {L'\x43a', 67, 93}, /* cyrillic Ka */
	{L'\x41b', 73, 93}, {L'\x43b', 73, 93}, /* cyrillic eL */
	{L'\x41c', 79, 93}, {L'\x43c', 79, 93}, /* cyrillic eM */
	{L'\x41d', 85, 93}, {L'\x43d', 85, 93}, /* cyrillic eN */
	{L'\x41e', 91, 93}, {L'\x43e', 91, 93}, /* cyrillic O */

	{L'\x41f', 97, 93}, {L'\x43f', 97, 93}, /* cyrillic Pe */
	{L'\x42f',103, 93}, {L'\x44f',103, 93}, /* cyrillic Ya */
	{L'\x420',109, 93}, {L'\x440',109, 93}, /* cyrillic eR */
	{L'\x421',115, 93}, {L'\x441',115, 93}, /* cyrillic eS */
	{L'\x422',121, 93}, {L'\x442',121, 93}, /* cyrillic Te */

	{L'\x423',127, 93}, {L'\x443',127, 93}, /* cyrillic U */
	{L'\x416',133, 93}, {L'\x436',133, 93}, /* cyrillic Je */
	{L'\x412',139, 93}, {L'\x432',139, 93}, /* cyrillic Ve */
	{L'\x42c',145, 93}, {L'\x44c',145, 93}, /* cyrillic MyagkijZnak */
	{L'\x42b',151, 93}, {L'\x44b',151, 93}, /* cyrillic Y */

	{L'\x417',157, 93}, {L'\x437',157, 93}, /* cyrillic Ze */
	{L'\x428',163, 93}, {L'\x448',163, 93}, /* cyrillic Sha */
	{L'\x42d',169, 93}, {L'\x44d',169, 93}, /* cyrillic E */
	{L'\x429',175, 93}, {L'\x449',175, 93}, /* cyrillic Scha */
	{L'\x427',181, 93}, {L'\x447',181, 93}, /* cyrillic Che */

	{L'\x42a',187, 93}, {L'\x44a',187, 93}, /* cyrillic TvyordyiZnak */
	{L'\x404',115, 83}, {L'\x454',115, 83}, /* ukrainian IE */
	{L'\x406', 49, 73}, {L'\x456', 49, 73}, /* ukrainian I */
	{L'\x407',109, 83}, {L'\x457',109, 83}, /* ukrainian YI */
	{L'\x491', 43, 93}, {L'\x490', 43, 93}, /* ukrainian GHE with upturn */

	{L'\x401',121, 83}, {L'\x451',121, 83}, /* cyrillic Yo */

	{L' ', 61, 83}
};

/*----------------------------------------------------------------------------*/
/*                              Functions                                     */
/*----------------------------------------------------------------------------*/

void copyNumArea(int x, int y, int sx, int sy, int dx, int dy)
{
	XCopyArea(DADisplay, pixnum, pixmap, gc, x, y, sx, sy, dx, dy);
}

void buttonDraw(DARect rect)
{
	copyNumArea((rect.x)-5, (rect.y)-8, rect.width, rect.height,
			rect.x, rect.y);
	DASetPixmap(pixmap);
}

void ActionPlay(int x, int y, DARect rect, void *data)
{
	if (data) {
		buttonDraw(rect);
	} else {
		GError *error = NULL;

		playerctl_player_play(player, &error);
		if (error != NULL)
			DAWarning("Could not execute command: %s",
				  error->message);
	}
}

void ActionPause(int x, int y, DARect rect, void *data)
{
	if (data) {
		buttonDraw(rect);
	} else {
		GError *error = NULL;

		playerctl_player_pause(player, &error);
		if (error != NULL)
			DAWarning("Could not execute command: %s",
				  error->message);
	}
}

void ActionEject(int x, int y, DARect rect, void *data)
{
	if (data) {
		buttonDraw(rect);
	} else {
		DAWarning("Eject function is no longer supported.");
	}
}

void ActionPrev(int x, int y, DARect rect, void *data)
{
	if (data) {
		buttonDraw(rect);
	} else {
		GError *error = NULL;

		playerctl_player_previous(player, &error);
		if (error != NULL)
			DAWarning("Could not execute command: %s",
				  error->message);
	}
}

void ActionNext(int x, int y, DARect rect, void *data)
{
	if (data) {
		buttonDraw(rect);
	} else {
		GError *error = NULL;

		playerctl_player_next(player, &error);
		if (error != NULL)
			DAWarning("Could not execute command: %s",
				  error->message);
	}
}

void ActionStop(int x, int y, DARect rect, void *data)
{
	if (data) {
		buttonDraw(rect);
	} else {
		GError *error = NULL;

		playerctl_player_stop(player, &error);
		if (error != NULL)
			DAWarning("Could not execute command: %s",
				  error->message);
	}
}

void ActionFastr(int x, int y, DARect rect, void *data)
{
	if (data) {
		buttonDraw(rect);
	} else {
		GError *error = NULL;

		playerctl_player_seek(player, -10000000, &error);
		if (error != NULL)
			DAWarning("Could not execute command: %s",
				  error->message);
	}
}

void ActionFastf(int x, int y, DARect rect, void *data)
{
	if (data) {
		buttonDraw(rect);
	} else {
		GError *error = NULL;

		playerctl_player_seek(player, 10000000, &error);
		if (error != NULL)
			DAWarning("Could not execute command: %s",
				  error->message);
	}
}

void ToggleWins(int x, int y, DARect rect, void *data)
{
	if (!player) {
		if ( (ev.xbutton.time-click_time) <= DBL_CLICK_INTERVAL )
		{
			click_time=0;
			ExecuteXmms();
		} else {
			click_time=ev.xbutton.time;
		}
	}
}

void ToggleVol(int x, int y, DARect rect, void *data)
{
	double volume;
	double factor;

	g_object_get(player, "volume", &volume, NULL);

	if (*(int*)data == 1)
		factor = 0.01 * volume_step;
	else
		factor = -0.01 * volume_step;
	volume += factor;

	if (volume > 1)
		volume = 1;
	if (volume < 0)
		volume = 0;

	g_object_set(player, "volume", volume, NULL);
}

void ChangeVol(int x, int y, DARect rect, void *data)
{
	float volume = ((float)x)/38;
	g_object_set(player, "volume", volume, NULL);
}

void ToggleTime(int x, int y, DARect rect, void *data)
{
	if (t_time)
		t_time = 0;
	else t_time =1;
}

void buttonPress(int button, int state, int x, int y)
{
	if (button==1)
		left_pressed=1;
	if (player)
	{
		if (button == 1)
		{
			char *tmp="1";
			DAProcessActionRects(x, y, buttonRects, sizeof(buttonRects)/sizeof(DAActionRect), tmp);
			DAProcessActionRects(x, y, displayRects, sizeof(displayRects)/sizeof(DAActionRect), tmp);
		}
		if (button == 2)
		{
			DAProcessActionRects(x, y, toggleRect, sizeof(toggleRect)/sizeof(DAActionRect), NULL);
		}
		if (button == 3)
		{
			char *tmp="1";
			DAProcessActionRects(x, y, toggleRect, sizeof(toggleRect)/sizeof(DAActionRect), tmp);
		}
		if ((button == 4) || (button == 5))
		{
			if (button == 5)
			{
				/* Wheel scrolls down */
				int tmp=2;
				DAProcessActionRects(x, y, globRect, sizeof(globRect)/sizeof(DAActionRect), &tmp);
			} else {
				/* Wheel scrolls up */
				int tmp=1;
				DAProcessActionRects(x, y, globRect, sizeof(globRect)/sizeof(DAActionRect), &tmp);
			}
		}
	}
	else
		DAProcessActionRects(x, y, toggleRect, sizeof(toggleRect)/sizeof(DAActionRect), NULL);
}

void buttonRelease(int button, int state, int x, int y)
{
	if (button==1)
		left_pressed=0;
	if (player)
	{
		if (button == 1)
		{
			copyNumArea(0,51, 54, 20, 5,39);
			DASetPixmap(pixmap);
			DAProcessActionRects(x, y, buttonRects, sizeof(buttonRects)/sizeof(DAActionRect), NULL);
		}
	}
}

void buttonDrag(int x, int y) 
{
	motion_event=1;
	if (left_pressed==1) {
		DAProcessActionRects(x,y, volumeRects, 1, NULL);
		DrawVolume();
		DASetPixmap(pixmap);
	}
}

int PlayerConnect(void)
{
	GError *error = NULL;
	static int previous_error_code = 0;
	static char* player_name = NULL;

	player = playerctl_player_new(NULL, &error);
	if (error != NULL) {
		/* don't spam error message */
		if (error->code != previous_error_code)
			DAWarning("Connection to player failed: %s",
				  error->message);
		previous_error_code = error->code;
		player_name = NULL;
		return 0;
	} else {
		previous_error_code = 0;
		if (!player_name) {
			g_object_get(player, "player_name", &player_name, NULL);
			player_name++; /* get rid of opening dot */
			if (player_name)
				DAWarning("Connected to %s", player_name);
		}
		return 1;
	}
}

void DisplayRoutine()
{
	int time = 0, length = 0, position = 100;
	char *title = NULL;
	GError *error = NULL;

	PlayerConnect();

	/* Compute diplay */
	if (!player)
	{
		if (run_excusive)
			exit(0);
		title = strdup("--");
		title_pos = 0;
		arrow_pos = 5;
	} else {
		char *length_str, *position_str, *status;

		g_object_get(player, "status", &status, NULL);
		if (status) {
			if (!strcmp(status, "Playing") ||
			    !strcmp(status, "Paused")) {
				g_object_get(player, "position", &time, NULL);

				title = playerctl_player_get_title(player,
								   &error);
				if (error != NULL)
					DAWarning("%s", error->message);

				length_str =
					playerctl_player_print_metadata_prop(
						player, "mpris:length", &error);
				if (error != NULL)
					DAWarning("%s", error->message);
				if (length_str)
					length = atoi(length_str);
				else
					length = 0;

				position_str =
					playerctl_player_print_metadata_prop(
						player, "xesam:trackNumber",
						&error);
				if (error != NULL)
					DAWarning("%s", error->message);
				if (position_str)
					position = atoi(position_str);
				else
					position = 0;

				if (!strcmp(status, "Paused") && pause_norotate)
					arrow_pos = 5;
			} else { /* not playing or paused */
				title = strdup("--");
				title_pos = 0;
				arrow_pos = 5;
			}

		} else { /* status undefined */
			title = strdup("--");
			title_pos = 0;
			arrow_pos = 5;
		}
	}

	/*Draw everything */
	if (t_time && length) DrawTime((length-time) / 1000);
	else DrawTime(time / 1000);
	DrawPos(position);
	DrawArrow();
	DrawVolume();
	DrawTitle(title);

	DASetPixmap(pixmap);

	if (title != NULL)
		free(title);
}

void DrawPos (int pos)
{
	char posstr[16];
	char *p = posstr;
	int i=1;

	if (pos > 99) pos=0;
	sprintf(posstr, "%02d", pos);
	
	for (;i<3; i++)
	{
		copyNumArea((*p-'0')*6 + 1, 1, 6, 7, (i*6)+39, 7);
		p++;
	}
}

void DrawTime(int time)
{
	char timestr[16];
	char *p = timestr;
	int i=0;

	time = time / 1000;

	/* 3 cases:
	 *     up to 99 minutes and 59 seconds
	 *     up to 99 hours and 59 minutes
	 *     more
	 */
	if (time < 6000)
	{
		sprintf(timestr, "%02d%02d", time / 60, time % 60);
	} else {
		if (time < 360000)
		{
			sprintf(timestr, "%02d%02d", time / 3600,
					time % 3600 / 60);
		} else {
			sprintf(timestr, "%02d%02d", 0, 0);
		}
	}

	for (;i<4; i++)
	{
		copyNumArea((*p-'0')*7 + 2, 11, 7, 9, i<2 ?(i*7)+7:(i*7)+12, 7);
		p++;
	}
}

void DrawArrow(void)
{
	copyNumArea((arrow_pos*8)+30, 22, 8, 9, 47, 19);
	arrow_pos++;
	if (arrow_pos > 4) arrow_pos = 0;
}

void DrawVolume(void)
{
		int volume;
		double volume_double;

		g_object_get(player, "volume", &volume_double, NULL);
		volume = (int)(36 * volume_double);
		if (volume > 36)
			volume = 36;
		copyNumArea(61, 0, volume, 6, 7, 18);
		copyNumArea(97, 0, 36-volume, 6, 7+volume, 18);
}

void DrawKbps(int bps)
{
	char kbpstr[16];
	char *p = kbpstr;
	int i=1;

	if (bps > 999000) bps=0;
	sprintf(kbpstr, "%03d", bps / 1000);

	for (;i<4; i++)
	{
		copyNumArea((*p-'0')*6 + 1, 1, 6, 7, (i*6)+1, 26);
		p++;
	}
	copyNumArea(55, 39, 18, 8, 25, 26);
}

void DrawChar(wchar_t wc, int x, int y)
{
	int i;
	for(i = 0; i < sizeof(glyphs)/sizeof(glyphdescr) - 1; ++i)
	{
		if(wc == glyphs[i].c)
			break;
	}
	copyNumArea(glyphs[i].x, glyphs[i].y, 6, 8, x, y);
}

int DrawChars(char *title, int tpos, int pos)
{
	wchar_t wc;

	mbtowc(NULL, NULL, 0);
	while(*title && (pos <= (tpos + DISPLAYSIZE)))
	{
		int len = mbtowc(&wc, title, MB_CUR_MAX);
		title += len;
		if(pos >= tpos)
			DrawChar(wc, (pos - tpos)*6 + 7, 26);
		++pos;
	}
	return pos;
}

void DrawTitle(char *name)
{
	int len, pos, tpos = title_pos;

	if (name == NULL)
		return;

	len = pos = DrawChars(name, tpos, 0);

	if(pos < 6)
	{
		DrawChars("      ", tpos, pos);
		return;
	}

	if(pos <= tpos + DISPLAYSIZE)
		pos = DrawChars(SEPARATOR, tpos, pos);

	if(pos <= tpos + DISPLAYSIZE)
		DrawChars(name, tpos, pos);

	if(tpos >= len + strlen(SEPARATOR))
		title_pos = 0;

	title_pos = title_pos + 0.5;
}

void ExecuteXmms(void)
{
	char *command;
	int status;

	command=malloc(strlen(xmms_cmd)+5);
	sprintf(command, "%s &", xmms_cmd);
	status = system(command);
	if (status)
	{
		fprintf(stderr, "XMMS can't be launched, exiting...");
		exit(1);
	}
	while (!PlayerConnect())
		usleep(10000L);
	free(command);
}
/*----------------------------------------------------------------------------*/
/*                                   Main                                     */
/*----------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
	short unsigned int height, width;
	DACallbacks callbacks={NULL, buttonPress, buttonRelease, buttonDrag,
		NULL, NULL, NULL};

	/* Initialization */
	DAParseArguments(argc, argv, options,
		sizeof(options)/sizeof(DAProgramOption),
		"XMMS remote control by Bastien Nocera <hadess@hadess.net>",
		PACKAGE_STRING);

	setlocale(LC_ALL, "");
	DAInitialize(displayName, "wmusic", 64, 64, argc, argv);
	DASetCallbacks(&callbacks);
	DASetTimeout(100);

	DAMakePixmapFromData(wmusic_master_xpm, &pixmap,
			&mask, &height, &width);
	DAMakePixmapFromData(wmusic_digits_xpm, &pixnum,
			&masknum, &height, &width);
	gc = DefaultGC(DADisplay, DefaultScreen(DADisplay));
	DASetShapeWithOffset(mask, 0, 0);

	DASetPixmap(pixmap);
	DAShow();

	/* End of initialization */

	if (options[4].used) pause_norotate=1;
	if (options[5].used) t_time=1;
	if (options[6].used) run_excusive=1;

	PlayerConnect();

	/* Launch xmms if it's not running and -r or -R was used */
	if ((!player) && (options[2].used || run_excusive))
	{
		ExecuteXmms();
	}

	/* Update the display */
	DisplayRoutine();

	while (1) {
		if (DANextEventOrTimeout(&ev, 100))
				DAProcessEvent(&ev);
		if (!motion_event)
			DisplayRoutine();
		else
			motion_event=0;
	}
	return 0;
}

