/*
 * wmmp3
 * Copyright (c)1999 Patrick Crosby <xb@dotfiles.com>.
 * This software covered by the GPL.  See COPYING file for details.
 *
 * main.h
 *                                                                
 * Header file for main.c 
 *                                                                
 * $Id: main.h,v 1.4 1999/10/08 22:21:32 pcrosby Exp $
 */

#ifndef __MAIN_H__

#define __MAIN_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#include "buttons.h"
#include "wmgeneral.h"
#include "mpg123ctl.h"
#include "wmmp3.xpm"

#define streq(s1, s2) \
        (strcmp(s1, s2) == 0)

#endif
