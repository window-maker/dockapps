#ifndef DRAW_TEXT_H_
#define DRAW_TEXT_H_

#include <X11/Xlib.h>
#include <string.h>
#include "font.xpm"
#include "display_text.xpm"

void restore_background(void);
void flush_background(void);
void init_variables(Display	*g_display, GC g_gc, Window	g_icon_window, int	offset_w, int offset_h, Pixmap **background);
#endif /* DRAW_TEXT_H_ */
