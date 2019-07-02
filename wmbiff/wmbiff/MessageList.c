#include "Client.h"
#include "MessageList.h"
#include <X11/Xlib.h>
#ifdef HAVE_X11_XPM_H
#include <X11/xpm.h>
#endif
#ifdef HAVE_XPM_H
#include <xpm.h>
#endif
#include <X11/Xutil.h>			/* needed for Region on solaris? */
#include <assert.h>

#define LEFT_MAR 6
#define RIGHT_MAR 6
#define COL_SEP 4

extern Display *display;
extern Window Root;
extern int screen;
extern int x_fd;
extern int d_depth;
extern Window win;

static XSizeHints mysizehints;
extern Pixel back_pix, fore_pix;
static Window newwin;
static GC localGC;
extern Pixel GetColor(const char *name);

static XFontStruct *fn;
static int fontHeight;
extern const char *foreground;
extern const char *background;

Pop3 *Active_pc;

static int loadFont(const char *fontname)
{
	if (display != NULL) {
		fn = XLoadQueryFont(display, fontname);
		if (fn) {
			XSetFont(display, localGC, fn->fid);
			fontHeight =
				fn->max_bounds.ascent + fn->max_bounds.descent + 2;
			return 0;
		} else {
			printf("couldn't set font! (%s)\n", fontname);
		}
	}

	return -1;
}

static int flush_expose(Window w)
{
	XEvent dummy;
	int i = 0;

	while (XCheckTypedWindowEvent(display, w, Expose, &dummy))
		i++;

	return i;
}

struct msglst *Headers;
void msglst_show(Pop3 *pc, int x, int y)
{
	int maxfrm = 0;
	int maxsubj = 0;
	int limit = 10;
	XGCValues gcv;
	unsigned long gcm;

	Active_pc = pc;				/* hold so we can release later. */

	/* local gc */
	gcm = GCForeground | GCBackground | GCGraphicsExposures;
	gcv.foreground = GetColor(foreground);
	gcv.background = GetColor(background);
	gcv.graphics_exposures = 0;
	localGC = XCreateGC(display, Root, gcm, &gcv);

	if (fn == NULL) {
		/* loadFont? or use a proportional instead?  mmm. */
		if (loadFont("-*-fixed-*-r-*-*-10-*-*-*-*-*-*-*") < 0) {
			return;
		}
	}
	if (pc->getHeaders == NULL) {
		DM(pc, DEBUG_INFO, "no getHeaders callback\n");
		return;
	}
	Headers = pc->getHeaders(pc);
	if (Headers == NULL) {
#define NO_MSG "no new messages"
		mysizehints.height = 5 + fontHeight;
		mysizehints.width = XTextWidth(fn, NO_MSG, strlen(NO_MSG));
		DM(pc, DEBUG_INFO, "no new messages\n");
	} else {
		struct msglst *h;
		mysizehints.height = 5;
		for (h = Headers; h != NULL && limit > 0; h = h->next, limit--) {
			int frmlen;
			char *c;
			int subjlen;

			if ((c = index(h->from, '\r')) != NULL) {
				*c = '\0';		/* chomp newlines */
			}
			if ((c = index(h->subj, '\r')) != NULL) {
				*c = '\0';		/* chomp newlines */
			}

			if ((c = index(h->from, '<')) != NULL) {
				*c = '\0';		/* chomp <foo@bar */
			}
			if (h->from[0] == '"') {	/* remove "'s */
				for (c = &h->from[1]; *c && *c != '"'; c++) {
					*(c - 1) = *c;
				}
				*(c - 1) = '\0';
			}


			subjlen = XTextWidth(fn, h->subj, strlen(h->subj));
			frmlen = XTextWidth(fn, h->from, strlen(h->from));
			if (frmlen > maxfrm) {
				maxfrm = frmlen;
			}
			if (subjlen > maxsubj) {
				maxsubj = subjlen;
			}
			mysizehints.height += fontHeight;
		}
		mysizehints.width =
			maxfrm + maxsubj + LEFT_MAR + RIGHT_MAR + COL_SEP;
	}

	/* Create a window to hold the stuff */
	mysizehints.flags = USSize | USPosition;
	mysizehints.x = max(x - mysizehints.width, 0);
	mysizehints.y = max(y - mysizehints.height, 0);

	newwin =
		XCreateSimpleWindow(display, Root, mysizehints.x, mysizehints.y,
							mysizehints.width, mysizehints.height, 2,
							gcv.foreground, gcv.background);
	XSetWMNormalHints(display, newwin, &mysizehints);
	XStoreName(display, newwin, pc->label);
	XSelectInput(display, newwin, ExposureMask);

	{							/* I confess I don't know what this does or whether it matters */
		XSetWindowAttributes xswa;
		xswa.backing_store = Always;
		xswa.bit_gravity = CenterGravity;
		XChangeWindowAttributes(display, newwin,
								CWBackingStore | CWBitGravity, &xswa);
	}

	XMapWindow(display, newwin);
}

/* may be called without the window open */
void msglst_hide(void)
{
	if (newwin) {
		flush_expose(newwin);	/* swallow the messages */
		XDestroyWindow(display, newwin);
		//   } else {
		// no window fprintf(stderr, "unexpected error destroying msglist window\n");
		if (Active_pc->releaseHeaders != NULL && Headers != NULL) {
			Active_pc->releaseHeaders(Active_pc, Headers);
		}
		newwin = 0;
	}
}

void msglst_redraw(void)
{
	XEvent dummy;
	unsigned int width, height;
	unsigned int bw, d;
	int x, y;
	Window r;

	if (newwin == 0) {
		return;
	}

	while (XCheckTypedWindowEvent(display, newwin, Expose, &dummy));
	XGetGeometry(display, newwin, &r, &x, &y, &width, &height, &bw, &d);

	XSetForeground(display, localGC, GetColor(background));
	XFillRectangle(display, newwin, localGC, 0, 0, width, height);

	XSetForeground(display, localGC, GetColor(foreground));
	XSetBackground(display, localGC, GetColor(background));

	if (Headers == NULL) {
		XDrawString(display, newwin, localGC, 0, fontHeight,
					NO_MSG, strlen(NO_MSG));
		flush_expose(newwin);
	} else {
		int linenum;
		struct msglst *h;
		int limit = 10;
		int maxfrm = 0;

		/* draw the from lines */
		for (h = Headers, linenum = 0; h != NULL && linenum < limit;
			 h = h->next, linenum++) {
			int frm = XTextWidth(fn, h->from, strlen(h->from));
			if (frm > maxfrm) {
				maxfrm = frm;
			}
			XDrawString(display, newwin, localGC, LEFT_MAR,
						(linenum + 1) * fontHeight, h->from,
						strlen(h->from));
		}

		/* draw the subject lines */
		for (h = Headers, linenum = 0; h != NULL && linenum < limit;
			 h = h->next, linenum++) {
			XDrawString(display, newwin, localGC,
						LEFT_MAR + maxfrm + COL_SEP,
						(linenum + 1) * fontHeight, h->subj,
						strlen(h->subj));
		}
	}
}
