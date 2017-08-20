#include "draw_text.h"

static Display	*display;
static GC 		gc;
static Pixmap draw_area_pixmap, font_pixmap, display_text_pixmap;
static Window	icon_window;

static void dockapp_copyarea(Pixmap src, Pixmap dist, int x_src, int y_src, int w,
		int h, int x_dist, int y_dist);
static int	offset_w, offset_h;
void init_variables(Display	*g_display, GC g_gc, Window	g_icon_window, int	g_offset_w, int g_offset_h, Pixmap **background) {
	display = g_display;
	gc = g_gc;
	icon_window = g_icon_window;
	offset_h = g_offset_h;
	offset_w = g_offset_w;

	XpmCreatePixmapFromData(display, icon_window, display_text_xpm, &display_text_pixmap, NULL, NULL);
	XpmCreatePixmapFromData(display, icon_window, font_xpm, &font_pixmap, NULL, NULL);

	*background = &display_text_pixmap;
}

void restore_background() {
	XpmCreatePixmapFromData(display, icon_window, display_text_xpm, &display_text_pixmap, NULL, NULL);

}
void flush_background() {
	XCopyArea(display, display_text_pixmap, icon_window, gc, 0, 0, 64, 64, offset_w, offset_h);
	XFlush(display);

}

void dockapp_copyarea(Pixmap src, Pixmap dist, int x_src, int y_src, int w,
		int h, int x_dist, int y_dist) {
	XCopyArea(display, src, dist, gc, x_src, y_src, w, h, x_dist, y_dist);
	//XCopyArea(display, src, text_window, gc, x_src, y_src, w, h, x_dist, y_dist);
}

void draw_text(char *text, int dx, int dy, Bool digit) {
	int ax, ay = 1, bx, len, i;
	char tmptext[255] = "";
	len = strlen(text);
	bx = 4;

	/*
	 for (i = 0; i < len; i++) {
	 digit = (!isalpha(text[i])) ? True : False;
	 }
	 */

	if (digit) {
		if (len == 4)
			dx -= 6;
		strcat(tmptext, text);
		if (len == 3)
			strcat(tmptext, text);
		if (len == 2) {
			tmptext[0] = 0x20;
			tmptext[1] = text[0];
			tmptext[2] = text[1];
			len++;
		}
		if (len == 1) {
			tmptext[0] = ' ';
			tmptext[1] = ' ';
			tmptext[2] = text[0];
			len += 2;
		}
	} else {
		strcpy(tmptext, text);
	}

	for (i = 0; i < len; i++) {
		if (isalpha(tmptext[i])) {
			ax = ((tolower(tmptext[i]) - 97) * 6) + 1;
			ay = 1;
		} else {
			ax = ((tmptext[i] - 33) * 6) + 1;
			ay = 10;
		}
		/* Space */
		if (tmptext[i] == 0x20)
			ax = 79;
		/* Draw Text */
		dockapp_copyarea(font_pixmap, display_text_pixmap, ax, ay, 6, 8, dx, dy);
		dx += 6;
	}

}
