---
layout: default
title: Compilation and Installation
---


=========================================
Window Maker Compilation and Installation
=========================================

A guide to configure, compile and install WINDOW MAKER from sources.

.. contents:: Table of Contents

This manual is for Window Maker, version git#next.

----

Prerequisites
-------------

Supported Platforms
...................

- Intel GNU/Linux Systems in general, `ix86` and `x86_64` but other
  architectures should work
- BSD systems
- Solaris, at least on release 10 and 11

Patches to make it work on other platforms are welcome.

Software Dependencies
.....................

The following software is required to use WINDOW MAKER:

- X11R6.x

  Window Maker can be compiled in older versions of *X*, like
  *X11R5* (*Solaris*) or *X11R4* (*OpenWindows*) but
  it will not work 100% correctly.  In such servers there will not be
  application icons and you'll have trouble using the dock.  Upgrading the
  client libraries (*Xlib*, *Xt*, etc.) will help if you
  can't upgrade the server.

The following is required to build WINDOW MAKER:

- Basic obvious stuff
  - *gcc* (or some other ANSI C compiler, supporting some C99 extensions)
  - *glibc* development files (usually ``glibc-devel`` in Linux distributions)
  - *X* development files (``XFree86-devel`` or something similar)

- *Xft2* and its dependencies

  Dependencies include *freetype2* and *fontconfig*.  You will also need the
  development files for them (``xft2-devel``).  Sources are available at:
  http://www.freedesktop.org/wiki/Software/Xft

**Note**: WINDOW MAKER is known to compile with *gcc* and *clang*; the code
source is mostly ANSI C (also known as C89 and C90) but is uses very few of the
C99 novelties; it also uses a few attributes introduced in the C11 standard but
those are detected automatically, so most compilers should work.

Special Dependencies
....................

If you want to compile using the sources from the git repository instead of the
distribution package, you will also need:

- *git*
- *autoconf* 2.69
- *automake* 1.12
- *libtool* 1.4.2

Optional Dependencies
.....................

These libraries are not required to make <small>WINDOW MAKER</small> work, but
they are supported in case you want to use them. Version numbers are indicative,
but other versions might work too.

- *libXPM* 4.7 or newer

  Older versions may not work!

  Available from http://xlibs.freedesktop.org/release

  There is built-in support for <em>XPM</em> files, but it will not load images
  in some uncommon encodings.

- *libpng* 0.96 or newer and *zlib*

  For *PNG* image support, http://www.libpng.org/pub/png/libpng.html

- *libtiff* 3.4 or newer

  For *TIFF* image support, http://www.libtiff.org/


- *libjpeg* 6.0.1 or newer

  For *JPEG* image support, http://www.ijg.org/

  Note that if you don't have it, ``configure`` will issue a big warning in the
  end, this is because JPEG images are often used in themes and for background
  images so you probably want this format supported.

- *libgif* 2.2 or *libungif*

  For *GIF* image support, http://giflib.sourceforge.net/

- *WebP* 0.4.1 or newer

  The reference library from *Google* for their image format,
  https://developers.google.com/speed/webp/download

- *GNU xgettext*

  If you want to use translated messages, you will need *GNU gettext*.
  Other versions of *gettext* are not compatible and will not work. Get
  the *GNU* version from http://www.gnu.org/software/gettext/

- *Pango* 1.36.8 or newer

  This library can be used by the *WINGs* toolkit to improve support for
  *UTF-8* and for languages written in right-to-left direction, in some
  widgets. You have to explicitly ask for its support through (see `Configure
  Options <#configure-options>`__). You can get it from
  http://www.pango.org/Download

- *libbsd*

  This library can be used by the *WINGs* utility library to make use of
  ``strlcat`` and ``strlcpy`` instead of using built-in functions if your system
  does not provide them in its core *libc*. You should let WINDOW MAKER's
  ``configure`` detect this for you. You can get it from
  http://libbsd.freedesktop.org/wiki/

- *Inotify*

  If you have Linux's *inotify* support, WINDOW MAKER will use it to check for
  configuration updates instead of polling regularly the file. The needed header
  comes with the kernel, typical packages names include:

  - ``kernel-headers`` for *Slackware* and *Fedora*
  - ``linux-userspace-headers`` for *Mageia*
  - ``linux-libc-dev`` for *Debian* and *Ubuntu*
  - ``linux-glibc-devel`` for *OpenSuSE*

- *MagickWand* 6.8.9-9 or newer

  If found, then the library *WRaster* can use the *ImageMagick* library to let
  WINDOW MAKER support more image formats, like *SVG*, *BMP*, *TGA*, ... You
  can get it from http://www.imagemagick.org/

- *Boehm GC*

  This library can be used by the *WINGs* utility toolkit to use a
  *Boehm-Demers-Weiser Garbage Collector* instead of the traditional
  ``malloc``/``free`` functions from the *libc*. You have to explicitly ask for
  its support though (see `Configure Options <#configure-options>`__). You can
  get it from http://www.hboehm.info/gc/

----

Building WINDOW MAKER
---------------------

Getting the Sources
...................

The latest version of WINDOW MAKER (``-crm``) can be downloaded from
http://www.windowmaker.org/

Alternatively, the development branch, called ``#next`` is in the *git*
repository at http://repo.or.cz/w/wmaker-crm.git

If you want to use the *git* versions, you can get it with:

.. code:: console
   :class: highlight

   git clone -b next git://repo.or.cz/wmaker-crm.git

then, assuming you have the dependencies listed in `Special Dependencies
<#special-dependencies>`__, you have to type:

.. code:: console
   :class: highlight

   ./autogen.sh

to generate the configuration script.


Build and Install

For a quick start, type the following in your shell prompt:

.. code:: console
   :class: highlight

   ./configure
   make

then, login as *root* and type:

.. code:: console
   :class: highlight

   make install
   ldconfig

or if you want to strip the debugging symbols from the binaries to make them
smaller, you can type instead:

.. code:: console
   :class: highlight

   make install-strip
   ldconfig

This will build and install WINDOW MAKER with default parameters.

If you want to customise some compile-time options, you can do the following:

1. (optional) Look at the `Configure Options <#configure-options>`__, for the
   options available. Also run:

   .. code:: console
      :class: highlight

      ./configure --help

   to get a complete list of options that are available.


1. Run configure with the options you want. For example, if you want to use the
   ``--enable-modelock`` option, type:

   .. code:: console
      :class: highlight

      ./configure --enable-modelock

1. (optional) Edit ``src/wconfig.h`` with your favourite text editor and browse
   through it for some options you might want to change.

1. Compile. Just type:

   .. code:: console
      :class: highlight

      make

1. Login as root (if you can't do that, read the [I don't have the
   *root*](#No-Root-Password)) and install WINDOW MAKER in your system:

   .. code:: console
      :class: highlight

      su root
      make install

User specific configuration
...........................

These instructions do not need to be followed when upgrading WINDOW MAKER
from an older version, unless stated differently in the *NEWS* file.

Every user on your system that wishes to run WINDOW MAKER must do the
following:

1. Install Window Maker configuration files in your home directory. Type:

   .. code:: console

      wmaker.inst

   ``wmaker.inst`` will install WINDOW MAKER configuration files and will setup
   X to automatically launch WINDOW MAKER at startup.

That's it!

You can type ``man wmaker`` to get some general help for configuration and
other stuff.

Read the *User Guide* for a more in-depth explanation of WINDOW MAKER.

You might want to take a look at the *FAQ* too.

Locales/Internationalisation
............................

WINDOW MAKER has national language support. The procedure to enable national
language support is described in the dedicated
`Enabling Languages support <wmaker_i18n.html#enabling-languages-support>`__
in ``README.i18n``.

Configure Options
.................

These options can be passed to the configure script to enable/disable some
WINDOW MAKER features. Example:

.. code:: console
   :class: highlight

   ./configure --enable-modelock --disable-gif

will configure WINDOW MAKER with *modelock* supported and disable *gif* support.
Normally, you won't need any of them.

To get the list of all options, run ``./configure --help``

Installation Directory
''''''''''''''''''''''

The default installation path will be in the ``/usr/local`` hierarchy; a number of
option can customise this:


- ``--prefix=PREFIX``
- ``--exec-prefix=EPREFIX``
- ``--bindir=DIR``
- ``--sysconfdir=DIR``
- ``--libdir=DIR``
- ``--includedir=DIR``
- ``--datarootdir=DIR``
- ``--datadir=DIR``
- ``--localedir=DIR``
- ``--mandir=DIR``
   Standard options from *autoconf* to define target paths, you probably want to
   read Installation Names in *`INSTALL`*.

- ``--sbindir=DIR``
- ``--libexecdir=DIR``
- ``--sharedstatedir=DIR``
- ``--localstatedir=DIR``
- ``--oldincludedir=DIR``
- ``--infodir=DIR``
- ``--docdir=DIR``
- ``--htmldir=DIR``
- ``--dvidir=DIR``
- ``--pdfdir=DIR``
- ``--psdir=DIR``
   More standard options from *autoconf*, today these are not used by WINDOW
   MAKER; they are provided automatically by *autoconf* for consistency.

- ``--with-gnustepdir=PATH``
   Specific to WINDOW MAKER, defines the directory where WPrefs.app will be
   installed, if you want to install it like a *GNUstep* applications. If not
   specified, it will be installed like usual programs.

- ``--with-pixmapdir=DIR``
   Specific to WINDOW MAKER, this option defines an additional path where
   *pixmaps* will be searched. Nothing will be installed there; the default
   path taken is ``DATADIR/pixmaps``, where ``ATADIR` is the path defined from
   ``--datadir``.

- ``--with-defsdatadir=DIR``
   Specific to WINDOW MAKER, defines the directory where system configuration
   files, e.g., ``WindowMaker``, ``WMRootMenu``, etc., are installed. The
   default value is ``SYSCONFDIR/WindowMaker``, where ``SYSCONFDIR`` is the
   path defined from ``--sysconfdir``.


External Libraries
''''''''''''''''''

Unless specifically written, ``configure`` will try to detect automatically for
the libraries; if you explicitly provide ``--enable-FEATURE`` then it will
break with an error message if the library cannot be linked; if you specify
``--disable-FEATURE`` then it will not try to search for the library. You can
find more information about the libraries in the `Optional Dependencies
<#Optional-Dependencies>`__


``--enable-boehm-gc``

   Never enabled by default, use Boehm GC instead of the default *libc*
   ``malloc()``

``--disable-gif``

   Disable GIF support in *WRaster* library; when enabled use ``libgif`` or
   ``libungif``.

``--disable-jpeg``

   Disable JPEG support in *WRaster* library; when enabled use ``libjpeg``.

``--without-libbsd``

   Refuse use of the ``libbsd`` compatibility library in *WINGs* utility
   library, even if your system provides it.

``--disable-magick``

   Disable *ImageMagick's MagickWand* support in *WRaster*, used to support for
   image formats.

``--enable-pango``

   Disabled by default, enable *Pango* text layout support in *WINGs*.

``--disable-png``

   Disable PNG support in *WRaster*; when enabled use ``libpng``.

``--disable-tiff``

   Disable TIFF support in *WRaster*. when enabled use ``libtiff``.

``--disable-webp``

  Disable WEBP support in *WRaster*. when enabled use ``libwebp``.

``--disable-xpm``

   Disable use of ``libXpm`` for XPM support in *WRaster*, use internal code
   instead.

The following options can be used to tell ``configure`` about extra paths that
needs to be used when compiling against libraries:

``--with-libs-from``

   specify additional paths for libraries to be searched. The ``-L`` flag must
   precede each path, like:

   .. code::
      :class: highlight

      --with-libs-from="-L/opt/libs -L/usr/local/lib"

``--with-incs-from``

   specify additional paths for header files to be searched. The ``-I`` flag
   must precede each paths, like:

   .. code::
      :class: highlight

      --with-incs-from="-I/opt/headers -I/usr/local/include"

X11 and Extensions
''''''''''''''''''

``configure`` will try to detect automatically the compilation paths for X11
headers and libraries, and which X Extensions support can be enabled.  if you
explicitly provide ``--enable-FEATURE`` then it will break with an error
message if the extension cannot be used; if you specify ``--disable-FEATURE``
then it will not check for the extension.

- ``--x-includes=DIR``
- ``--x-libraries=DIR``

   *Autoconf*'s option to specify search paths for *X11*, for the case were it
   would not have been able to detect it automatically.

``--disable-xlocale``

   If you activated support for Native Languages, then *X11* may use a hack to
   also configure its locale support when the program configure the locale for
   itself.  The ``configure`` script detects if the *Xlib* supports this or
   not; this options explicitly disable this initialisation mechanism.

``--enable-modelock``

   XKB language status lock support. If you don't know what it is you probably
   don't need it. The default is to not enable it.

``--disable-shm``

   Disable use of the *MIT shared memory* extension. This will slow down
   texture generation a little bit, but in some cases it seems to be necessary
   due to a bug that manifests as messed icons and textures.

``--disable-shape``

   Disables support for *shaped* windows (for ``oclock``, ``xeyes``, etc.).

``--enable-xinerama``

   The *Xinerama* extension provides information about the different screens
   connected when running a multi-head setting (if you plug more than one
   monitor).

``--enable-randr``

   The *RandR* extension provides feedback when changing the multiple-monitor
   configuration in X11 and allows to re-configure how screens are organised.

   At current time, it is not enabled by default because it is NOT recommended
   (buggy); WINDOW MAKER only restart itself when the configuration change, to
   take into account the new screen size.

Feature Selection
'''''''''''''''''


``--disable-animations``

   Disable animations permanently, by not compiling the corresponding code into
   WINDOW MAKER. When enabled (the default), you still have a run-time
   configuration option in *WPrefs*.

``--disable-mwm-hints``

   Disable support for Motif's MWM Window Manager hints. These attributes were
   introduced by the Motif toolkit to ask for special window appearance
   requests.  Nowadays this is covered by the NetWM/EWMH specification, but
   there are still applications that rely on MWM Hints.

``--enable-wmreplace``

   Add support for the *ICCCM* protocol for cooperative window manager
   replacement. This feature is disabled by default because you probably don't
   need to switch seamlessly the window manager; if you are making a package
   for a distribution you'd probably want to enable this because it allows
   users to give a try to different window managers without restarting
   everything for an extra cost that is not really big.

``--disable-xdnd``

   Disable support for dragging and dropping files on the dock, which launches
   a user-specified command with that file. Starting from version 0.65.6 this
   feature is enabled by default.

``--enable-ld-version-script``

   This feature is auto-detected, and you should not use this option. When
   compiling a library (``wrlib``, ...), *gcc* has the possibility to filter
   the list of functions that will be visible, to keep only the public API,
   because it helps running programs faster.

   The ``configure`` script checks if this feature is available; if you specify
   this option it will not check anymore and blindly trust you that it is
   supposed to work, which is not a good idea as you may encounter problems
   later when compiling.

``--enable-usermenu``

   This feature, disabled by default, allows to add a user-defined custom menu
   to applications; when choosing an entry of the menu it will send the key
   combination defined by the user to that application. See <a
   href="http://repo.or.cz/wmaker-crm.git/blob/HEAD:/NEWS">Application User
   Menu</a> in *NEWS* for more information.

``--with-menu-textdomain=DOMAIN``

   Selection of the domain used for translation of the menus; see `Translations
   for Menus <wmaker_i18n.html#Translations-for-Menus>`__ in *README.i18n*.


Developer Stuff
'''''''''''''''

These options are disabled by default:

``--config-cache``

   If you intend to re-run the ``configure`` script often, you probably want to
   include this option, so it will save and re-use the status of what have been
   detected in the file ``config.cache``.


``--enable-debug``

   Enable debugging features (debug symbol, some extra verbosity and checks)
   and add a number of check flags (warnings) for the compiler (in *gcc*
   fashion).

``--enable-lcov=DIRECTORY``

   Enable generation of code coverage and profiling data; if the ``DIRECTORY``
   is not specified, use ``coverage-report``.

   This option was meant to be use with *gcc*; it was not used recently so it
   is probable that is does not work anymore; the ``configure`` script will not
   even check that your compiling environment has the appropriate requirements
   and works with this.  Despite all this, if you think there's a use for it
   and feel in the mood to help, do not hesitate to discuss on the mailing list
   `wmaker-dev@lists.windowmaker.org
   <mailto:wmaker-dev@lists.windowmaker.org>`__ to get it working.

Miscellaneous
-------------

Platform Specific Notes
.......................

- *GNU/Linux* in general

  Make sure you have ``/usr/local/lib`` in ``/etc/ld.so.conf`` and that you run
  ``ldconfig`` after installing. Uninstall any packaged version of WINDOW MAKER
  before installing a new version.

- *RedHat GNU/Linux*

  *RedHat* systems have several annoying problems. If you use it, be sure to
  follow the steps below or WINDOW MAKER will not work:


  - if you installed the WINDOW MAKER that comes with *RedHat*, uninstall it
    before upgrading;

  - make sure you have ``/usr/local/bin`` in your ``PATH`` environment variable;

  - make sure you have ``/usr/local/lib`` in ``/etc/ld.so.conf`` before running ``ldconfig``;

- *PowerPC MkLinux*

  You will need to have the latest version of *Xpmac*. Older versions seem to
  have bugs that cause the system to hang.

- *Debian GNU/Linux*

  If you want *JPEG* and *TIFF* support, make sure you have ``libtiff-dev``
  and ``libjpeg-dev`` installed.

- *SuSE GNU/Linux*

  If you installed the WINDOW MAKER package from *SuSE*, uninstall it before
  trying to compile *Window Maker* or you might have problems.

- *MetroX* (unknown version)

  *MetroX* has a bug that corrupts pixmaps that are set as window backgrounds.
  If you use *MetroX* and have weird problems with textures, do not use
  textures in title bars. Or use a different X server.

I don't have the *root* password :(
...................................

If you can't get superuser privileges (can't be *root*) you can install *Window
Maker* in your own home directory. For that, supply the ``--prefix`` option
when running configure in step 2 of building WINDOW MAKER.  You will also need
to supply the ``--with-gnustepdir`` option, to specify the path for
``WPrefs.app``.

Example:

.. code:: console
   :class: highlight

   ./configure --prefix=/home/jshmoe --with-gnustepdir=/home/jshmoe/GNUstep/Applications

Then make ``/home/jshmoe/bin`` be included in your search ``PATH``, add
``/home/jshmoe/lib`` to your ``LD_LIBRARY_PATH`` environment variable and run
``bin/wmaker.inst``

Of course, ``/home/jshmoe`` is supposed to be replaced by your actual home
directory path.


Upgrading
.........

If you are upgrading from an older version of WINDOW MAKER:

#. Configure and build WINDOW MAKER as always
#. Install WINDOW MAKER (but do not run ``wmaker.inst``)
#. Read the *NEWS* file and update your configuration files if necessary.

----

Troubleshooting
---------------

When you have some trouble during configuration (while running configure), like
not being able to use a graphic format library you think you have installed,
look at the ``config.log`` file for clues of the problem.

Error with loading fonts, even if they exist
............................................

This is probably a problem with NLS (Native Language Support), you probably
want to look at the `Troubleshooting <wmaker_i18n.html#Troubleshooting>`__ in
*README.i18n* or try rebuilding without NLS support, which is done with:


.. code:: console
   :class: highlight

   ./configure LINGUAS=""

configure doesn't detect *libtiff*, or other graphic libraries
..............................................................

Delete ``config.cache``, then rerun configure adding the following options to
``configure`` (among the other options you use):

.. code:: console
   :class: highlight

   --with-libs-from="-L/usr/local/lib"
   --with-incs-from="-I/usr/local/include -I/usr/local/include/tiff"

Put the paths where your graphic libs and their corresponding header files are
located. You can put multiple paths in any of these options, as the example of
``--with-incs-from`` shows. Just put a space between them.

configure doesn't detect *libXpm*
.................................

Check if you have a symbolic link from ``libXpm.so.4.9`` to ``libXpm.so``

Segmentation fault on startup

- Check if the version of *libXPM* you have is at least 4.7
- Check if you have an updated version of ``~/GNUstep/Defaults/WindowMaker``

If you're not sure, try renaming ``~/GNUstep`` to ``~/GNUtmp`` and then run
``wmaker.inst``

"...: your machine is misconfigured. gethostname() returned (none)"
...................................................................

the host name of your machine is set to something invalid, that starts with a parenthesis.
Do a ``man hostname`` for info about how to set it.


The root menu contains only 2 entries. ("XTerm" and "Exit...")
..............................................................

WINDOW MAKER could not read your menu definition file. You should check the
output of ``wmaker`` for an error, it may be visible in the console or in the
``.xsession-errors`` file.
