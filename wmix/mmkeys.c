/* WMix -- a mixer using the OSS mixer API.
 * Copyright (C) 2014 Christophe CURIS for the WindowMaker Team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*
 * mmkeys.c: functions related to grabing the Multimedia Keys on keyboard
 */

#include <stdio.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>

#include "include/common.h"
#include "include/config.h"
#include "include/mmkeys.h"


/* The global configuration */
struct multimedia_keys mmkeys;

/* The list of keys we're interrested in */
static const struct {
	KeySym  symbol;
	KeyCode *store;
	const char *name;
} key_list[] = {
	{ XF86XK_AudioRaiseVolume, &mmkeys.raise_volume, "AudioRaiseVolume" },
	{ XF86XK_AudioLowerVolume, &mmkeys.lower_volume, "AudioLowerVolume" },
	{ XF86XK_AudioMute,        &mmkeys.mute,         "AudioMute"        }
};

/* The modifiers that should not have impact on the key grabbed */
static const struct {
	KeySym symbol;
	const char *name;
} modifier_symbol[] = {
	{ XK_Caps_Lock, "CapsLock" },
	{ XK_Num_Lock,  "NumLock"  }
};

typedef struct {
	int count;
	unsigned int list[1 << lengthof(modifier_symbol)];
} modifier_masks;

/* Local functions */
static void mmkey_build_modifier_list(Display *display, modifier_masks *result);


/*
 * Grab the multimedia keys on the X server
 *
 * That basically means that whenever these keys are pressed
 * the events will be sent to us instead of the application
 * that has current focus.
 */
void mmkey_install(Display *display)
{
	modifier_masks mod_masks;
	Window root_window;
	int i, j;

	mmkey_build_modifier_list(display, &mod_masks);

	root_window = DefaultRootWindow(display);

	for (i = 0; i < lengthof(key_list); i++) {
		KeyCode key;

		key = XKeysymToKeycode(display, key_list[i].symbol);
		*(key_list[i].store) = key;

		if (key == None)
			continue;

		for (j = 0; j < mod_masks.count; j++) {
			XGrabKey(display, key, mod_masks.list[j], root_window,
			         False, GrabModeAsync, GrabModeAsync);
		}
		if (config.verbose)
			printf("Found multimedia key: %s\n", key_list[i].name);
	}
}

/*
 * Build the list of bit-masks for all the modifiers we want to not have impact on our grab
 */
static void mmkey_build_modifier_list(Display *display, modifier_masks *result)
{
	XModifierKeymap *mods;
	KeyCode mod_code[lengthof(modifier_symbol)];
	unsigned int mod_mask[lengthof(modifier_symbol)];
	char buffer[256];
	int nb_modifiers;
	int i, j, k;

	/* Get the bitmask associated with the modifiers */
	for (i = 0; i < lengthof(modifier_symbol); i++) {
		mod_code[i] = XKeysymToKeycode(display, modifier_symbol[i].symbol);
		mod_mask[i] = 0L;
	}

	mods = XGetModifierMapping(display);
	for (i = 0; i < 8; i++) {
		for (j = 0; j < mods->max_keypermod; j++) {
			KeyCode key_mod;

			key_mod = mods->modifiermap[i * mods->max_keypermod + j];
			for (k = 0; k < lengthof(mod_code); k++) {
				if ((mod_code[k] != None) && (key_mod == mod_code[k]))
					mod_mask[k] |= 1 << i;
			}
		}
	}
	XFreeModifiermap(mods);

	/* Count the number of modifiers found (and display the list to the user) */
	if (config.verbose)
		strcpy(buffer, "Found key modifiers: ");

	nb_modifiers = 0;
	for (i = 0; i < lengthof(modifier_symbol); i++) {
		if (mod_mask[i] != 0) {
			if (config.verbose) {
				if (nb_modifiers > 0)
					strcat(buffer, ", ");
				strcat(buffer, modifier_symbol[i].name);
			}
			nb_modifiers++;
		}
	}
	if (config.verbose) {
		if (nb_modifiers == 0)
			strcat(buffer, "None");
		puts(buffer);
	}

	/* Build the list of possible combinations of modifiers */
	result->count = 1 << nb_modifiers;
	for (i = 0; i < lengthof(result->list); i++)
		result->list[i] = 0L;
	k = 1;
	for (i = 0; i < lengthof(mod_mask); i++) {
		if (mod_mask[i] != 0) {
			for (j = 1; j < result->count; j++)
				if (j & k)
					result->list[j] |= mod_mask[i];

			k <<= 1;
		}
	}
}
