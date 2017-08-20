#ifndef DRAW_TEXT_H_
#define DRAW_TEXT_H_

#include <ctype.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <string.h>
#include "font.xpm"
#include "display_text.xpm"

void draw_text(char *text, int dx, int dy, Bool digit);
void restore_background(void);
void flush_background(void);
void init_variables(Display	*g_display, GC g_gc, Window	g_icon_window, int	offset_w, int offset_h, Pixmap **background);
#endif /* DRAW_TEXT_H_ */
