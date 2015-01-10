/*  File:     wmcalc_c.h
 *  Author:   Edward H. Flora <ehflora@access1.net>
 *  Version:  0.2
 *
 *  Description:
 *  This file contains the constants settings for the wmcalc program,
 *  except for the error constants, which are contained in the file wmcalc_err.h
 *
 *  Change History:
 *  Date       Modification
 *  10/25/00   Original file creation, extracted from wmcalc.h
 */
#ifndef WMCALC_C_H
#define WMCALC_C_H

#define CONFIGFILEMAX 128
#define CALC_CMD_SIZE 128
#define CONFFILENAME  "/.wmcalc"
#define CONFTEMPFILE  "/tmp/wmcalc.tmp"
#define CONFIGGLOBAL  CONF"/wmcalc.conf"
#define VER           0
#define REL           5

#define LMASK         100
#define MMASK         200
#define RMASK         300

#define CALCDONE      20        /* Anything >= 10 should work */
#define DISPSIZE      10        /* Number of characters in display */
#define NUM_BUTTONS   21
#define NUM_MEM_CELLS 10

#define APP_WIDTH     64
#define APP_HEIGHT    64

#define NO_BUTTON     -1

#endif
