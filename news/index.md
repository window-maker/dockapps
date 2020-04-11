---
layout: default
title: News
---

News
====

Version 0.95.9 released
-----------------------

Window Maker 0.95.9 was released on April 4th 2020

* SwitchPanel is now more configurable: you can configure the switch
  panel icon size by setting the "SwitchPanelIconSize" option to your
  preferred value in ~/GNUstep/Defaults/WindowMaker. The font size used
  in this panel now is also sensible to changes in the system font.
* New user configuration directory environment variable.  In previous
  versions, the GNUstep directory used to store a user's Window Maker
  configuration files was specified by the GNUSTEP_USER_ROOT environment
  variable, which defaulted to ~/GNUstep.  However, this environment
  variable was deprecated in gnustep-make v2.  Therefore, it has been
  replaced by the WMAKER_USER_ROOT environment variable.
* libXmu is now an optional dependency.If the library is not found,
  compilation work, the only limitation will arise when trying to
  install the standard colormap on displays which are not TrueColor.
  Please note that if you have the library but not the headers,
  configure will still stop; there is no user option to explicitly
  disable the library use.

Version 0.95.8 released
-----------------------

Window Maker 0.95.8 was released on March 11th 2017.

* See the [NEWS](http://repo.or.cz/wmaker-crm.git/blob/HEAD:/NEWS) file
  and/or the git logs for an overview of the changes.

Version 0.95.7 released
-----------------------

Window Maker 0.95.7 was released on August 2nd 2015.

* Window [snapping](http://repo.or.cz/w/wmaker-crm.git/commit/df49061) feature
  has been added, which allows one to "snap" a window to one side of the screen
  by dragging it to that side (Doug Torrance).
* New mouse actions configuration were
  [added](http://repo.or.cz/w/wmaker-crm.git/commit/0d0169a) to WPrefs (David
  Maciejak).
* New button and wheel mouse
  [actions](http://repo.or.cz/w/wmaker-crm.git/commit/f40095a) (David Maciejak).
* Many code cleanups and refactoring by Christophe Curris.

Version 0.95.6 released
-----------------------

Window Maker 0.95.6 was released on August 30th 2014.

* Window Maker can now load WebP images and support ImageMagick library to
  support even more formats like SVG, BMP, TGA, ... (David Maciejak)
* Add mini-window [apercu](http://repo.or.cz/w/wmaker-crm.git/commit/c6c7652),
  a small preview of window contents (David Maciejak)
* Support for up to 9-buttons mouse added (David Maciejak)
* Many configuration options added to WPrefs.app (Doug Torrance)
* Add wmiv, an image viewer application (David Maciejak)
* Bug fixes and code cleanups by various people.

Version 0.95.5 released
-----------------------

Window Maker 0.95.5 was released on August 29th 2013.

* Window Maker can now maximize windows to the top/bottom halves of the
  screen as well as to the corners (top left, top right etc). The keyboard
  shortcuts to do that can be configured via WPrefs (Renan Traba).
* Support for [drawers](http://www.dechelotte.com/en/wmaker.php) in the dock
  has been added (Daniel Dechelotte).
* Keyboard shortcuts to move windows between workspaces (Iain Patterson).
* Window border colours and width are now configurable (Iain Patterson).
* The menu is now able to parse command-generated
  [proplist style menus](http://repo.or.cz/w/wmaker-crm.git/commit/c21ae6b).
  WPrefs support for this has been added too (Andreas Bierfert).
* Plus a few other new features and a lot of bug fixes and code cleanups by
  various people.

Version 0.95.4 released
-----------------------

Window Maker 0.95.4 was released on January 3rd 2013. There was a major code
cleanup related to icons, some changes in WPrefs, the addition of a new
"Center" placement strategy, support for _NET_FRAME_EXTENTS, the removal of CPP
dependency to process menu files and small fixes and improvements all around.

Version 0.95.3 released
-----------------------

Window Maker 0.95.3 was released on May 16th 2012. This release fixes a
regression which would cause more than one instance of an application to start
(under some circumstances) when using menu shortcuts. The window maximization
procedures now have a more intuitive behavior with respect to remembering the
old geometry and going back to it. Furthermore, there are some other small
fixes and cleanups.

Version 0.95.2 released
-----------------------

Window Maker 0.95.2 was released on February 14th 2012, and it contains just a
few commits on top of 0.95.1. They were necessary to fix a few issues like
'make dist' not compiling. Furthermore a few more code cleanups slipped in.

Version 0.95.1 released
-----------------------

Window Maker 0.95.1 was released on January 29th 2012.

The last official Window Maker release was version 0.92.0 from 2005, and
version 0.95.1 contains many bug fixes and also a few new features.

### New features and highlights

The following list is incomplete, but should give a first-order approximation
to the new features in this release. For the truly curious among you, reading
through `git log` is the only complete source of information.

* [Left Half/Right Half
  Maximize](http://repo.or.cz/w/wmaker-crm.git/commit/6924454).
* [Maximus: tiled
  maximization](http://repo.or.cz/w/wmaker-crm.git/commit/cf62d15).
  Maximizes a window such that it occupies the largest area without overlapping
  others.
* [New mouse-resizing
  functionality](http://repo.or.cz/w/wmaker-crm.git/commit/a063338).
  Windows can now be resized vertically (horizontally) using MOD+Wheel
  (CTRL+Wheel).
* [History and TAB completion in
  dialogs](http://repo.or.cz/w/wmaker-crm.git/commit/05720d9). To use this new
  functionality in your old WMRootMenu, replace %a by %A in the relevant entry.
  It will look like this `(Run..., SHEXEC, "%A(Run, Type command:)")`. Or use
  `wmgenmenu` to generate a new menu.
* [Bouncing appicon effect](http://repo.or.cz/w/wmaker-crm.git/commit/a257e16).
* New applications
  ([wmgenmenu](http://repo.or.cz/w/wmaker-crm.git/commit/1861880) and wmmenugen)
  to generate the root menu automatically by looking which applications you have
  on your $PATH. Translations to German, [Spanish and
  French](http://repo.or.cz/w/wmaker-crm.git/commit/077a2ea) of menus generated
  by wmgenmenu.
* [Automatic detection of configuration
  changes](http://repo.or.cz/w/wmaker-crm.git/commit/56d8568). Linux users whose
  kernel supports the [inotify](http://en.wikipedia.org/wiki/Inotify) mechanism
  have their configuration changes detected automatically without polling,
  reducing the number of CPU wakeups.
* [Improved dockapp
  recognition.](http://repo.or.cz/w/wmaker-crm.git/commit/9318a7f)
* And many trivial things which reduce little annoyances one might have. For
  example, an option was added to control whether or not Window Maker should do
  [automatic workspace
  switching](http://repo.or.cz/w/wmaker-crm.git/commit/d6c134f) to satisfy a
  focus request from a window located in another workspace.
* (For developers).
  The [addition](http://repo.or.cz/w/wmaker-crm.git/commit/442e387) of a debian/
  folder which allows the creation of a debian package for wmaker using the git
  sources.
* [Added keyboard shortcut to uncover/cover the
  dock](http://repo.or.cz/w/wmaker-crm.git/commit/b6689a0).
* [Mac OS X-style window
  cycling](http://repo.or.cz/w/wmaker-crm.git/commit/18408ff).
* [Preliminary XRandR
  support](http://repo.or.cz/w/wmaker-crm.git/commit/c201e16) (needs a bit more
  work to be bug-free; not compiled in by default. Use --enable-xrandr if you
  want to test it).

### Bug fixes

Window Maker 0.92.0 was already very stable, but many bugs were fixed in this
release. A **very** incomplete list is given below, and as time permits it will
be updated (including links to the commits) in the future. But the message now
is that if you don't like bugs, use version 0.95.1.

* [Fix loading saved states on 64-bit
  systems](http://repo.or.cz/w/wmaker-crm.git/commit/37829a7)
* Fix to avoid a segfault when creating more than 81 workspaces, as reported on
  youtube [here](http://www.youtube.com/watch?v=fkNJZvKwmhE).
* [Periodic focus bug](http://repo.or.cz/w/wmaker-crm.git/commit/c91bb1b).

### Summary of changes

A lot of effort was put into cleaning up the code, with lots of code removal and
tidying things up. The following output should give you an idea of the
development in the last cycle:

    git diff --shortstat wmaker-0.92.0+..wmaker-0.95.1
     592 files changed, 118361 insertions(+), 133342 deletions(-)
    git diff --shortstat 688a56e8ab67b..wmaker-0.95.1
     566 files changed, 37676 insertions(+), 41817 deletions(-)

The first shortstat is really everything, including the (huge) patch generated
in this [commit](http://repo.or.cz/w/wmaker-crm.git/commit/688a56e) from 2009,
which changed the old sources to the linux kernel coding style. The second
shortstat contains the summary of development afterwards -- but included is the
addition of a debian folder with files summing around ~20k lines. The full
diffstat for the second command can be seen [here](fulldiffstat.html).

![Info v0.95.1](/img/v0_95_1.png)
