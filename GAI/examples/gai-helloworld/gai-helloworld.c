
/* As simple as a GAI applet can become. */

#include "config.h"
#include <gai/gai.h>

int main(int argc, char **argv)
{
    GdkPixbuf *bg;
    
    gai_init2(&applet_defines, &argc, &argv);
    bg = gai_text_create("Hello world!", "arial", 12, 
                         GAI_TEXT_NORMAL, 0, 0, 0);
    gai_background_from_gdkpixbuf(bg, GAI_BACKGROUND_MAX_SIZE_IMAGE);
    g_object_unref(bg);
    gai_start();
    return 0;
}
