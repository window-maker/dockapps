/*
 * wmmp3
 * Copyright (c)1999 Patrick Crosby <xb@dotfiles.com>.
 * This software covered by the GPL.  See COPYING file for details.
 *
 * buttons.c                                                    
 *                                                                
 * This file contains button code.
 *                                                                
 * $Id: mpg123ctl.c,v 1.12 1999/10/08 06:21:41 pcrosby Exp $
 */

#include "buttons.h"

struct coord {
	int x;
	int y;
	int w;
	int h;
};

struct coord btn_pos[] = {
	{35, 34, 12, 11},	/* stop */
	{46, 34, 12, 11},	/* play */
	{35, 45, 12, 11},	/* back */
	{46, 45, 12, 11},	/* next */
	{6, 34, 12, 11},	/* prev_dir */
	{17, 34, 12, 11},	/* random */
	{6, 45, 12, 11},	/* next_dir */
	{17, 45, 12, 11},	/* repeat */
	{5, 18, 54, 12}		/* song title */
};

struct coord btn_up[] = {
	{35, 70, 12, 11},	/* stop */
	{46, 70, 12, 11},	/* play */
	{35, 81, 12, 11},	/* back */
	{46, 81, 12, 11},	/* next */
	{6, 70, 12, 11},	/* prev_dir */
	{17, 70, 12, 11},	/* random */
	{6, 81, 12, 11},	/* next_dir */
	{17, 81, 12, 11}	/* repeat */
};

struct coord btn_down[] = {
	{35, 97, 12, 11},	/* stop */
	{46, 97, 12, 11},	/* play */
	{35, 108, 12, 11},	/* back */
	{46, 108, 12, 11},	/* next */
	{6, 97, 12, 11},	/* prev_dir */
	{17, 97, 12, 11},	/* random */
	{6, 108, 12, 11},	/* next_dir */
	{17, 108, 12, 11}	/* repeat */
};

struct coord btn_gray[] = {
	{95, 70, 12, 11},	/* stop */
	{106, 70, 12, 11},	/* play */
	{95, 81, 12, 11},	/* back */
	{106, 81, 12, 11},	/* next */
	{66, 70, 12, 11},	/* prev_dir */
	{77, 70, 12, 11},	/* random */
	{66, 81, 12, 11},	/* next_dir */
	{77, 81, 12, 11}	/* repeat */
};

void button_down(int i)
{
	copyXPMArea(btn_down[i].x, btn_down[i].y, btn_down[i].w, btn_down[i].h,
		    btn_pos[i].x, btn_pos[i].y);
}

void button_up(int i)
{
	copyXPMArea(btn_up[i].x, btn_up[i].y, btn_up[i].w, btn_up[i].h,
		    btn_pos[i].x, btn_pos[i].y);
}

void button_gray(int i)
{
	copyXPMArea(btn_gray[i].x, btn_gray[i].y, btn_gray[i].w, btn_gray[i].h,
		    btn_pos[i].x, btn_pos[i].y);
}
