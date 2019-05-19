---
layout: default
title: WINGs
---

WINGs Is Not GNUstep
====================

While GNUstep is our ideal development framework, it's overkill for a window
manager like Window Maker. We had a need for a quick, lightweight toolkit to
handle basic window manager tasks, which is how WINGs was born, and why it has
become an integral part of Window Maker's core.

Unlike the general uses of the GNUstep development environment, the WINGs
toolkit was designed as a specific solution for Window Maker. It is not
implemented in an object-oriented language, but was designed with OO schemas in
mind. It is encapsulated in objects that have various methods (functions),
which in turn can be accessed like real objects (i.e it's unknown what they
contain, and they only have the interface functions to alter their data). As
much as C will allow, that is. What really matters is that it's functional and
small enough for our purposes.

Surprisingly, there have been several developers who think WINGs is mature and
functional enough to write full fledged applications with it. For developers
who are interested in creating real applications, we would encourage them to
look at GNUstep instead. GNUstep is written in Objective-C, and anyone with a
solid C++ background shouldn't need more than an hour to begin programming in
Objective-C. For more information on this, please visit the `GNUstep Developer
Documentation <http://www.gnustep.org/developers/documentation.html>`_ section.

So, what does WINGs do for us, specifically? It contains many necessary
widgets, such as the buttons, file browser, color chooser, and text editor
dialog that are all used for creating the UI. It is currently missing a few
important items, such as DnD, treeview, and application menus, but those will
be integrated in future releases.

One of the more important aspects of WINGs is that it now provides proplist
functionality. proplist, short for `property list
<http://en.wikipedia.org/wiki/Property_list>`_, is what Window Maker uses to
generate and maintain structured configuration files. This data is stored as
plain ASCII text under a user's ~/GNUstep directory.  These files are what make
up the menus, the current state and appearance of the desktop, the Dock, the
Clip, and the values set in WPrefs.

As an example, here is a short snippet from the proplist version of the default
menu:

.. code::
   :class: highlight

   (
           Applications,
           (
                   Info,
                   ("Info Panel", INFO_PANEL),
                   (Legal, LEGAL_PANEL),
                   ("System Console", EXEC, xconsole),
                   ("System Load", SHEXEC, "xosview || xload"),
                   ("Process List", EXEC, "xterm -e top"),
                   ("Manual Browser", EXEC, xman)
           ),
           (Run..., EXEC, "%a(Run,Type command to run:)"),
   ...
   )

External sources of information
===============================

As this section evolves, we will be providing more documentation on the
internals of WINGs. For the time being, developers interested in WINGs should
see Alexey Voinov's `WINGsman documentation project
<http://voins.program.ru/windowmaker/wingsman.html>`_. Starters may find `this
tutorial </WINGs_tutorial/WINGtoc.html>`_, which includes a library listing
based on Voinov's work, useful. We'll try to cover some examples and/or more
tutorials on how to program small applications in WINGs in the near future. For
anyone already using WINGs for a project, please `contact us <{{ site.baseurl
}}/lists>`_, as we'd like to get an idea of its popularity and practical uses,
as well as some additional material to place here.
