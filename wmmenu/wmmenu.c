#include <stdio.h>
#include <stdlib.h>

#include <dockapp.h>

#include "options.h"
#include "menu.h"
#include "version.h"
#include "pixmaps.h"
#include "buttonbar.h"
#include "events.h"

extern int main (int argc, char ** argv)
{
    Options_ParseDefaults () ;
    Options_Parse (argc, argv) ;
    Menu_LoadFromFile (MenuName) ;

    DAParseArguments (Options_Argc, Options_Argv, NULL, 0,
        "wmmenu", VERSION) ;
    DAOpenDisplay (NULL, argc, argv) ;
    DACreateIcon((char*)Menu_GetTitle (), 48, 48, argc, argv);

    Pixmaps_LoadMenu () ;
    Pixmaps_LoadTile () ;
    /* needs tile to be loaded before to have autoscale work */
    Pixmaps_LoadHighlight () ;
    /* bar build needs highlight mask and menu icons */
    ButtonBar_Build () ;
    Events_SetCallbacks () ;

    DAShow () ;
    Events_Loop () ;

    return EXIT_SUCCESS ;
}

