/*
 * GAI - General Applet Interface Library
 * Copyright (C) 2003 Jonas Aaberg <cja@gmx.net>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * - info scanner
 *
 * The following functions try to scan an info text that
 * represents information about the applet. Such a text can be
 * embedded as a static string right in the applet code, or you
 * can load it from a file and hand it to the info set function.
 * In many cases you might want to do both, to have a set of
 * sane defaults plus the availability of changing parts for i18n.
 *
 * The info text format is designed to be able to read an rpm .spec
 * format or a makefile format - it's a list of key/pair entries
 * with the two seperated by either colon (:) or equals (=). Here
 * is an example:
 *
 * Name: my-applet
 * Version: 0.0.1
 * Summary: my first applet
 * Author: myself
 * License: Lesser GPL
 * %description
 *    This is the first applet I ever wrote and it is so simple
 *    I do not think it should be expanded any further.
 */

#define _GNU_SOURCE 1
/* we want isblank() */

#include <glib.h>
#include <ctype.h>
#include <string.h>
#include "config.h"
#include "gai.h"
#include "gai-private.h"


#ifndef isblank
#define isblank(C) ((C) == ' ' || (C) == '\t')
#endif

enum
{
    name_info = 1,
    nice_name_info = 2,
    version_info = 4,
    author_info = 8,
    license_info = 16,
    about_info = 32
};

static void
gai_scan_info(const char* p, const char* allow)
{
    const char* key; int key_len;
    const char* val; int val_len;
    int seen = 0;
    if (! p) return;

    if (allow) 
    { 
	seen = 0xFFFF;
	if (strstr(allow,"author"))  seen ^= author_info;
	if (strstr(allow,"name"))    seen ^= name_info;
	if (strstr(allow,"summary")) seen ^= nice_name_info;
	if (strstr(allow,"version")) seen ^= version_info;
	if (strstr(allow,"license")) seen ^= license_info;
	if (strstr(allow,"description")) seen ^= about_info;
	GAI_INT(seen);
    }

    /* who's afraid of goto's? */
 nextline:
    while (*p == '\n') p++;                                 /* A */
    if (! *p) return;                                       /* termination */
    while (isblank(*p)) p++;
    key = p;
    while (isalnum(*p) || strchr("%+-_",*p)) p++;
    key_len = p-key;
    while (isblank(*p)) p++;
    val = p;
    while (*p && *p != '\n') p++;                          /* B */
    val_len = p-val;                                /* (A and B assert step) */
    if (key_len <= 0) goto nextline;
    if (*key == '%') goto percent;
    if (val_len < 0) goto nextline;
    if (*val != ':' && *val != '=') goto nextline;
    val++; val_len--;

 defines:
    while (val_len && isspace(val[val_len-1])) val_len--;

    { /* a lil'bit of debugging... */
	char* tmp = g_strdup_printf (" '%.*s' = '%.*s'", 
				     key_len, key, val_len, val);
	GAI_NOTE(tmp);
	g_free (tmp);
    }

    if (!g_strncasecmp ("name",key,key_len))
    {
	if (seen&name_info) goto nextline; seen|=name_info;
	if (GAI.applet.name) g_free (GAI.applet.name);
	GAI.applet.name = g_strndup (val, val_len);
    }
    else if (!g_strncasecmp ("summary",key,key_len))
    {
	if (seen&nice_name_info) goto nextline; seen|=nice_name_info;
	if (GAI.applet.nice_name) g_free (GAI.applet.nice_name);
	GAI.applet.nice_name = g_strndup (val, val_len);
    }
    else if (!g_strncasecmp ("version",key,key_len))
    {
	if (seen&version_info) goto nextline; seen|=version_info;
	if (GAI.applet.version) g_free (GAI.applet.version);
	GAI.applet.version = g_strndup (val, val_len);
    }
    else if (!g_strncasecmp ("author",key,key_len))
    {
	if (seen&author_info) goto nextline; seen|=author_info;
	if (GAI.applet.author) g_free (GAI.applet.author);
	GAI.applet.author = g_strndup (val, val_len);
    }
    else if (!g_strncasecmp ("license",key,key_len))
    {
	if (seen&license_info) goto nextline; seen|=license_info;
	if (GAI.applet.license) g_free (GAI.applet.license);
	GAI.applet.license = g_strndup (val, val_len);
    }
    goto nextline;
 percent:
    if (!g_strncasecmp ("%define",key,key_len))
    {
	if (val_len < 0) goto nextline;
	while (val_len && isblank(*val)) { val++; val_len--; }
	key = val;
	while (val_len && isalnum(*key)) { val++; val_len--; }
	key_len = val-key;
	while (val_len && isblank(*val)) { val++; val_len--; }
	if (key_len <= 0) goto nextline;
	goto defines;
    }
    else if (!g_strncasecmp ("%description",key,key_len))
    {
	for (;*p;p++)
	{
	    if (*p == '\n' && !isspace(p[1]))
		break;
	    val_len++;
	}
	if (seen&about_info) goto nextline; seen|=about_info;
	if (GAI.applet.description) g_free (GAI.applet.description);
	GAI.applet.description = g_strndup (val, val_len);
    }	    
    goto nextline;
}

void
gai_about_from (const char* text)
{
    GAI_ENTER;  gai_is_init ();
    g_assert(text !=NULL);

    gai_scan_info (text, "summary,author,description,icon,license");
    /* these will set the GAI items just like in gai_init_about: 
     * GAI.applet.nice_name = $1 name
     * GAI.applet.author = $2 author
     * GAI.applet.description = $3 about
     * GAI.applet.icon = $4 pic
     * additionally we allow it to set
     * GAI.applet.license = $1 of gai_init_about_set_license
     */
    GAI_LEAVE;
}



