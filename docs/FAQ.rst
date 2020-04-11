---
layout: default
title: FAQ
---


FAQ
===

Have questions about Window Maker? If so, look no further. Below is our
collection of Frequently Asked Questions and their corresponding answers. Many
of these have been adapted from the `original FAQ
<http://web.archive.org/web/20030401182339/http://www.dpo.uab.edu/~grapeape/wmfaq.html>`_
by Chris Green. Questions are routinely taken and added in from the mailing
lists and IRC forums.

.. sectnum::
.. contents:: Table of Contents
   :backlinks: none

----

Introduction to Window Maker
----------------------------

What is Window Maker?
.....................

Window Maker is an X11 window manager originally designed to provide
integration support for the GNUstep Desktop Environment.  In every way
possible, it reproduces the elegant look and feel of the NEXTSTEP[tm] user
interface. It is fast, feature rich, easy to configure, and easy to use. It is
also free software and part of the GNU Project, with contributions being made
by programmers from around the world

Where can I get Window Maker?
.............................

Window Maker can be obtained from the official website, http://windowmaker.org/,
or from various mirror sites listed at http://windowmaker.org/mirrors.html

Where are the mailing lists and archives?
.........................................

All information regarding the Window Maker
mailing lists can be found at http://windowmaker.org/lists.html

Where can I find more documentation?
....................................

Additional documentation can be found in the Window Maker source distribution,
or at http://windowmaker.org/documentation.html

What is an appicon?
...................

An appicon is the icon produced by an application that initially is in the
bottom left corner of the screen while an application is running. For an
example, run xterm and notice the icon in the corner (make sure that you use
xterm and not a default rxvt when testing, because many versions of rxvt do not
properly set their window attributes).

.. TODO (fix link)

For a more indepth discussion of how an appicon relates to Window Maker, see
question 1.10

How can I get a question added to the FAQ?
..........................................

For now, the best method is to E-mail your question to faq@windowmaker.org. We
are working on a web-based submission form to our FAQ system, which will enable
users to submit questions for review.

How do I report bugs?
.....................

.. TODO link to the bugform?
.. TODO wrong url for bugtracker (we don;t have any!)

You can look at the BUGFORM file in the source distribution of Window Maker.
Alternatively, you can use the Window Maker Bug Tracker at
http://windowmaker.org/cgi-bin/bugs

Is there an anomymous cvs server?
.................................

Yes there is.  To check out from cvs, first

.. code:: console
   :class: highlight

   export CVSROOT=":pserver:anoncvs@cvs.windowmaker.org:/cvsroot"
   cvs login

There is no password, so simply hit enter when prompted.

Then issue the following command ("wm" is the name of the module):

.. code:: console
   :class: highlight

   cvs -z3 checkout -d WindowMaker wm

To update your source tree, cd to the WindowMaker directory and type

.. code:: console
   :class: highlight

   cvs -z3 update -dP

inside the WindowMaker directory.

For more detailed CVS instructions, please visit
http://windowmaker.org/development-cvs.html

Where can I find the Window Maker IRC channel?
..............................................

.. TODO change irc server to freenode

The official Window Maker IRC channel can be accessed by connecting to
irc.windowmaker.org on port 6667, and joining #WindowMaker

What is the difference between appicons, mini-windows, and minimized applications?
..................................................................................

Thanks to Jim Knoble for this answer:

Many window managers are capable of turning large windows into smaller *icons*
which represent the window yet don't take as much screen real estate.  We're
all familiar with that model.

Window Maker has two kinds of these icons. One kind is created when an
application - technically, a window group - is started. It represents the
entire application and is called an *appicon*. Such icons are square tiles
containing only the picture which represents the application; they have no
titles.

The second kind of icon in Window Maker is created when a particular window
(possibly one belonging to an application displaying more than one window) is
*miniaturized* (which is the same action as *minimizing* or *iconifying* in
other window management models) using the miniaturization button on the
window's titlebar. These miniaturized windows are called *miniwindows* and can
normally be distinguished from appicons by their small titlebar at the top of
the tile.

How do I make sense of Window Maker's version number scheme?
............................................................

The numbering scheme is relatively simple, and is in the format of three
numbers separated by dots. The first number is the "major" revision number.
The second is the "minor" revision number. And finally, the third is the "patch
level" number.

To put this all into perspective, let's examine the version number "0.65.1".
This number signifies that there has not been a major revision release, that
its minor revision is newer than the previous one (0.64.x), and that it's on
the first patch level after the 0.65.0 release. This still might be confusing,
so go away with this in mind: numbers ending in .0 tend to be new feature
releases but less stable than .1, .2, .3 patch level releases, the latter of
which are used to fix bugs.

It is generally safe to go with the highest numbered patch release.

----


Installing Window Maker
-----------------------

Why are no icons showing up after installing Window Maker?
..........................................................

As of WindowMaker version 0.15.0, the default setup includes .tiff icons which
require you to have compiled Window Maker with libtiff support. For assistance
on compiling libtiff, see the following question.

How do I make Window Maker link against libtiff?
................................................

Many UNIX operating systems have difficulty finding third party libraries by
default. Unfortunately, there are too many of these to include instructions for
them all.

In general, you will want to ensure the latest version of libtiff is installed
(see ftp://www.libtiff.org). Typically on non-Linux systems, libtiff will be
located under /usr/local, with includes and libs in those respective
sub-directories.


Often, it will be necessary to add /usr/local/lib to the system's
LD_LIBRARY_PATH environment variable (especially so on Solaris, but see 'man
ld' for details on your platform). Furthermore, it is possible to supply
special flags to the configure script to help it find where the libraries are.
An example is given below:

.. code:: console
   :class: highlight

   ./configure --with-libs-from="-L/usr/local/lib" \
      --with-incs-from="-I/usr/local/include"

Also, you will want to make sure you're using GNU make (gmake) for the Window
Maker compile.

How do I switch CDE's window manager to use WindowMaker?
........................................................

Method 1:
'''''''''

Peter Ilberg gives us this answer:

Install WM wherever you want it, mine is in /opt/WindowMaker-0.16.0 (eg. use
./configure --prefix=/opt/WindowMaker-0.16.0). Run the install script
wmaker.inst in your home directory.

Add the following two lines to .dtprofile in your home directory:

.. code:: console
   :class: highlight

   SESSIONTYPE=xdm; export SESSIONTYPE
   PATH=:/usr/contrib/bin/X11:$PATH:.; export PATH

This tells CDE to go looking for an .xinitrc/.xsession instead of using the
default environment.

Make your .xsession/.xinitrc executable (VERY IMPORTANT, wmaker.inst did NOT do
this automatically for me) using eg.

.. code:: console
   :class: highlight

   chmod ugo+x .xsession

Your .xsession/.xinitrc should look something like this:

.. code:: bash
   :class: highlight

   #!/bin/sh

   <some other init stuff that you want/need>
   exec wmaker

Things to try if it doesn't work: (somewhat fuzzy and random)

This should do it although I did have problems sometimes initially which I
fixed by randomly trying absolute pathes for wmaker in .xsession/.xinitrc
and/or making the dtprofile/.xinitrc/etc executable. It helps logging in on the
console (select from CDE login screen) and start X manually using "X". If it
works that way it should work when logging into the CDE environment. Remember
to Check your paths!

If it doesn't work, you can also substitute some other window manager for
wmaker in the .xinitrc and see if that works. If it does you know at least that
.xinitrc is getting called/executed, so your WM path is wrong or not set.

Method 2:
'''''''''

Thomas Hanselman gave this alternative answer (via Peter Ilberg):

Build and install WM wherever you want, as described in Method 1. You can
install and run WM just fine from your home directory. That's what I'm doing,
since I don't have root access at work :(. Then, in your Xdefaults file in your
home directory, add the following line:

.. code:: console
   :class: highlight

   Dtsession*wmStartupCommand: <path to WindowMaker executable>

Then, log out, and log back in, and, unless I've forgotten a step (or this is a
custom Nortel thing), you should be in Window Maker heaven ;).

Difference between the methods: (according to Thomas)
'''''''''''''''''''''''''''''''''''''''''''''''''''''

I've been told that the difference between setting the resource and Peter's
method is that if you override the window manager with the resouce, you still
get the CDE resources read into the resource database (so you still have your
color settings & such from CDE), whereas with Peter's, the CDE resource
don't get read into the database. I don't know if this is true or not, however.
Also, another thing to note with Window Maker and HP-UX 10.20 - if you select
"Exit Session" from the WM root menu, WindowMaker and all of your applications
are killed, but you may not be logged out. Again, this might be an artifact
from my work environment, or the way I start Window Maker.

Owen Stenseth adds:
'''''''''''''''''''

When using this method it is possible to exit Window Maker cleanly by using the
dtaction command. I use the following in my Window Maker menu:

.. code::
   :class: highlight

   "Exit Session"      EXEC dtaction ExitSession

The only problem I have at the moment is I seem to get multiple copies of
asclock running when I log in again.

Do I need to rerun wmaker.inst with every new version of Window Maker?
......................................................................

Dan Pascu reveals the answer:

If this is necessary, it will be listed in the NEWS file included in the source
distribution. Since 0.15.x, the domain files have been changed in such a way
that re-running wmaker.inst is redundant. The user config files are by default
merged in with the global ones normally located in
/usr/local/share/WindowMaker/Defaults. So, even if new options are added, they
should be automatically added to the environment.

Why am I only getting a root menu with xterm and exit items?
............................................................

Most likely, the problem is that Window Maker can not find a copy of the C pre
processor in a directory such as /lib. The file /lib/cpp should be a symbolic
link to whatever C compiler's cpp you are using. For example:

.. code:: shell-session
   :class: highlight

   $ file `which cpp`
   /usr/bin/cpp link to /usr/bin/cpp-2.95

Another possibility is your /usr/X11/lib/X11/xinit/xinitrc is a broken symlink.
Either create a new symlink, or do something like:

.. code:: shell-session
   :class: highlight

   $ cp /usr/X11/lib/X11/xinit/xinitrc.fvwm2 \
      /usr/X11/lib/X11/xinit/xinitrc.wmaker
   $ ln -sf /usr/X11/lib/X11/xinit/xinitrc.wmaker \
      /usr/X11/lib/X11/xinit/xinitrc

then just edit /usr/X11/lib/X11/xinit/xinitrc and replace the exec of 'fvwm2'
by '/usr/local/bin/wmaker' (should be somewhere towards the end of the file,
most probably the very last line).

Thanks to Tomas Szepe for the second part.

How do I get Window Maker to use more than 16 colors on my SGI Indy Workstation?
................................................................................

Thanks to Peter H. Choufor this answer:

By default, the SGI X Server uses 8-bit Pseudocolor mode. To change it, edit
(as root) the file /usr/lib/X11/xdm/Xservers. Change it to read:

.. code::
   :class: highlight

   :0 secure /usr/bin/X11/X -bs -c -class TrueColor -depth 24

Using WindowMaker with Solaris 2.6 CDE
......................................

Thanks to Rob Funk for this answer:

Assuming you installed Window Maker according to the README's that come with
the source, all you need to run Window Maker on a Solaris box is an entry in
the .xinitrc. This should work for any version. When you run wmaker.inst the
first time, allow it to make changes to the .xinitrc file. Mine looks like
this:

.. code:: sh
   :class: highlight

   #!/bin/sh
   # Window Maker Default .xinitrc
   exec /usr/local/bin/wmaker

Believe it or not, that's all that it takes. This, in fact, runs Window Maker
instead of OpenWindows. In order to choose Window Maker, you simply choose
"OpenWindows Desktop" in the "Options - Session" Menus And Choose "CDE Desktop"
if you want CDE.

The color schemes and settings for Window Maker are seperate from CDE. I tested
on a SPARC 10, but I assume Solaris x86 would work also.

(webmaster note: It works fine on Solaris x86)

How do I install Window Maker on a Solaris box?
...............................................

Here are some hints from John Kemp:

Installing Window Maker on a Solaris 2.6 box might require one or two little
hints. Here you are (this was on a system running xdm by the way, but similar
suggestions apply otherwise):

#. /usr/openwin/lib/X11/xdm/Xservers like this:

   .. code::
      :class: highlight

      :0 local /usr/openwin/bin/X -dev /dev/fb defdepth 24 defclass TrueColor

#. Turn off shm in the WindowMaker configure:

   .. code:: shell-session
      :class: highlight

      $ ./configure --disable-shm

#. might have to modify your LD_LIBRARY_PATH:, or make "wmaker" a script that
   does it for you (mv wmaker wmaker.exe):

   .. code:: sh
      :class: highlight

      LD_LIBRARY_PATH=/usr/local/lib:/usr/local/X11/lib:/usr/lib:/usr/openwin/lib
      export LD_LIBRARY_PATH
      /usr/local/bin/wmaker.exe $*

The real key is the "--disable-shm".

(webmaster note: Window Maker should work fine with SHM enabled, at least it
does under Solaris 8. Try the default first, and then use this if you run into
problems with it)

How do I fix an error such as libwraster.so.1: cannot open shared object file?
..............................................................................

If you have an error when running Window Maker such as

.. code:: shell-session
  :class: highlight

  libwraster.so.1: cannot open shared object file

These are the instructions for Linux.

First, make sure that /usr/local/lib ( or whatever directory you installed
Window Maker to) is listed in your /etc/ld.so.conf ). You need to run ldconfig
so the new shared libraries will be loaded. After running ldconfig as root, the
linker should properly load the libraries. You need to run this every time you
update Window Maker.

Thanks to Joseph Czapiga, the BSD procedure for adding shared library
directories is:

.. code:: shell-session
  :class: highlight

  ldconfig -m /usr/local/lib  (m means merge)

How do I fix an error dealing with aclocal: configure.in: 15: macro 'AM_PROG_LIBTOOL' not found in library?
...........................................................................................................

You need to install libtool. It also must be a libtool different from version
1.2b ( shipped with redhat 5.2 ). You can get libtool from ftp.gnu.org/pub/gnu
Make sure the autoconf and automake versions you have installed are at least:

- autoconf 2.12
- automake 1.3
- libtool 1.2

From Blaine Horrocks:

*You can also work around this problem on RedHat5.2 by copying the distributed
aclocal.m4 to acinclude.m4 before running configure for the first time.
Configure works fine and doing the make succeeds.*

When I run wmaker, it quits complaining about '__register_frame_info'
.....................................................................

This is related to having compiled Window Maker on a system whose libraries
were compiled by egcs or gcc 2.8.0, and then using the binaries on a system
whose libraries were compiled by gcc 2.7.2.x

Try compiling Window Maker with the newer gcc or recompile your system
libraries with the older gcc. It's generally a bad idea to mix and match.

How do I make libjpeg link against Window Maker?
................................................

The newest jpeg libs are availible at http://www.ijg.org

How many of you have seen that darned "lib reports 62 caller expects 61" type
of error? Here are some answers that will possibly help you out.

First things first. As always, make sure there are not older copies of libjpeg
floating around on your system. ]Some distributions by default come with an old
libjpeg.so.1 in the /usr/X11R6/lib/ directory. This can simply be deleted. Or
if something complains after you delete it, recompile it if you can to look for
the new lib in the right place, or if that fails, as a last resort, you might
add a symlink to the new lib like so:

.. code:: shell-session
   :class: highlight

   ln -s /usr/local/lib/libjpeg.so.6.0.2 libjpeg.so.1

Note that you should use your system's version of ldconfig to properly manage
your library cache (or other appropriate mechanism).

On Linux, this would mean having /usr/local/lib in /etc/ld.so.conf and then
running ldconfig.

Now on to the error. This is basically caused by your application having been
compiled to dynamically use the libjpeg.so shared library. When you install a
new lib and then try to run your program again, it expects the lib it was
compiled against, in this case the older libjpeg.so.6.0.1 and instead finds
libjpeg.so.6.0.2 and reports the error.

The fix is actually rather simple. Along with adding a libjpeg.so.6 symlink
like so (just in case):

.. code:: shell-session
   :class: highlight

   ln -s libjpeg.so.6.0.2 libjpeg.so.6

where you installed your new lib, you simply need to recompile your app too
link it against the new library.

Also, make sure to use GNU make for the Window Maker compile.

How do I start Window Maker after running wmaker.inst?
......................................................

As of version 0.53.0, the wmaker.inst script will modify your X startup script
(.xinitrc or .Xclients or .Xsession) to do something thats (hopefully)
appropriate.

In order to run wmaker, a user needs to have an ~/.xinitrc file consisting of
something similar to

.. code:: sh
   :class: highlight

   #!/bin/sh
   exec wmaker

This will vary from system to system, but the existance of an .xinitrc file
will generally override the system defaults.

How do I make libpng link against Window Maker?
...............................................

The newest png libs are availible at http://www.libpng.org/pub/png/libpng.html

You should also get the newest zlib libs from http://www.gzip.org

Generally, the same rules apply here as with libjpeg. Make sure there are no
older versions of the necessary libs floating around on your system, then try
to configure and make again.

Also, make sure to use GNU make (gmake) for the Window Maker compile.

How do I make giflib or libungif to link against Window Maker?
..............................................................

The newest versions of both these libraries are available at
http://prtr-13.ucsc.edu/~badger/software/


Users have had a few problems with giflib... it seems that the install process
didn't install the header file libgif.h, so although the Window Maker configure
found the lib (libgif.so.x), when you actually try to compile, it fails when it
looks for the header to include the make. One solution is to simply copy it
from the libgif source directory to the local system include directory.
(/usr/local/include/ on many systems).

Also, make sure to use GNU make (gmake) for the Window Maker compile.

How do I fix an error similar to "wrlib: could not allocate shared memory segment: invalid argument"
....................................................................................................

This relates to a shared memory problem on Solaris. Usually one can't see it -
but it is visible if X is started from command line (or fail-safe session for
that matter). In any of the cases, on the stderr you get an error message like
this:

.. code:: console
   :class: highlight

   "wrlib: could not allocate shared memory segment: invalid argument"

This one is generated by wrlib if Window Maker is compiled with shared-memory
usage enabled (which is the default). The explanation is that Solaris by
default comes with a shared memory segment size of maximum 1 M. What happends
is that if you have a really-really cool(tm) background, it is usually much
bigger than that 1 M segment of shared memory. To see your defaults relating
the IPC settings check the output of the "sysdef" command (look for IPC Shared
Memory). There you should see the maximum allocable size for a shared memory
segment. If it is less than 5 M you should really increase it by adding the
following line in your /etc/system file:

.. code::
   :class: highlight

   set shmsys:shminfo_shmmax=20971520

- Make sure you don't already have this value set. If you do, simply increase
  the value. In case you have a much bigger value, stick to what you have,
  because you should have no problems with it.
- The value allows a maximum segment size of 20 M, which really should be
  enough for anyone. If not, try using a smaller background image!
- Make sure you spell the line *exactly* as shown, otherwise at boot time the
  kernel will complain of not finding such a module name and will not set a
  thing about it!
- Make sure you don't delete other lines or modify them "beyond recognition",
  for evil things may happen at boot time.

After adding this to your /etc/system you need to reboot in order for the new
limit to take effect. Also, you may want to check the new limit just to make
sure it has been set.

Thanks to Bogdan Iamandei for this answer.

How do I add Window Maker to the Solaris dtlogin screen?
........................................................

The two files that determine alternate window managers are:

.. code::
   :class: highlight

   /usr/dt/config/C/Xresources.d/Xresources.*
   /usr/dt/config/Xsession.*

If you look in there, you'll find Xresources.ow and Xsession.ow, respectively.
All you need are two files that set up Window Maker (or any other window
manager) in a similar fashion, calling them Xresources.wm and Xsession.wm (or
whichever extension you prefer).

Here is an example setup:

.. code:: resource
   :class: highlight

   # **************************************************************************
   #
   # Window Maker config file
   # Mike Bland <mbland@cmu.edu>
   #
   # /usr/dt/config/C/Xresources.d/Xresources.wm
   #
   # used by dtlogin
   #
   # **************************************************************************

   Dtlogin*altDtsIncrement:        True

   Dtlogin*altDtName:      Window Maker
   Dtlogin*altDtKey:       /usr/local/bin/wmaker
   Dtlogin*altDtStart:     /usr/dt/config/Xsession.wm
   #Dtlogin*altDtLogo:     /usr/local/share/logos/WM_logo.xpm

Once I get a logo ready, I'll add it to the dtlogin screen by uncommenting the
last line.

And this example script:

.. code:: ksh
   :class: highlight

   #!/bin/ksh
   # **************************************************************************
   #
   # Window Maker startup script
   # Mike Bland <mbland@cmu.edu>
   # /usr/dt/config/Xsession.wm
   #
   # used by dtlogin
   #
   # **************************************************************************

   . /usr/local/etc/.profile       # Sources the file containing necessary
                                   # environment variables (especially
                                   # LD_LIBRARY_PATH=/usr/local/lib:...);
                                   # make sure it's executable.

   WINDOW_MANAGER=/usr/local/bin/wmaker

   export WINDOW_MANAGER

   /usr/local/bin/wmaker

What happened to libPropList?
.............................

The libPropList dependency has been removed as of Window Maker version 0.70.0,
and is replaced by cleaner, more robust code in the WINGs toolkit. This new
code maintains existing proplist compatibility, so there are no visable changes
for users, and existing file formats will work as they did before.

For developers, there is a proplist-compat.h header that provides a mapping
between the old and new function names. See the comments in this file for
further instructions.

----

Configuring Window Maker
------------------------

What are those files inside my ~/GNUstep directory?
...................................................

Here is a synopsis of the files in ~/GNUstep

* ~/GNUstep/WindowMaker/WindowMaker is main config file. This file controls
  options such as keybindings, fonts, pixmaps, and focus modes.
* ~/GNUstep/WindowMaker/WMWindowAttributes controls the "attributes" for
  individual applications and appicons. Options such as what icon to use are
  set here. For the most part, this is now best accessed via a right click on a
  title bar of an application and selecting "Attributes"
* ~/GNUstep/Defaults/WMState is the file that is automatically generated and
  contains the current dock settings. It is not recommended to edit this file
  by hand.
* ~/GNUstep/Defaults/WMRootMenu specifies what file to use as the root menu. In
  Window Maker 0.19.0 and higher, this file should be replaced by plmenu from
  ~/GNUstep/Defaults/WindowMaker so that one can use WPrefs.app to edit the
  menu.
* ~/GNUstep/Library/WindowMaker/menu is used to change your root menu, if you
  are using the old menu style.

How do I enable the normal X sloppy focus mode?
...............................................

If you are using WPrefs, you can choose the ``Window Focus Prefrences`` tab and
then select the ``Input Focus Mode`` Slider.

Scroll Down and choose ``Sloppy`` Focus Mode.

You may also use a text editor on ``~/GNUstep/Defaults/WindowMaker`` and change
the following:

.. code::
   :class: highlight

   FocusMode = sloppy;

How do I get my auto-arrange icons to work?
...........................................

In WPrefs, choose the ``Icon Prefrences Tab`` and select the ``Auto Arrange
Icons`` Checkbox. Or in ``~/GNUstep/Defaults/WindowMaker`` set

.. code::
   :class: highlight

   AutoArrangeIcons=YES;

and the icons should now auto-arrange.

How do I get my Meta-Tab to cycle through windows correctly?
............................................................

To use WPrefs to modify these, choose the ``Ergonomic Prefrences`` tab and
check ``Raise window when switching focus with keyboard (Circulate Raise)``

Or you can use a text editor to make sure that these settings are in your
``~/GNUstep/Defaults/WindowMaker`` file:

.. code::
   :class: highlight

   CirculateRaise = YES;
   RaiseDelay = 1;

As of 0.61.0, MS Window's Style application tabbing is supported by default.

How do I get a tile background for my appicons (those things in the dock)?
..........................................................................

These can all be adjusted by the ``Appearance Preferences`` tab in WPrefs.

Select the tile and then choose the edit texture dialog. Then you may choose
any of the different tile background options in the The old text editor method
is provided below for convience.

You need to change one line in your '~/GNUstep/Defaults/WindowMaker' file.

.. code::
   :class: highlight

   IconBack = (spixmap, tile.black.xpm, white);

The last parameter is the color that fills in any transparent parts of your
icon.

How do you dock <insert program here> that doesn't have an appicon in the new version of WindowMaker?
.....................................................................................................

There is now an option available to emulate appicons so that Window Maker can
dock just about anything now. To dock a misbehaving application, right click on
the title bar and select the attributes menu. Next, select the pull down menu's
"Advanced Options" item. Under the ``Advanced Options`` menu, select the
``Emulate Application Icon`` Option then Save, Apply and close the dialog.

This should allow you do dock the program normally.

Dan Pascu adds:

Emulate Appicon does exactly the same as dockit. So if Emulate Appicon does not
work, dockit will not work either. For such apps you can do nothing. They are
badly coded (they do not set the instance.class hints). For these Attributes
are also not available, since attributes apply to an instance and/or class
hint.

Note: Dockit was previously distributed with Window Maker and was launched from
the top dock icon.

Elliott Potter adds:

There's another way to dock applications that misbehave ... I've only done this
with a couple of things (Adobe AcroRead is the only one I remember at the
moment).

If Attributes -> Advanced Options -> Emulate Application Icon doesn't work:

- Dock another application to the clip, where you want your application to go.
  I used gv, but anything you can dock will work.
- Quit WindowMaker
- Edit ~/GNUstep/Defaults/WMState.

  If you're docking to the clip, scroll down to the Workspaces section.
  When you find whatever you docked, you'll see:

  .. code::
     :class: highlight

     {
         Command = gv;
         Name = GV.gv;
         AutoLaunch = No;
         Forced = No;
         BuggyApplication = No;
         Position = "6,0"
         Omnipresent = No;
         DropCommand = "gv %d";
     },

  Edit it to use the info for your new application:

  .. code::
     :class: highlight

     {
          Command = acroread;         # use the full pathname if you have to
          Name = acroread.acroread;
          AutoLaunch = No;
          Forced = No;
          BuggyApplication = No;
          Position = "6,0"
          Omnipresent = No;
          DropCommand = "acroread %s";
     },

  Then edit WMWindowAttributes, and add a line for your application's
  icon...you can edit the line that was inserted, or make a new one - I
  just make a new one:

  .. code::
     :class: highlight

     acroread.acroread = {Icon = pdf.tiff;};

  Then re-start WindowMaker, and your icon should be there! You can move it
  around like any other docked app now, but the Attributes section still won't
  work.

How do I get x11amp to not have a title bar ( or any other program for that matter )?
.....................................................................................

Right Click on the title bar and go to the attributes menu. Click on Window
Attributes and click the the Disable titlebar and Disable Resizebar options.
Click Save, and then click Apply then close the Attributes panel.

By Default, to get back to the attributes menu, use the key combination
Control-Esc.

How do I set a pixmap background?
.................................

Here is the in depth explanation straight from the NEWS file:

wmsetbg now accepts the following options:

.. TODO wow! how old this thing is!

.. code::
   :class: highlight

        usage: wmsetbg [-options] image
        options:
        -d
                dither image
        -m
                match colors
        -t
                tile image
        -s
                scale image (default)
        -u
                update Window Maker domain database
        -D <domain>
                update <domain> database
        -c <cpc>
                colors per channel to use

By default, it will try to guess if dithering is needed or not and proceed
accordingly. Using -d or -m will force it to dither or match colors.

Dithering for more than 15bpp is generally not needed, and will only result in
a slower processing. Don't use dithering except when needed, because it is
slower. Else rely on wmsetbg which will detect if dithering is needed and use
it.

- ``-u`` - will update the WorkspaceBack in the default database domain file in
  ~/GNUstep/Defaults/WindowMaker, and let Window Maker refresh the screen.
  Please note that this option only works under Window Maker, and will have no
  effect under other window managers, since it rely on Window Maker to update
  the image after it reads the updated defaults database.
- ``-D`` - <domain> is same as above, but will update the domain <domain>
  instead of the default Window Maker domain.
- ``-c`` <cpc> will set the color per channel to use. Only needed for
  PseudoColor visuals. Window Maker will automatically pass the value read from
  the Window Maker domain database.

The following line is straight from your WindowMaker-0.15.x
~/GNUstep/Library/WindowMaker/menu file and should all be on one line.

"Images" OPEN_MENU BACKGROUNDS_DIR ~/GNUstep/Library/WindowMaker/Backgrounds
WITH wmsetbg -u -t

This should give you an idea on how to add other entries for different image
directories. See the help info at the top of the
~/GNUstep/Library/WindowMaker/menu file for more information.

If you for some reason would like to set your background image with XV, for
instance to use an image format not yet supported by wmsetbg or to use one of
XV's special modes, edit the file ~/GNUstep/Library/WindowMaker/autostart and
insert the line


.. code:: sh
   :class: highlight

   xv -root -quit -maxpect ~/background.jpg

or

.. code:: sh
   :class: highlight

   xv -root -quit -max ~/background.jpg

you can also try variations of this to get different tiling and other effects
(where X is a number 1-9 I believe):

.. code:: sh
   :class: highlight

   xv -root -quit -rmodeX ~/background.jpg

If you would like xv functionality in your menu, heres a nice little tip from
Alfredo:

Add the following line to your ~/GNUstep/Library/WindowMaker/menu file. (all on
one line)

.. code:: sh
   :class: highlight

   "More Backgrounds" OPEN_MENU /home/whoever/backgrounds xv -root -maxpect -quit

Can I put pixmaps in my root menu and title bars?
.................................................

Put the pixmaps in a directory that is located in your pixmap path set on
``Search Path Configuration`` Tab.

Then switch ``Appearance Preferences`` tab and select what widget you would to
adjust under the ``Texture`` tab. Click edit. Chose an image texture format and
then search for the texture.

You can use a similar procedure for any type of menu editing.

You can use png, gif, ppm, tiff, jpeg and xpm images interchangeably in Window
Maker if you have compiled in support for those formats.

How do I get my Minimize Icon to look like the triangle I see in screenshots?
.............................................................................

This involves a minor source tweak. Instructions are available at
http://largo.windowmaker.org/tips.php#titlebar_icons

Why does Netscape have a black and white Icon when I minimize it?
.................................................................

Craig Maloney  has this answer:

If you happen to ``--enable-openlook`` at compile time, Netscape (and
presumably other apps as well) believe they're running under OLVWM, and
minimise with monochrome icons. Once compiled without OpenLook support,
Netscape minimizes with the correct icon.

How do I get superfluous bells and whistles working?
....................................................

Open WPrefs and go under  the ``Other Configurations`` tab. Under ``Animations
and Sound``, depress the Superfluous tab.

  Alternatively, you may add

.. code::
   :class: highlight

   Superfluous=YES;

to your ~/GNUstep/Defaults/Windowmaker file.

How do I get the classic NeXT(tm)-like style back?
..................................................

Open WPrefs and go under the ``Other Configurations`` tab. Under ``Title Bar
Style``, select the classic look.

Or you can add

.. code::
   :class: highlight

   NewStyle=NO;

to your ~/GNUstep/Defaults/Windowmaker file.

How do I get the window menu with only a two button mouse?
..........................................................

In WPrefs, under ``Mouse Prefrences``, the mouse actions can be mapped to a
button of choice.

Jim Noble  explains another way to do this:

If you've got a two-button mouse under some versions of Solaris x86, there's no
way (that I'm aware of) to emulate a 3-button mouse. The right button can be
either MB2 or MB3, but chording doesn't work.

.. code::
   :class: highlight

   ApplicationMenuMouseButton = Left;

and

.. code::
   :class: highlight

   WindowListMouseButton = Right;

in ~/GNUstep/Defaults/WindowMaker ought to allow the left button to activate
the root menu, and the right button (as MB2) to activate the windows menu.

How do I edit my root menu?
...........................

You can now use WPrefs.app ( its appicon looks like a heart rate meter with a
GNUStep icon backgroud ). Note that this will replace any oldstyle menus and
there is no way to convert the oldstyle menu to the new libproplist style menu.

For old style menus, edit the file ``~/GNUstep/Library/WindowMaker/menu`` and
save your changes. Window Maker should detect the change and automatically
update. If you are having a problem getting it to reload the menu, try

.. code:: shell-session
   :class: highlight

   $ touch menu

to force the modification time into the future.

WPrefs disappeared from the Dock! How do I get it back?
.......................................................

Pascal Hofstee  offers this answer:

You should just start it from a terminal by supplying it's FULL path-name,
which is usually the following: ``/usr/local/GNUstep/Apps/WPrefs.app/WPrefs``.

At this point, a new appicon should be generated which can be placed back into
the Dock.

How can I define my own Icon for a program? (instead of the Icon the Application Supplies?)
...........................................................................................

You can right click on the titlebar of the running app and choose the
"Attributes..." option, then click on the "Ignore client supplied icon"
checkbox. Click "Apply", "Save" and close the Attributes Editor.

Another method is to edit ``~/GNUstep/Defaults/WMWindowAttributes`` by hand and
use the ``AlwaysUserIcon=YES;`` option for the app. For example:

.. code::
   :class: highlight

   xmcd = {
         Icon = "Radio.xpm";
         AlwaysUserIcon=Yes;
   };

How do I turn off the workspace titles between workspaces?
..........................................................

In Window Maker 0.60.0, an option was added to turn this off.

By editing ``~/GNUstep/Defaults/WindowMaker`` insert or modify the key
``WorkspaceNameDisplayPosition = none;`` Other valid options for this include
``center``/``top``/``bottom``/``topleft``/``topright``/``bottomleft``/``bottomright``;

How do I add dynamic items to my root menu?
...........................................

A few programs are floating about, notably wkdemenu.pl that can produce output
from other menu styles. In order to get WindowMaker to launch the process
everytime you want to use the menu, use something like

.. code::
   :class: highlight

   ("External Menu", OPEN_MENU, "| bob.sh")

in a proplist style menu. You can tell if you have a proplist style menu if you
can edit it with WPrefs.

You can do this directly in WPrefs by going to the menu editor, adding an
"external menu", and then clicking the "ask guru button" and filling in the
process name.

Thanks to Igor P. Roboul

How do I remove or hide appicons?
.................................

There are two options here, and you need to consider which one you prefer. Read
both of these before you decide.

First, if you do not want to use the clip or dock at all, you can launch wmaker
with with

.. code:: shell-session
   :class: highlight

   $ wmaker --no-clip --no-dock

and then in ``~/GNUstep/Defaults/WMWindowAttributes`` add

.. code::
   :class: highlight

   "*" = {NoAppIcon=Yes;};

The problem with this method is if you use the dock for dockapps, it renders
them with out an appicon to write to. An alternative method if you are willing
to let the clip be on your desktop is to right click on the clip > clip options
> auto attract. Double click the clip so that it is grayed and all appicons
will be hidden. Then you can hide the clip behind the dock so that it is out of
your way. This will allow appicons to work.

I disabled my titlebar. How can I get it back?
..............................................

Thanks to  Jim Knoble for this answer

Set the focus to the window and then use the keystroke assigned to the titlebar
menu. If you're not sure what the keystroke is, you can find out using WPrefs:
in the keyboard section, select the *Open window commands menu* item in the
list of actions. The keystroke assigned to it ought to appear in the
*Shortcut' area*.

Typically it is Control-Esc or F10 in older version of WindowMaker.

How do I remove ALT+Mouse1 from the action Window Maker grabs for an application?
.................................................................................

Do [Button3Down] (for righthanded mouse users, [RightButtonDown]) on the
titlebar of the desired window.  Choose ``Attributes...``. In the Attributes
inspector, choose ``Advanced Options``.  Check ``Don't Bind Mouse Clicks``.
Apply or Save as desired, then close the Attributes inspector.

The result is that [Alt+Button1] (which usually grabs a window to move it
around), [Alt+Button2] (which usually grabs a window to move it around without
changing the window stacking order), and [Alt+Button3] (which usually resizes a
window) all get passed to the application instead of performing their usual
action.

How do I configure the Dock and Clip to use less space on a small screen?
.........................................................................

This answer is current as of WindowMaker-0.61.1.

For the Clip, either:

- Disable the Clip from WPrefs (panel number 7), or
- Hide the Clip under the Dock (for example, in the upper righth and corner of
  the screen).

The latter is probably more useful on desktops with limited space, since you
can still set the Clip to attract app-icons so they don't clutter your desktop.

For the Dock, try the following:

#. Exit Window Maker.
#. Log in via a text console or using a different window manager.
#. Edit ~/GNUstep/Defaults/WMState using your favorite text editor
   (for example, vi, emacs, or pico).

#. Find the *Applications* part of the *Dock* structure. Find the item with
   *Position = "0,0";*. Change the *Command* item to the command you want the
   top tile to launch. Change the *Name* item to the *<instance>.<class>* name
   of the application you just made the Command item start (for example, if
   *Command* is *"xedit"*, then *Name* should be *xedit.Xedit*).
#. Save the WMState file.
#. Start an X session with Window Maker.
#. Check that the top tile starts the command you told it to. (You should still
   also be able to move the Dock up and down using [LeftDrag] on the top tile.)
#. You can configure the tile (including autolaunch and the drop-command) in
   the regular manner ([RightButtonDown] on the tile and choose *Settings...*
   from the resulting menu).

Why do dashes not work as menu entries?
.......................................

If you wish to use a ``-`` as part of a menu item name, you must enclose the
name in double quotes. This will only apply if you're editing the
~/GNUstep/Defaults/WMRootMenu file manually, as it is handled properly within
WPrefs.

This will work:

.. code::
   :class: highlight

   (ssh,
   ("us-gw", EXEC, "Eterm -e ssh us-gw"),

This will not:

.. code::
   :class: highlight

   (ssh,
   (us-gw, EXEC, "Eterm -e ssh us-gw"),

Thanks to Martin Sillence for pointing this out.

----

Using Window Maker
------------------

How do add new icons to the Dock?
.................................

First, launch an application. If an icon (henceforth called an ``appicon``)
appears in the bottom left corner of the screen, left click and drag it over
near the Dock. You will see a slightly opaque square of where the Dock will
place the appicon. When you do, release the mouse button and the appicon should
now be in the Dock.

Next, right click on the desktop to bring up the menu. Select Workspace -> Save
Session to make this permanent.

What is the difference between the Exit and Exit Session Option?
................................................................

Another answer from Dan Pascu:

Exit will exit wmaker, but can leave other X apps running, provided that it was
not the last app launched in the .xinitrc (for instance, if you had exec
wmaker, followed by exec xterm, exiting wmaker using 'Exit' will leave the
xterm running so you could start another window manager, etc.)  This is
accomplished because X will not shutdown unless all X apps are closed.

Exit session will exit wmaker, but will also close all running apps, thus the X
server will be closed too.


How do I "dock" icons on the clip?
..................................

Just drag icons near it like you would for the dock. If you are having a
problem docking icons, you should try moving the clip away from the dock.

Why do none of my key bindings (ie: Alt+#) work in Window Maker?
................................................................

If you are using XFree86, make sure scroll lock and numlock are off or no
bindings will work (XFree bug). You can try using the XFree86 Numlock Hack by
editing the line ``#undef NUMLOCK_HACK`` in $(WindowMaker)/src/wconfig.h and
changing it to ``#define NUMLOCK_HACK``.

With the release of 0.18.0, this hack is now working and hopefully no one will
have to ask this question again.

How do I rename workspaces?
...........................

Right click to bring up the root menu. Go under the Workspaces menu item and
hold the control key down. Next, click on the workspace entry you would like to
rename, type the name, and press enter.

How can I resize a window if the window is larger than my current desktop?
..........................................................................

David Reviejo best summed up this answer:

Maybe you know: Alt+Left click and drag to move the window.

Try this: Alt+Right click and drag to resize (by moving the nearest window
corner)

Another move/resize tip: while you are moving or resizing a window, you can
change the move/resize mode by pressing the SHIFT key.

How do I "undock" appicons?
...........................

If the program is not running, just drag the icon to the middle of your desktop
and watch it disappear.  If the program is running, hold down Meta and drag the
icon off the dock.

I docked an application but when I run it the button is permanently shaded and
I can't run new instances. You probably docked the application with dockit. To
fix it remove the icon and use the "Emulate Application Icon" checkbox in the
Advanced Options section of the Attributes panel for the window. Then restart
the application to get the application icon you must use to dock the
application. It can also mean that you did something you shouldn't, which is
changing the program that is ran from the docked icon. For example, if you
docked rxvt you must NOT change it to xterm, for example. You also can't do any
changes that might alter the contents of the WM_CLASS hint for the window, like
the -name parameter for xterm, rxvt and other programs.

When I run wmaker it complains about not being able to load any fonts.
......................................................................

Check if the locale settings are correct. If you're not sure what to do, unset
the LANG environment variable before running wmaker.

.. TODO give complete explanation

When I set the root background with wmsetbg by hand it works, but when I do
that from the configuration files it doesnt! If you set the root background
with wmsetbg by hand, it will obviously find the image, since you have
explicitly specified it by hand. But if you simply put it in
``~/GNUstep/Defaults/WindowMaker`` in some option like WorkspaceBack, it will
not find the image because Window Maker can't read your mind to figure where
you put the image. So, to fix it, you have to either place the full path for
the image in the texture specification or put the path for the directory you
put your background images in the PixmapPath option. You can also put all your
background images in places like ``~/GNUstep/Library/WindowMaker/Backgrounds``
or ``/usr/local/share/WindowMaker/Backgrounds``.

David Green says that another possibility is that you have two copies of the
worker programs: wmsetbg (and possibly setstyle) and the wrong one is in the
path first.

What is the purpose of being able to draw a box on the root menu with a left click?
...................................................................................

Its purpose is two-fold.

First, it is used to select multiple windows on a desktop at a time. When these
windows are selected, they can be moved around on your desktop and will retain
their relative positions.

Second, once selected, they are persistent through desktop changes. So it is
useful for moving large numbers of windows between desktops.

You can also select windows with shift+click.

----

Application Compatibility
-------------------------

How do I assign gimp an appicon?
................................

You can enter the following line in WMWindowAttributes:

.. code::
   :class: highlight

   gimp={Icon="gimp.tiff";};

Window Maker now can assign Icons from within the windowmanager. To do so,
right click on the title bar of an app, click on the droplist->Icon and
WorkSpace entry, enter the icon file name (make sure this is in your pixmap
path), click update, apply, and then save.

How do I get an appicon for XEmacs 20.3+?
.........................................

Thanks to Michael Hafner for this answer.

You don't need to patch the XEmacs code, just run

.. code:: shell-session
   :class: highlight

   ./configure --with-session=yes (in addition to any other options you use)

in your XEmacs 20.3+ sourcedir and rebuild it. Then XEmacs shows an appicon
when running and you can easily dock it.

Where do I get the nifty clock program I always see on people's desktops?
.........................................................................

It's called asclock. Once included with Window Maker, it now is available at
ftp://ftp.windowmaker.org/pub/contrib/srcs/apps/asclock.tgz.

asclock was written by Beat Christen and used to have its own website, which
seems to have disappeared.  However, references to it exist all over the place,
and can be found by searching `Google
<http://www.google.com/search?q=asclock%22>`_.

Beat Christen wrote this awhile back:

Please note that the asclock-gtk version 2.0 beta 4 (asclock-gtk-2.0b4.tar.gz)
does not have the -d switch yet and that the asclock-xlib-2.1b2.tar.gz does not
have the shaped asclock builtin.

A wonderful alternative to asclock is Jim Knoble's `wmclock
<https://www.dockapps.net/wmclock>`_. It duplicates asclock and adds some much
needed improvements.

How do you dock asclock?
........................

It is highly recommended that you use the asclock mentioned previously in
question 5.3. The asclock that is typically included in AfterStep will not
properly dock with Window Maker. At this point, there are at least four or five
different versions of asclock floating about.

For older versions such as asclock-classic , use a command line similar to

.. code:: shell-session
   :class: highlight

   asclock -shape -iconic -12 &

For newer versions such as asclock-xlib 2.0 and asclock-gtk use

.. code:: shell-session
   :class: highlight

   asclock -shape -iconic -12 -d &

Drag it from the top right corner of the clock to the dock. Right click on the
icon and select autolaunch.

In order to make asclock launch every time you start Window Maker, right click
on the outer edge of the border for asclock until a menu appears. Select the
"Settings" item and then select the "Lauch this Program Automatically" option
then select the "OK" button.

If you get an error such as sh: /dev/console: Permission denied, login as root,
cd to /dev/ and run

.. code:: shell-session
   :class: highlight

   ./MAKEDEV console

Where can I get more dockapps?
..............................

The Window Maker team got tired of people E-mailing constantly asking where the
websites for obscure dockapps disappeared to. So we've created the ultimate
dockapps community website. Visit `dockapps.net <http://www.dockapps.net>`_ for
the latest, up-to-date links, information, and download for Window Maker and
related dockapps.

Another large index of dockapp links is available at
http://www.bensinclair.com/dockapp. The downside to this is that they're only
links, so if someone stops maintaining a dockapp, or their web hosting provider
cuts them off, you won't be able to get to it. Still, Ben Sinclair's site was
the first big "dockapp warehouse" site, so we give credit where credit is due.
:)

How do I get an appicon for rxvt so I can dock it?
..................................................

.. TODO check out urls and legitimacy of the question

The default rxvt that comes with most distributions is an outdated version of
rxvt. The newest development version of rxvt is availible from
ftp://ftp.math.fu-berlin.de/pub/rxvt/devel/. As of the time of this writing,
the version is 2.4.7 and it natively produces an appicon without a patch.

John Eikenberry has also created an rpm which is available from
ftp://ftp.coe.uga.edu/users/jae/windowmaker

How do I allow Alt+# to work in an rxvt/xterm session?
......................................................

First, Launch a unique instance of rxvt or xterm. This can be done using the -N
option of rxvt.

.. code:: shell-session
   :class: highlight

   rxvt -name foo -e irc

Then, go to the Attributes menu (right click on titlebar -> Attributes) /
Advanced Options and enable "Don't Bind Keyboard shortcuts". Click Save and
Apply and you should be able to run your session without the shortcuts.

How do I get different icons for different rxvt's and xterms?
.............................................................

The hint is the -name option for xterm or rxvt. This will allow you to change
the exact WM_CLASS in the attributes menu and assign a unique icon.

.. code:: shell-session
   :class: highlight

   rxvt -name foo -title Testing

Then Right click on the title bar to bring up the attributes menu, and you will
be able to edit the properties for foo.XTerm (ie: assign a unique icon).

How do I launch multiple instances of XTerm from one appicon?
.............................................................

Thanks for the update by Sara C. Pickett:

The easiest way to accomplish this is to dock XTerm as normal. Then Go to the
Attributes menu -> Application Specific and select no application icon for
XTerm.

Then right-click on the docked appicon and select settings. Change the
Application Path with arguments section to

.. code:: shell-session
   :class: highlight

   '/bin/sh -c "exec xterm &"'

Window Maker breaks scilab.
...........................

If you refer to the problem of the "graphics" window of scilab not showing up
in Window Maker, this is caused by a bug in scilab. You can see the cause of
the problem by yourself, by running xprop on the graphic window:
WM_NORMAL_HINTS(WM_SIZE_HINTS):

.. code::
   :class: highlight

   user specified location: 136679205, 1074468360
   user specified size: 400 by 300
   program specified minimum size: 400 by 300

Now, when scilab opens it's window, Window Maker nicely does exactly what it is
told, that is, map the window at position 136679205, 1074468360 which obviously
falls outside the screen no matter how big is your monitor ;)

Meanwhile, the workaround for this is to open the window list menu (click on
the root window with the middle mouse button) and click on the ScilabGraphic
entry. The window should be brought to your reach. Then, open the window
commands menu (right click on window's titlebar) and open the Attributes panel.
Go to the "Advanced Options" section, check the "Keep inside screen" option and
save.

If you can recompile Scilab, this came from a Scilab developer:

replace

.. code:: C
   :class: highlight

   size_hints.flags = USPosition | USSize | PMinSize;

with

.. code:: C
   :class: highlight

   size_hints.flags = /** USPosition |**/ USSize | PMinSize;

in routines/xsci/jpc_SGraph.c

How do I get a transparent xterm/rxvt/xconsole?
...............................................

You need a terminal emulator that has support for transparency, like Eterm, the
latest rxvt, wterm, aterm or gnome-terminal.

You can find these programs on http://www.freshmeat.net.

How do I dock an arbitrary console application like mutt?
.........................................................

There are two key things to do if you want a program (such as mutt) to be able
to start in a terminal window from the Dock or the Clip:

- Make the terminal window start the program you want to run instead of a
  shell. Both xterm and rxvt (and its descendants) are capable of doing this.
  For example:

  .. code:: shell-session
     :class: highlight

     xterm -e mutt
     rxvt -e mutt
     gnome-terminal -e mutt

- Convince Window Maker that the resulting terminal window is not a regular
  terminal window, but rather some other program instance. Both xterm and rxvt
  are also capable of doing this.  Make sure that -e is the last command
  option. For example:

  .. code:: shell-session
     :class: highlight

     xterm -name muttTerm -e mutt
     rxvt -name muttTerm -e mutt
     gnome-terminal --name=muttTerm -e mutt

  This causes the instance of the terminal window that you start to have an
  <instance-name>.<class-name> pair of ``muttTerm.XTerm`` (usually rxvt's
  class is also XTerm; don't know about its descendants, such as wterm and
  Eterm).

  Do not use spaces or periods in the instance name. For example, these are
  BAD instance names:

  .. code:: shell-session
     :class: highlight

     xterm -name mutt.term -e mutt
     rxvt -name 'mutt term' -e mutt

  Window Maker will not like you if you use them.

  With a different instance name, you can now do the following:

  - Dock the resulting appicon in the dock, or clip it to the Clip.
  - Assign a different icon and different window properties to the `special'
    terminal window running your program (make sure you choose the exact
    ``muttTerm.XTerm`` window specification in the Attributes editor).
  - Specify different resource settings for muttTerm in your ~/.Xdefaults file
    (e.g., different default foreground and background colors).

There are a few other non-key things you can do to complete the process:

- Tell the terminal window to display a more meaningful or prettier title and
  icon title than what gets put there due to ``-e``. For example:

  .. code:: shell-session
     :class: highlight

     rxvt -title 'Mail (mutt)' -n 'Mail' -name muttTerm -e mutt

  Xterm works the same way.

- These are getting to be a lot of command-line options. Make a wrapper script
  to use so you don't have to remember them all:

  .. code:: shell-session
     :class: highlight

     mkdir ~/bin
     cat >~/bin/muttTerm <<EOF
     #!/bin/sh
     rxvt -title 'Mail (mutt)' -n 'Mail' -name muttTerm -e mutt
     EOF
     chmod +x ~/bin/muttTerm

  Now you can do the same thing as that really long command in [3] above using
  the simple:

  .. code:: shell-session
     :class: highlight

     ~/bin/muttTerm

  If you put ~/bin in your PATH, you can use the even simpler:

  .. code:: shell-session
     :class: highlight

     muttTerm

- If you want to be sly, you can change the docked muttTerm to use your new
  wrapper script instead of the really long command; then, when you want to
  change anything in the really long command except for the instance name, you
  can just change the wrapper script, and it's done. Here's the procedure:

  - [RightButtonDown] on the muttTerm dock tile
  - Choose ``Settings...``
  - Replace the text in the ``Application path and arguments`` field with the following:

  .. code::
     :class: highlight

     muttTerm

  - Choose ``OK``

    Note that Window Maker needs to know that ~/bin is on your PATH for this to
    work; you may need to exit your X session and start it again.

    To change the instance name of the terminal window (e.g., from ``muttTerm``
    to ``mailTerm`` or ``blah`` or ``terminalWindowRunningMutt``), you need to
    do the following

  - Change your muttTerm script
  - Undock your old muttTerm
  - Run your muttTerm script
  - Dock the resulting terminal window
  - Do the stuff in first 4 subpoint above again.

Good luck.

Thanks to Jim Knoble for this answer.

How do I get an appicon for Netscape?
.....................................

If you are not using one of the latest Navigators, you can

#. Right click on the title bar
#. Click ``Attributes``
#. Select ``Advanced Options`` from the pull down menu
#. Select ``Emulate Application Icon``
#. Click Save

and older netscapes should now produce an application icon.

If you are using a newer rpm from Redhat Linux, try running

.. code:: shell-session
   :class: highlight

   grep irix `which netscape`

This seems to have been introduced in their 4.7 update. Comment out
irix-session management restart netscape. Alternatively, you may run either

.. code:: shell-session
   :class: highlight

   /usr/lib/netscape/netscape-communicator

or

.. code:: shell-session
   :class: highlight

   /usr/lib/netscape/netscape-navigator

depending on which rpms you have installed.

How can I dock an application from a remote machine using ssh?
..............................................................

This answer asumes that you have already set up RSA authentication using
``ssh-keygen``. To be able to launch applications without being prompted for
the password, you can use ``ssh-agent`` and ``ssh-add`` as follows.

With the addition to ~/.xsession of

.. code:: shell-session
   :class: highlight

   eval `ssh-agent`
   ssh-add /dev/null

just before

.. code:: shell-session
   :class: highlight

   exec wmaker

Then ssh will no longer prompt for the RSA-key passphrase. The ``/dev/null``
argument to ``ssh-add`` causes it to use the ``ssh-askpass`` graphical dialog.

The following procedure shows how to dock a remote xterm using ``ssh``.  This
procedure should work well for any well-behaved X11 application, including most
Dock applets.

#. From a terminal window, start an ssh session with ``xterm`` as the command:

   .. code:: shell-session
      :class: highlight

      ssh -a -C -X remote.example.net "xterm -name blah"

   (The '-a' switch turns off agent forwarding, for security reasins and the
   '-X' switch turns on X11 forwarding, required for the remote xterm to run.
   The -C option turns on compression, very useful for things such as X)

#. When the remote xterm appears, find the appicon. If it's not already in the
   Clip, drag it there.

#. [RightButtonDown] on the appicon and choose 'Settings...' from the menu.
   Note that the 'Application path and arguments' field contains only:

   .. code:: shell-session
      :class: highlight


      xterm -name blah

   Change that to:

   .. code:: shell-session
      :class: highlight

      ssh -a -C -X remote.example.net "xterm -name blah"

   The backslashes and double quotes are critical. Change the contents of
   'Command for files dropped with DND' in the same fashion, putting '%d'
   inside the double quotes.

   If you wish, change the icon so that you can recognize the tile easily.
   Press 'OK'.

#. [RightButtonDown] on the appicon again and choose 'Keep Icon(s)'.

#. Exit the remote xterm. The new Clip tile should remain, with the three dots
   at the lower lefthand corner to indicate the app is no longer running.

#. [DoubleClick] on the new Clip tile.  You should get the remote xterm again
   after a short while, depending on the speed of your network and of the
   remote machine.

#. You may either leave the remote application in the Clip, or drag it to the
   Dock.

.. note::
   You should be wary of docking something like ``wminet`` or ``wmnet`` in the
   manner, since you may create a feedback loop by causing additional network
   traffic, which the program monitors, causing yet more network traffic...

How do you make an omnipresent window not take focus whenever switching workspaces?
...................................................................................

Typically, on applications like xmms, they are set to omnipresent so they will
appear on every workspace. This causes the app to often get the focus
unintentionally when switching workspaces.

To remedy this,

#. Bring up the ``Attributes`` menu. You can do this by [Right Clicking] on the
   title bar and seletcing ``Attributes``. Alternatively, you may hit
   'Control+ESC' at the same time to bring up the title bar menu on apps that
   do not have a title bar.

#. In the ``Window Attributes`` menu, select ``Skip Window List``

#. Push ``Save`` and then hit the close dialog window icon in the upper right
   corner of the window frame.

Now the window will not take focus when switching workspaces.

.. note::
   this will also make the window non-focusable via keyboard window switching.
   The only way to shift focus to the window is via the mouse.

----

Themes and Dockapps
-------------------

What exactly are themes?
........................

Themes are a great aspect of Window Maker allowing a user to simply save the
entire 'look' of their desktop in an archive to distribute freely among
friends, fellow users and/or the whole net in general. :)

See the `theme-HOWTO <{{ site.baseurl }}/themes/themepacks.html>`_ for an
in-depth walk-through on making a Theme archive.

How do I install a theme?
.........................

This should be as simple as untarring the Theme.tar.gz into one of two places.
You can untar it to the global /usr/local/share/WindowMaker/* directory, and
have it be accessable to all users, or you can untar it to your own
~/GNUstep/Library/WindowMaker/ directory for your own personal use.

Use your favorite variation of the following:

.. code:: shell-session
   :class: highlight

   gzip -dc "Theme.tar.gz" | tar xvf -

Note that directory may differ on different systems

Why do my themes not load the background?
.........................................

Likely you have not compiled Window Maker with support for the background image
format, usually JPEG.

You can check this by the following command

.. code:: shell-session
   :class: highlight

   ldd `which wmaker`

.. TODO: check url

If libjpeg is not listed, you will need to install libjpeg that is available
from ftp.windowmaker.org

How do I make a Theme?
......................

Please see the `theme-HOWTO <{{ site.baseurl }}/themes/themepacks.html>`_ for
details on making both new and old style themes (and the differences between
the two), here is a short summary on making old style themes. Also, read the
README.themes file included with the Window Maker distribution in the
WindowMaker/ directory.

In this walk-through when I use WindowMaker/, it can refer to the global
/usr/local/share/WindowMaker/ directory or the users own
~/GNUstep/Library/WindowMaker/ directory.

To make a Theme.tar.gz, these are the steps I take:

#. Optionally create a README for your theme in WindowMaker/, call it
   something like "ThemeName.txt"

#. Use the following command to add the Theme files to your .tar file.

   .. code:: shell-session
      :class: highlight

      tar cvf ThemeName.tar ThemeName.txt Themes/ThemeName
      Backgrounds/ThemeNameBG.jpg Backgrounds/ThemeNameTile.xpm

   You can add as many more images as you need from the appropriate directories
   nder WindowMaker/ following that general idea. You can even optionally add
   an IconSets/ThemeName.iconset and it's associated icons to your theme in the
   same manner. This should be stated in your README if you decide to include
   these.

#. Then gzip your .tar file to make your ThemeName.tar.gz file with this
   command:

   .. code:: shell-session
      :class: highlight

      tar cvf ThemeName.tar ThemeName.txt Themes/ThemeName
      Backgrounds/ThemeNameBG.jpg Backgrounds/ThemeNameTile.xpm

   You can add as many more images as you need from the appropriate directories

   .. code:: shell-session
      :class: highlight

      gzip -9 ThemeName.tar

#. Now give it to your friends!

I untarred a theme in ~/GNUstep/Library/WindowMaker like the README says,but it doesnt show up in the menu!
...........................................................................................................

Make sure the OPEN_MENU command for the Themes entry in your menu has the path for your personal themes directory included in it. To be sure, add

.. code::
   :class: highlight

   #define USER_THEMES_DIR ~/GNUstep/Library/WindowMaker/Themes

in your wmmacros file.

----

Miscellaneous Questions
-----------------------

Is there a pager for Window Maker?
..................................

Not at the moment because there is not a pressing need for a pager. The concept
of multiple desktops does exist and there are currently 3 ways to switch
between them.

First, the Meta+Number combination will switch between desktops. The Workspaces
menu will also let you switch workspaces. Lastly, the clip will also scroll one
through workspaces.  For those that would like to send an application to a
specific workspace, either drag it to the edge of the desktop onto the next
workspace, or right click on its title bar, select 'Move To', and click the
workspace you want it to be moved to.

However, Window Maker does support KDE and GNOME protocols, including their
workspace management, so you may use their pager in conjunction with Window
Maker in these.  Note that in order for this to work, you must enable support
when you configure Window Maker (using the --enable-kde and --enable-gnome
configure options).

Note also that the Blackbox pager application will work with Window Maker.

How do I use getstyle and setstyle?
...................................

To capture the current Window Maker style, use the command

.. code:: shell-session
   :class: highlight

   getstyle > current.style

To replace the current style, use the command

.. code:: shell-session
   :class: highlight

   setstyle filename.style

Why was libPropList removed from the distribution?
..................................................

Alfredo Kojima writes:

libPropList was removed from Window Maker because other programs also use it,
such as GNOME.  If libPropList is distributed with wmaker, it would cause
problems with whatever version of libPropList you already had installed.

Now, there is no more GNOME libproplist and Window Maker libproplist. There is
only libPropList which is worked on as a single community effort.

Why don't you distribute normal diff or xdelta patches?
.......................................................

Whenever possible, plain diff patches are distributed. If the new version has
new binary files, normal diff won't be able to handle them, so a patch package
is distributed instead. We don't use xdelta because a) most systems do not have
xdelta installed and b) xdelta is picky and requires the files to be patched to
be exactly the same as the one used to make the patch.  The patch package
scheme used is much more flexible.

We do not distribute a simple diff with the binary files separately (and
variations, like uuencoding the binary files) because a) it is more complicated
and error prone to require the user to manually move the files to the correct
places  b) the current patch package scheme *does* distribute the binary files
and diff files separately. If the user wants to install everything by hand,
nobody will object to that and c) sooner or later someone will certainly ask
for a script to automate the file moving stuff.

So we hacked a script (mkpatch) that automatically creates a patch package with
the normal text diff file, a list of removed files and the binary files that
have changed or been added, plus a script that does the patching automatically.
If you don't like the script, you can apply the patch and move the files
manually. Or download the whole distribution.

Will you add GNOME or KDE support?
..................................

Support for GNOME and KDE hints has been included since 0.50.0.

Note that you must enable this support at compile time with the proper
arguments to configure (--enable-kde and --enable-gnome).

How can I produce a backtrace when Window Maker keeps crashing?
...............................................................

Thanks to Paul Seelig for this answer:

You can use the GNU debugger "gdb" to get exact information about how and where
wmaker crashed.  Sending this information to the developers is the most
convenient way to help in debugging.

The wmaker binary needs to be compiled with debugging turned on ("./configure
--with-debug etc.") for this to work.

Exit wmaker and start a failsafe X session with an open xterm.

First type the command "script" to log the following session into a file
commonly called "~/typescript".  Then enter "gdb wmaker" at the shellprompt:

.. code:: shell-session
   :class: highlight

   [shell prompt]~ > script
   Script started, output file is typescript
   [shell prompt]~ > gdb wmaker
   GNU gdb 4.17.m68k.objc.threads.hwwp.fpu.gnat
   Copyright 1998 Free Software Foundation, Inc.
   GDB is free software, covered by the GNU General Public License, and you are
   welcome to change it and/or distribute copies of it under certain conditions.
   Type "show copying" to see the conditions.
   There is absolutely no warranty for GDB.  Type "show warranty" for details.
   This GDB was configured as "i486-pc-linux-gnu"...
   (gdb)

At the gdb prompt simply type "run" to start the WMaker session:

.. code::
   :class: highlight

   (gdb) run
   Starting program: /usr/bin/X11/wmaker

Try to reproduce the error which has provoked the crash before and if you
succeed the session will simply freeze and you will see something similiar to
following prompt:

.. code::
   :class: highlight

   Program received signal SIGSEGV, Segmentation fault.
   0x809ea0c in WMGetFirstInBag (bag=0x0, item=0x811e170) at bag.c:84
   84          for (i = 0; i < bag->count; i++) {
   (gdb)

Now you just type "bt" for "backtrace" and gdb will show you where the crash
happened:

.. code::
   :class: highlight

   (gdb) bt
   #0  0x809ea0c in WMGetFirstInBag (bag=0x0, item=0x811e170) at bag.c:84
   #1  0x807c542 in wSessionSaveState (scr=0x80c28e8) at session.c:299
   #2  0x807bd88 in wScreenSaveState (scr=0x80c28e8) at screen.c:1089
   #3  0x807cf54 in Shutdown (mode=WSExitMode) at shutdown.c:111
   #4  0x8078101 in exitCommand (menu=0x80f7230, entry=0x80fdb38)
   at rootmenu.c:193
   #5  0x8078403 in wRootMenuPerformShortcut (event=0xbffff360) at rootmenu.c:401
   #6  0x80630f7 in handleKeyPress (event=0xbffff360) at event.c:1492
   #7  0x8061c86 in DispatchEvent (event=0xbffff360) at event.c:232
   #8  0x8093092 in WMHandleEvent (event=0xbffff360) at wevent.c:585
   #9  0x8061dae in EventLoop () at event.c:322
   #10 0x806b238 in main (argc=1, argv=0xbffff404) at main.c:594
   (gdb)

To quit the debugger just type "quit" and say "y":

.. code::
   :class: highlight

   (gdb) quit
   The program is running.  Exit anyway? (y or n) y
   [shell prompt]~ >

To quit the script session type "exit" again:

.. code:: shell-session
   :class: highlight

   [shell prompt]~ > exit
   exit
   Script done, output file is typescript
   [shell prompt]~ >

Send the resulting "~/typescript" together with a concise explanation about how
to reproduce the bug (please use the included BUGFORM for instruction) to the
`developers <{{ site.baseurl }}/lists>`_.

----

Troubleshooting Tips
--------------------

No questions are currently available for this chapter.

----

Programming for Window Maker
----------------------------

How do I get a normal X application to produce an appicon?
..........................................................

Another insightful answer from who else but Dan Pascu.

You must define the WM_CLASS (XSetClassHint()) and the CLIENT_LEADER or
XWMHints.window_group properties, which are automatically set by most
applications that use Xt (Motif, Athena ...), but if you use plain Xlib you
must set them by hand.

Also you must make a call to XSetCommand(dpy, leader, argv, argc);

Take a look at WindowMaker-0.12.3/test/test.c that is an example for writing
such an app (which also have an app menu).

How do I get my tcl/tk application to produce an appicon?
.........................................................

Oliver Graf writes:

The main window (normally this is called '.' [dot] in tk) should use the
following lines:

.. code::
   :class: highlight

   wm command . [concat $argv0 $argv]
   wm group . .

All child windows attached to the same app-icon should use:

.. code::
   :class: highlight

   toplevel .child
   wm group .child .

where .child should be replaced by the actual window path.

Replace '.' with the actual main-window path and 'wm group .child .' should be
added for each 'toplevel .child' call.

What is WINGs?
..............

WINGs Is Not GNUstep. ;)

It is the widget library written for the widgets in Window Maker. It is
currently under heavy development but several people have started writing
applications in it. Its goal is to emulate the NeXT(tm)-style widgets.

`http://www.ozemail.com.au/~crn/wm/wings.html
<https://web.archive.org/web/20010715234415/http://members.ozemail.com.au:80/~crn/wm/wings.html>`_
is the closest thing to an information center about WINGs. You can find out
more information in our `WINGs development <wings.html>`_ section.

Where can I get more information about WINGs?
.............................................

Nic Berstein has created a WINGs development list.

The purpose of this list is to provide a forum for support, ideas, suggestions,
bug reports etc. for the WINGs widget set library.

To subscribe to this list, send a message with the word ``subscribe`` in the
**BODY** of the message to: `wings-request@postilion.org
<mailto:wings-request@postilion.org>`_.
