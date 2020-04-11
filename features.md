---
layout: default
title: Features
---

Features
========

Window Maker firmly adheres to the behavior and functionality of the
[NeXTSTEP](http://en.wikipedia.org/wiki/NeXTSTEP)&trade; user interface. The
developers have put forth a great deal of effort in capturing the essence and
beauty of the original design, and have incorporated some new ideas of their
own. This has always followed the philosophy of keeping to those features which
fit well into the overall design, while limiting the amount of "feature creep"
that tends to bloat other window managers. A summary of the main Window Maker
features are presented below.

Core (usability)
----------------

* Almost complete [ICCM](http://en.wikipedia.org/wiki/Icccm) compliance.
* National language I18N support (over 11 locales).
* Built-in icon dithering with support for 4bpp and 8bpp displays.
* Popup menus that support keyboard traversal, which can be "pinned" to the
  root window.
* Support for [GNUstep](http://gnustep.org), [GNOME](http://gnome.org), and
  [KDE](http://kde.org) window hints to better integrate with those desktop
  environments.
* Support for Motif&trade; and OPEN LOOK&trade; window hints to better
  interface with applications based on those toolkits.
* Built-in GUI configuration utility that eliminates the need to hand edit
  config files.
* Application [Dock](http://en.wikipedia.org/wiki/Dock_(computing)) (similar to
  NEXTSTEP/MacOS X Dock) that can be configured using drag and drop.
* Workspace Dock (aka Clip/Fiend) which is a workspace specific Dock extender.
* Support for rudimentary session management.
* Support for [dockapps](http://en.wikipedia.org/wiki/Dockapps) (equivalent of
  applets or epplets).
* Ability to change all preferences and menus on-the-fly without having to
  restart the window manager.
* Support for multiple workspaces (aka "virtual desktops").
* Multiple display support ([Xinerama](https://en.wikipedia.org/wiki/Xinerama)
  and [XRandR](http://en.wikipedia.org/wiki/RandR) extensions)
* Ability to maximize windows in half left/right/top/bottom of the screen and
  also quarters.
* Ability to display minimized window content as small preview (apercu).
* Up to 9 buttons mouses support.
* And more. You can go either through the git
  [log](https://repo.or.cz/wmaker-crm.git/shortlog) and/or
  [NEWS](https://repo.or.cz/wmaker-crm.git/blob/refs/heads/master:/NEWS) file.

Extras (eye candy)
------------------

* Built-in themes support.
* Over 13 types of window decorations, including custom defined.
* Support for XPM, PNG, JPEG, TIFF, GIF and PPM icons (no conversions with
  external programs) with an alpha-channel.
* Additional format, which [ImageMagick](https://www.imagemagick.org) supports,
  will be accessible if compiled with ImageMagick support.
* Support for setting the root window background (via the wmsetbg utility). Even
  differnt one per workspace.
* Optional superfluous animations, such as window shading, customizeable icon
  miniaturization effects, slide/scrolling menus, and much more.

Despite all of these features, Window Maker is not resource intensive and
remains stable across many UNIX varients. It is extremely flexible, and many
options can be included or excluded at compile time. This means that you can
easily tailor Window Maker to meet your needs; whether you are a minimalist
wanting to save resources, or an extremist that likes to theme everything in
sight. A screenshot of a typical Window Maker workspace is
[here](img/wmaker-screenshot.png).
