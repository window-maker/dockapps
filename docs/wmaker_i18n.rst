---
layout: default
title: Internationalisation
---

Window Maker Internationalisation
=================================

A guide to enable support for language translations in WINDOW MAKER and to the
contributors who want to help translating.

.. sectnum::
.. contents:: Table of Contents
   :backlinks: none

This manual is for Window Maker, version git#next.

----

Enabling Languages support
--------------------------

WINDOW MAKER has the possibility to be translated in many languages, but by
default none of them will be installed, and the support for translation will
not be compiled.

To enable the translation capabilities, you have to specify which language(s)
you want to be installed: this is done with the variable ``LINGUAS`` when
running the ``configure`` script. This variable should contain the
space-separated list of languages you want to install.

You could for instance enable both French (``fr``) and Dutch (``nl``) with
this:

.. code:: console
   :class: highlight

   $ ./configure LINGUAS="fr nl"

You can of course add any other option that you want to the ``configure``
command. From the moment you specify the variable, the ``configure`` script
will check that you have the appropriate dependencies for this (basically the
``gettext`` function and the ``libintl`` library); when you run ``make`` to
compile the project, it will also compile the translation (``mo`` files) for
the language(s) you asked (if available, of course), and during ``make
install`` it will install them in the usual directory.

The installation directory can be changed with the standard option
``--localedir`` to the ``configure`` script, the default path being
``PREFIX/share/locale/<lang>/LC_MESSAGES``).

Getting the list of supported languages
.......................................

The naming convention for the languages follows the ``ISO 639-1`` standard, for
which you can find a summary list in the `GNU gettext manual
<https://www.gnu.org/software/gettext/manual/html_node/Usual-Language-Codes.html>`_.

But as WINDOW MAKER does not support all of them, the ``configure`` script will
print a warning for each language you specify that it does not know, and sum up
at the end the list of enabled languages that will be installed.

There is a non-standard possibility to set ``LINGUAS`` to ``list``, in which
case the ``configure`` script will provide you the list of languages it
supports, and stop:

.. code:: console
   :class: highlight

   ./configure LINGUAS="list"

There is also another non-standard possibility to enable all the languages that
WINDOW MAKER supports by setting ``LINGUAS`` to ``*``. This is an internal
trick implemented so the development team can have the command ``make
distcheck`` include some checks on translations:

.. code:: console
   :class: highlight

   ./configure LINGUAS='*'

Translations for Menus
......................

In order to propose an *Application Menu* (also called *Root Menu*) that is
also translated in the language of the interface, WINDOW MAKER implements two
complementary mechanisms:

The first, always enabled when i18n support is enabled, is to look for the menu
file containing the name of the locale. For example, if the file is called
``menu`` and the language is set as ``LANG=fr_FR.utf-8``, then WINDOW MAKER
will search for, and use the first match found:

- ``menu.fr_FR.utf-8``
- ``menu.fr_FR``
- ``menu.fr``
- ``menu``

The second possibility, which is not enabled by default, is to be able to use a
custom ``po`` file which contains the translations for the text of the menu.
This feature is enabled at compile time, using the option
``--with-menu-textdomain`` to the ``configure`` script. For example, if you
specify:

.. code:: console
   :class: highlight

   ./configure --with-menu-textdomain=WMMenu

then the translations for the menu will be searched in the file ``WMMenu.mo``
located at the standard location, the default path being
`PREFIX/share/locale/[lang]/LC_MESSAGES/WMMenu.mo`.

If you do not enable the feature (the default behaviour, or with an explicit
``--without-menu-textdomain``), then WINDOW MAKER will **not** try to translate
the strings, even using its own domain file (``WindowMaker.mo``).


Setting ``LINGUAS`` at system level
...................................

As the variable ``LINGUAS`` is quite standard, you also have the possibility to
set its value in the ``config.site`` file for AUTOCONF. This file can be placed
in one of these paths:

- ``PREFIX/share/config.site``
- ``PREFIX/etc/config.site``

This way, the same language list will be used for all the programs that use
AUTOCONF that you would compile. Please note that if you also specify a value
on the command line, it will have precedence over the value in that file.

----

Choosing the Language
---------------------

If you have compiled and installed WINDOW MAKER with support for your language,
the effective translation is done is the very same way as any other application
on an UNIX system, you just have to set the shell variable ``LANG`` to your
language before ``wmaker`` is started. In ``sh`` type of shell (SH, KSH, BASH,
...), this is done for example with (``fr`` is for French):

.. code:: console
   :class: highlight

   export LANG=fr

There is also a command line option ``--locale`` for WINDOW MAKER which may be
used to set the language:

.. code:: console
   :class: highlight

   wmaker --locale fr

When using this option, WINDOW MAKER will use the locale you specified,
redefining the ``LANG`` environment variable to this value so all program
started from WINDOW MAKER will inherit its value.

If your system is using SYSTEMD, you can also configure the locale at system
level using the command:

.. code:: console
   :class: highlight

   localectl set-locale LANG=fr

You can check if the current value is properly supported with the command:

.. code:: console
   :class: highlight

   locale


If this does not work, you may need first to activate the support for your
locale in the system; you can get the list of currently enabled locales with
the command:

.. code:: console
   :class: highlight

   locale -a

You should be able to enable a new language support by editing the file
``/etc/locale.gen`` to uncomment the locale(s) you need (by removing the ``#``
character and space(s) in front of it, and by running the command
``locale-gen`` as root.

For further information, you may wish to read dedicated documentation, for
example from `the Linux Documentation Project
<http://tldp.org/HOWTO/HOWTO-INDEX/other-lang.html>`_ or through pages like
`Shell Hacks' note on Changing Locale
<http://www.shellhacks.com/en/HowTo-Change-Locale-Language-and-Character-Set-in-Linux>`_.

----

Troubleshooting
---------------

If I18N support does not work for you, check these:


- the ``LANG`` environment variable is set to your locale, and
  the locale is supported by your OS's locale or X's locale
  emulation. you can display all supported locales by
  executing "``locale -a``" command if it is available; you
  can check if your locale is supported by X's locale emulation,
  see ``/usr/share/X11/locale/locale.alias``

- check if you are using an appropriate fonts for the locale you chose. If
  you're using a font set that has a different encoding than the one used by
  XLIB or LIBC, bad things can happen. Try specifically putting the encoding in
  the ``LANG`` variable, like ``ru_RU.KOI8-R``. Again, see
  ``/usr/share/X11/locale/locale.alias``

- the fonts you're using support your locale. if your font setting on
  ``$HOME/GNUstep/Defaults/WindowMaker`` is like...

  .. code:: ini
     :class: highlight

     WindowTitleFont = "Trebuchet MS:bold:pixelsize=12";
     MenuTitleFont   = "Trebuchet MS:bold:pixelsize=12";

  then you can't display Asian languages (``ja``, ``ko``, ``ch``, ...)
  characters using ``Trebuchet MS``. A font that is guaranteed to work for any
  language is ``sans`` (or ``sans-serif``). ``sans`` is not a font itself, but
  an alias which points to multiple fonts and will load the first in that list
  that has the ability to show glyphs in your language. If you don't know a
  font that is suited for your language you can always set all your fonts to
  something like:

  .. code:: ini
     :class: highlight


     "sans:pixelsize=12"


  However, please note that if your font is something like:

  .. code:: ini
     :class: highlight

     "Trebuchet MS,sans serif:pixelsize=12"

  this will not be able to display Asian languages if any of the previous fonts
  before sans are installed. This is because unlike the proper font pickup that
  ``sans`` guarantees for your language, this construct only allows a font
  fallback mechanism, which tries all the fonts in the list in order, until it
  finds one that is available, even if it doesn't support your language.

  Also you need to change font settings in style files in the
  ``$HOME/Library/WindowMaker/Style`` directory.

- the ``LC_CTYPE`` environment variable is unset or it has the correct value.
  If you don't know what is the correct value, unset it.

----

Contribute to Translations
--------------------------

You may have noticed that many translations are not up to date, because the
code has evolved but the persons who initially contributed may not have had the
time to continue, so any help is welcome.

Since WINDOW MAKER 0.95.7 there are some targets to ``make`` that can help you
in that task.

Install the latest sources
..........................

If you want to contribute, the first step is get the development branch of the code;
this is done using ``git``. If you do not feel confident at all with using
``git``, you may also try to ask for a *snapshot* on the developer's mailing
list `wmaker-dev@lists.windowmaker.org
<mailto:wmaker-dev@lists.windowmaker.org>`_. With ``git`` the procedure is:

.. code:: bash
   :class: highlight

   # Get your working copy of the sources
   git clone git://repo.or.cz/wmaker-crm.git

   # Go into that newly created directory
   cd wmaker-crm

   # Switch to the branch where everything happens
   git checkout next

   # Generate the configuration script
   ./autogen.sh

Now you should have an up-to-date working copy ready to be compiled; you will
not need to go the full way but you should run the ``configure`` script, so it
will create the ``Makefile`s``, and you may want to compile the code once so it
will not do it again automatically later while you are doing something else:


.. code:: console
   :class: highlight

   # Setup the build, enabling at least the language you want to work on
   ./configure LINGUAS="<list of iso 639 country code>"

   # Compile the code once
   make

Updating the Translations
.........................

The typical process for translating one program is:

- generate a POT file (PO Template): this is done with ``xgettext`` which
  searches for all the strings from the sources that can be translated;
- update the PO file for your language: this is done with ``msgmerge`` which
  compares the PO file and aligns it to the latest template;
- edit the new PO file: this is done by you with your favourite editor, to add
  the missing ``msgstr``, review the possible *fuzzy matches*, …
- check the PO file: unfortunately there is no definitive method for this;
- submit your contribution to the project: this is done with ``git``.

In WINDOW MAKER, you have actually 4 ``po`` files to take care of:

- ``po/<LANG>.po``: for WINDOW MAKER itself
- ``WPrefs.app/po/<LANG>.po``: for the Preference Editor program
- ``WINGs/po/<LANG>.po``: for the graphic toolkit library
- ``util/po/<LANG>.po``: for the command-line tools of WINDOW MAKER

As stated previously, there is a ``make`` target that can help you to
automatically generate the POT and update the PO for these 4 cases:


.. code:: console
   :class: highlight

   make update-lang PO=<LANG>

Once run, it will have updated as needed the 4 ``po`` files against the latest
source code. You may wish to use the command ``git gui`` to view the changes;
you can now edit the files to complete the translation, correct them, remove
deprecated stuff, … Please note that the encoding should be set to *UTF-8* as
this is now the standard.

.. TODO: change mailing list address

If you think an error message is too obscure, just ask on the developer mailing
list `wmaker-dev@lists.windowmaker.org
<mailto:wmaker-dev@lists.windowmaker.org>`_: in addition to clarifications
there's even a chance for the original message to be improved!

You may find some information on working with ``po`` file in the `GNU gettext
documentation
<https://www.gnu.org/software/gettext/manual/html_node/Editing.html>`_.

Translate the Man Pages
.......................

You may want to extend the translation to the documentation that is provided to
users in the form of Unix *man pages*. The sources of the man pages are located
in the ``doc/`` directory; the translation should be placed in the directory
``doc/LANG/`` with the same file name.

.. TODO: change mailing list address

The directory will also need a file ``Makefile.am`` which provides the list of
man pages to be included in the distribution package and to be installed. You
can probably get inspiration from an existing one from another language; if you
do not feel confident about it do not hesitate to ask on the project's mailing
list (`wmaker-dev@lists.windowmaker.org
<mailto:wmaker-dev@lists.windowmaker.org>`_), either for help or to ask someone
to make it for you.

Please note that although most man pages sources are directly in man page
format (*nroff*, the file extension being a number), a few of them are
processed by a script (those with the ``.in`` extension, like ``wmaker.in``).
This is done because in some case we want the man page to reflect the actual
compilation options.

You may not want to bother with this hassle, in which case you can simply name
your translation file with the ``.1`` and remove the special ``@keyword@``
marks. If you are sure you want to keep that processing but do not feel
confident about hacking the ``Makefile.am`` do not hesitate to ask on the
project's mailing list (`wmaker-dev@lists.windowmaker.org
<mailto:wmaker-dev@lists.windowmaker.org>`_).

Checking the Result
...................

In the WINDOW MAKER build tree you also have another target that can help you,
it is ``make check``.

At current time, it does not check much, but if during the ``make update-lang``
new ``po`` file have been created you may get some errors, because you have to
add these new files to the variable ``EXTRA_DIST`` in the corresponding
``Makefile``.

If you do not feel confident about doing it, do not worry, just tell about it
when you submit your work, and some developer on the mailing list will just be
happy to do it for you when integrating your valuable contribution (we always
like when someone helps making WINDOW MAKER better).

Submitting your Contribution
............................

*Preliminary Remark*: if the update process made changes in a ``po`` file but
you did not change any ``msgstr`` content, it is probably a good idea to not
submit the changes to that ``po`` file because it would just add noise.

When you feel ready to send your changes, the first step is to prepare them.
This is done with ``git``: if you have not run the ``git gui`` previously then
it is a good time to do it now. This window offers you the possibility to show
your changes and to decide what you want to send.

The window is divided in 4 panes:

- top-right show the current changes you have selected, for review (and also
  for cherry-picking stuff if you want to select precisely)
- top-left ("Unstaged Changes") the list of files with changes to be send, you
  can click on the name of the file to see the changes, you can click on the
  icon of the file if you want to send all the changes in this file; an icon in
  blue shows a file that have been changed and an icon in black shows a file
  that is new
- bottom-left ("Staged Changes") the list of files with changes that you have
  chosen to send so far, you can click on the file name to view these changes,
  you can click on the icon if you want to remove the changes from this file
  from the list to send

- bottom-right ("Commit Message") the message you want to attach to your
  changes when you submit them to the development team

The idea here is to pick your changes to the ``po`` files; for the *commit
message* you may wish to stuck to a simple, single line:

| "Updated translations for <LANG>"

The penultimate step is to click on the button ``Sign Off`` (it will add a line
in the commit message), and then click on the button ``Commit``. From this
time, the commit message will clear itself and the "Staged Changes" also,
showing that your action was done.

You may now quit the ``git gui``, the final step begins by running this
command:

.. code:: console
   :class: highlight

   git format-patch HEAD^

.. TODO: change mailing list address

This will generate a file named like ``0001-updated-translations-for-XX.patch``
which contains your changes, ready for sending.  The goal will now be to email
this file to `wmaker-dev@lists.windowmaker.org
<mailto:wmaker-dev@lists.windowmaker.org>`_.  If you feel confident in having
``git`` send it for you, you may want to read the file
``The-perfect-Window-Maker-patch.txt`` to see how to configure ``git`` for
mailing, so you can run:

.. code:: console
   :class: highlight

   git send-email 0001-updated-translations-for-XX.patch
