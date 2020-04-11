---
layout: default
title: Guided Tour - Miscellaneous
---

Miscellaneous
=============

.. contents::
   :backlinks: none


Localization
------------

As soon as Window Maker is compiled with some options and gettext installed, it
is fully localizable. Check the INSTALL file.

However, localization of menus can be used without the LANG environment
variable set. Using pl menu allows to get menus in any available language
without setting this variable.

Why do such a "thing" instead of setting the localization the "right" way?

For some reasons users may want to keep the system default language instead of
defining a new localization. One of the main reason is that most software
doesn't exist in all languages.

Fonts
-----

It's possible to change the fonts in Window Maker, editing the WindowMaker file
or the WMGLOBAL file in ``~/GNUstep/Defaults``.

Once again the INSTALL file gives instructions on how to do it.

The specific file to edit varies according to the fonts to be changed.

The script *wsetfont* is provided to do the job.

Utilities
---------

Window Maker provides the user with some useful utilities.

There is a README file concerning these scripts in the util directory.

Almost each script has it's own man page recommended reading.

These utilities mainly concern the GUI: icons, styles, fonts, menus,
backgrounds.

A few of them deserve special interest as many users don't seem to know about
them.

The *wdwrite* script, for instance, writes data into the configuration files.

The *setstyle* (or *getstyle*) scripts are used to manage themes.

*Wxcopy* and *wxpaste* allows copying and pasting using the X cutbuffer.

The first one makes part of the default applications menu, in the selection
item.

For KDE users, wkdemenu.pl is worth using.

From version 0.63.0 on, a new utility is available : *wmagnify*. It allows
magnification of the area under the mouse pointer.
