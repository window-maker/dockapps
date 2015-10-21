/*
List of events catched:

button bar window:
    - button press and release => run a menu entry
    - enter/leave => show/hide bar (if not -O click) and highlight (if set)
    - pointer motion (if highlight set) => change highlight position

menu icon:
    - button press => show/hide bar
    - enter/leave (if not -O click) => show/hide bar
    - move/reparent => find WMFrame and bar coords, update frame events.

WMFrame (if some):
    - enter/leave (if not -O click) => show/hide bar
    - move/destroy => find WMFrame and bar coords, update frame events.

NB: Move is actually part of a larger "Configure" event;  Configure,
Reparent and Destroy events are catched with StructureNotifyMask.

*/

#include <stdlib.h>

#include <libdockapp/dockapp.h>

#include "types.h"
#include "events.h"
#include "buttonbar.h"
#include "options.h"
#include "menu.h"
#include "xobjects.h"
#include "pixmaps.h"
#include "error.h"

static bool BarShown = false ;
static bool HideBarDelayed = false ;

static void FindWMFrameAndBarPos (void)
{
    XWindowAttributes wa ;
    Window tile, parent, root, *children ;
    unsigned int nChildren ;
    long evMask ;

    /* find window just under root, it is the wm frame icon */
    /* (or DAWindow itself if none) */
    root = 0 ;
    tile = parent = DAWindow ;
    while (parent != root)
    {
	tile = parent ;
	XQueryTree (DADisplay, tile,
	    &root, &parent, &children, &nChildren) ;
	if (children != NULL) XFree (children) ;
    }

    /* container actually changed ? */
    if (WMFrame != tile)
    {
#if 0
	/*
	TODO: put this back once a solution has been found to avoid an X
	error, when WMFrame has been destroyed while we were looking for our
	new frame.  Meanwhile, it seems acceptable to not unregister events
	since in the normal case WMFrame is going to be destroyed !
	*/
	/* remove events from old container (if this is not the first time) */
	if (WMFrame != 0 && WMFrame != DAWindow)
	    XSelectInput (DADisplay, WMFrame, 0) ;
#endif

	/* add events to new container */
	if (tile != DAWindow)
	{
	    evMask = StructureNotifyMask ; /* move/destroy */
	    if (! ClickOnly)
		evMask |= EnterWindowMask | LeaveWindowMask ;
	    XSelectInput (DADisplay, tile, evMask) ;
	}

	WMFrame = tile ;
    }

    XGetWindowAttributes (DADisplay, WMFrame, & wa) ;
    ButtonBar_SetPositionFromDockApp (wa.x, wa.y, wa.width, wa.height) ;
}

static void EnterApp (void)
{
    if (Menu_HasChanged ())
    {
        Menu_Reload () ;

        Pixmaps_LoadMenu () ;
        Pixmaps_LoadTile () ;

        ButtonBar_Rebuild () ;
	/* adjust position depending on possible new size */
	FindWMFrameAndBarPos () ;
    }

    ButtonBar_Show () ;
    BarShown = true ;
    HideBarDelayed = false ;
}

static void LeaveBar (void)
{
    ButtonBar_Hide () ;
    BarShown = false ;
    HideBarDelayed = false ;
}

static void LeaveApp (void)
{
    if (BarShown)
    {
	ButtonBar_Unhighlight () ;
	HideBarDelayed = ! ClickOnly ;
    }
}

static void InvokeBar (int x, int y)
{
    int h, w ;
    h = Menu_GetNbRows () ;
    w = Menu_GetNbColumns () ;
    x /= TileXSize ;
    y /= TileYSize ;
    if (0 <= y && y < h && 0 <= x && x < w)
    {
        int entry ;
        entry = y + h*x ;

        if (entry < Menu_GetNbEntries ())
        {
            const char *command;

            command = Menu_GetEntryCommand (entry);
            if (system (command) == -1)
                warn("'%s' returned an error\n", command);
        }
    }

    LeaveBar () ;
}

static void PressApp (int button, int state, int x, int y)
{
    (void) button;
    (void) state;
    (void) x;
    (void) y;
    if (BarShown) LeaveBar () ;
    else EnterApp () ;
}

extern void Events_SetCallbacks (void)
{
    DACallbacks events ;
    XSetWindowAttributes ws ;
    XWindowAttributes wa ;

    events.destroy = NULL ;
    events.buttonPress = PressApp ;
    events.buttonRelease = NULL ;
    events.motion = NULL ;
    events.enter = (ClickOnly ? NULL : EnterApp) ;
    events.leave = (ClickOnly ? NULL : LeaveApp) ;

    DASetCallbacks (& events) ;

    /* update set of events we want to catch on the dock app */
    XGetWindowAttributes (DADisplay, DAWindow, & wa) ;
    ws.event_mask = wa.your_event_mask
	| StructureNotifyMask ; /* move/reparent */
    /* work around a bug in libdockapp: not selecting Enter/Leave events */
    if (! ClickOnly)
        ws.event_mask |= EnterWindowMask | LeaveWindowMask ;
    XChangeWindowAttributes (DADisplay, DAWindow, CWEventMask, & ws) ;
}

extern void Events_Loop (void)
{
    XEvent ev ;
    bool canShowBar = true ;

    while (true)
    {
	/* get some event to process */
	if (HideBarDelayed)
	{
	    if (! DANextEventOrTimeout (& ev, HideTimeout))
	    {
		/* timeout ! */
		LeaveBar () ;
		continue ; /* restart waiting for an event */
	    }
	}
	else
	{
	    XNextEvent (DADisplay, & ev) ;
	}
	/* event available, process it */


	if (ev.type == EnterNotify) /* catch entering any wmmenu window */
	{
	    if (canShowBar) EnterApp () ;
	    canShowBar = true ;
	}
	else
	if (ev.type == LeaveNotify) /* catch leaving any wmmenu window */
	{
	    /* when cursor goes from icon to dock tile */
	    /* take care to not show the bar back if it is already hidden */
	    if (ev.xany.window == DAWindow)
		canShowBar = BarShown ;
	    LeaveApp () ;
	}
	else
	if (ev.xany.window == DAWindow) switch (ev.type)
        {
	    case ReparentNotify :
	    case ConfigureNotify :
		/* find new WMFrame and update bar position */
		FindWMFrameAndBarPos () ;
		break ;

	    default :
		DAProcessEvent (& ev) ;
		break ;
        }
        else
        if (ev.xany.window == ButtonBarWindow) switch (ev.type)
        {
            case ButtonRelease :
                InvokeBar (ev.xbutton.x, ev.xbutton.y) ;
                break ;

	    case MotionNotify :
		if (HighlightImage != 0) /* try to avoid func call */
		{
		    ButtonBar_Highlight (
			ev.xmotion.x/TileXSize, ev.xmotion.y/TileYSize) ;
		}
		break ;
        }
        else
        if (ev.xany.window == WMFrame && WMFrame != 0) switch (ev.type)
	{
	    case DestroyNotify :
	    case ConfigureNotify :
		/* find new WMFrame and update bar position */
		FindWMFrameAndBarPos () ;
		break ;
	}
    }
}
