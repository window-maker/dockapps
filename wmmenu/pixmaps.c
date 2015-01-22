#include <stdio.h>
#include <assert.h>

#ifdef WITH_GDKPIXBUF
#include <gdk-pixbuf/gdk-pixbuf-xlib.h>
#endif
#include <dockapp.h>

#include "pixmaps.h"
#include "xobjects.h"
#include "options.h"
#include "utils.h"
#include "error.h"
#include "menu.h"

#include "defaultTile.xpm"
#include "defaultIcon.xpm"

static char ** CurrentDefault = defaultIcon_xpm ;

#ifdef WITH_GDKPIXBUF

extern void Pixmaps_FindLoad (const char * name,
    Pixmap * imageP, Pixmap * maskP, int * wP, int * hP)
{
    static int mustInitGdkPixbuf = 1 ;
    char path [FILENAME_MAX] ;
    GdkPixbuf * pix ;
    int width, height ;
    void (* problem) (const char *, ...) ;

    if (mustInitGdkPixbuf)
    {
        gdk_pixbuf_xlib_init (DADisplay, DefaultScreen (DADisplay)) ;
        mustInitGdkPixbuf = 0 ;
    }

    assert (name != NULL) ;
    assert (imageP != NULL) ;
    assert (maskP != NULL) ;
    problem = (CurrentDefault == NULL ? error : warn) ;

#define UseDefault() \
    if ((pix = gdk_pixbuf_new_from_xpm_data ( \
        (const char **)CurrentDefault)) == NULL) \
        error ("can't create internal default pixmap")

    if (name == NULL || name[0] == EOS)
    {
	if (CurrentDefault == NULL)
	    error ("missing image name specification") ;

        UseDefault () ;
    }
    else
    if (! File_FindInPath (path, sizeof path, PixmapPath, name))
    {
        problem ("can't find image \"%s\"", name) ;
        UseDefault () ;
    }
    else
    if ((pix = gdk_pixbuf_new_from_file (path)) == NULL)
    {
        problem ("can't load image \"%s\"", path) ;
        UseDefault () ;
    }

    /* get loaded geometry */
    width = gdk_pixbuf_get_width (pix) ;
    height = gdk_pixbuf_get_height (pix) ;

    /* scale if allowed, possible, and useful */
    if (AutoScale &&
	TileXSize > 0 && TileYSize > 0 &&
	(width > TileXSize || height > TileYSize))
    {
	GdkPixbuf * scaled ;

	/*
	Compare width/TileXSize to height/TileYSize to determine a new
	size that fits within TileXSize x TileYSize and keeps the same
	aspect ratio as before.  The biggest ratio wins.
	*/
	if (width*TileYSize >= height*TileXSize)
	{
	    /* width directs re-scaling */
	    height = (height * TileXSize) / width ;
	    width = TileXSize ;
	}
	else
	{
	    /* height directs re-scaling */
	    width = (width * TileYSize) / height ;
	    height = TileYSize ;
	}

	scaled = gdk_pixbuf_scale_simple (pix,
	    width, height, GDK_INTERP_HYPER) ;
	gdk_pixbuf_unref (pix) ;
	pix = scaled ;
    }

    if (wP != NULL) *wP = width ;
    if (hP != NULL) *hP = height ;
    gdk_pixbuf_xlib_render_pixmap_and_mask (pix, imageP, maskP, 128) ;
    /* don't forget to free now we've done pixmaps */
    gdk_pixbuf_unref (pix) ;
}

/*
We have to reimplement a few trivial gdk functions here to avoid linking with
it !
*/

extern gint gdk_screen_width (void)
{
    return DisplayWidth (DADisplay, DefaultScreen (DADisplay)) ;
}

extern gint gdk_screen_height (void)
{
    return DisplayHeight (DADisplay, DefaultScreen (DADisplay)) ;
}

extern gboolean gdk_color_parse (const gchar * spec, GdkColor * color)
{
    XColor scr ;
#if 1

    /* Lukasz Pankowski suggested this */
    if (! XParseColor (DADisplay,
        DefaultColormap (DADisplay, DefaultScreen (DADisplay)),
        spec, & scr))
#else
    XColor exact ;

    if (! XAllocNamedColor (DADisplay,
        DefaultColormap (DADisplay, DefaultScreen (DADisplay)),
        spec, & scr, & exact))
#endif
    {
        return FALSE ;
    }
    else
    {
        color->pixel = scr.pixel ;
        color->red   = scr.red ;
        color->green = scr.green ;
        color->blue  = scr.blue ;
        return TRUE ;
    }
}

#else

extern void Pixmaps_FindLoad (const char * name,
    Pixmap * imageP, Pixmap * maskP, int * wP, int * hP)
{
    char path [FILENAME_MAX] ;
    XpmAttributes attr ;
    void (* problem) (const char *, ...) ;

    assert (name != NULL) ;
    assert (imageP != NULL) ;
    assert (maskP != NULL) ;
    attr.valuemask = 0 ;
    problem = (CurrentDefault == NULL ? error : warn) ;

#define UseDefault() \
    if (XpmCreatePixmapFromData (DADisplay, DefaultRootWindow (DADisplay), \
            CurrentDefault, imageP, maskP, & attr) != XpmSuccess) \
        error ("can't create internal default pixmap")

    if (! File_FindInPath (path, sizeof path, PixmapPath, name))
    {
        problem ("can't find file \"%s\"", name) ;
        UseDefault () ;
    }
    else
    if (XpmReadFileToPixmap (DADisplay, DefaultRootWindow (DADisplay),
            path, imageP, maskP, & attr) != XpmSuccess)
    {
        problem ("can't load pixmap \"%s\"", path) ;
        UseDefault () ;
    }

    if (wP != NULL) *wP = attr.width ;
    if (hP != NULL) *hP = attr.height ;
}

#endif

extern void Pixmaps_LoadMenu (void)
{
    Pixmap image, mask ;
    bool saveAutoScale ;
    saveAutoScale = AutoScale ; /* save old value */
    AutoScale = false ; /* set temporary value */
    Pixmaps_FindLoad (Menu_GetPixmap (), & image, & mask, NULL, NULL) ;
    DASetShape (mask) ;
    DASetPixmap (image) ;
    AutoScale = saveAutoScale ; /* restore initial value */
}

extern void Pixmaps_LoadTile (void)
{
    int x, y ;

    if (TileImage != 0) XFreePixmap (DADisplay, TileImage) ;
    if (TileMask != 0) XFreePixmap (DADisplay, TileMask) ;

    CurrentDefault = defaultTile_xpm ;
    Pixmaps_FindLoad (TilePath, & TileImage, & TileMask, & x, & y) ;
    CurrentDefault = defaultIcon_xpm ;

    if (TileXSize <= 0) TileXSize = x ;
    if (TileYSize <= 0) TileYSize = y ;
}

extern void Pixmaps_LoadHighlight (void)
{
    int x, y ;

    if (HighlightImage != 0) XFreePixmap (DADisplay, HighlightImage) ;
    if (HighlightMask != 0) XFreePixmap (DADisplay, HighlightMask) ;

    if (HighlightPath[0] != EOS)
    {
	Pixmaps_FindLoad (HighlightPath,
	    & HighlightImage, & HighlightMask, & x, & y) ;
    }
    else
    {
	HighlightImage = 0 ;
	HighlightMask = 0 ;
    }
}

