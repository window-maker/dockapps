---
layout: default
title: Guided Tour - Windows
---

Windows
=======

.. contents::
   :depth: 1
   :backlinks: none
   :local:

Description
-----------

General layout of a window:

- *Titlebar*: Gives the name of the application, document or window. It's color
  (usually) indicates the focus state (active or inactive window). I say
  (usually) because some styles and themes do not provide different colors for
  focused or unfocused windows - although this is rare (and, I might add,
  cruel!).
- *Miniaturize button*: Clicking on the left button of the titlebar iconifies
  the window.
- *Close button*: Clicking on the right button of the titlebar closes the
  window or kills the application.
- *Resizebar*: The bottom part of the window. Dragging the resizebar with the
  mouse resizes the window.
- *Client area*: The window content. It can be an application, some text, a
  picture...

Focusing
--------

A window can be in two states: focused or unfocused. The focused window is the
active window, the one receiving keystrokes. It's titlebar has a differentiated
color (usually!). Dialog windows or panels opened from a main window,
automatically get the focus. As soon as they are closed, the main window gets
the focus back.

Two modes are available to focus a window:

- *Click to focus mode*: clicking on any part of the window activates it.
- *Focus follows mouse mode*: moving the mouse pointer over the window
  activates it.

Reordering
----------

Windows can overlap other windows, in which case some will hide all or part of
others. Clicking on the titlebar or resizebar with the left mouse button brings
a window to the "front" (gives that window focus). Selecting a window from the
window list menu does the same.

Some key bindings are provided and are very useful when a window is hidden
behind others.

- *Meta key + click on the titlebar with left mouse button*-

  sends the window to the back and gives focus to the topmost window.

- *Meta key + click on the client area with left mouse button*-

  brings the window to the front and focuses it.

- *Meta key + Up Arrow key*-

  brings the current focused window to the front.

- *Meta key + Down Arrow key*-

  sends the current focused window to the back.

Many window attributes can be modified from the attributes panel in the window
commands menu (clicking the right mouse button on the titlebar). From version
0.62.0, window cycling was changed to Windows style (Alt-Tab).

Moving
------

Clicking on the titlebar of a window and dragging it with the left mouse button
pressed moves the window. The little box in the middle indicates the current
position in pixels relative to the top left corner of the screen (+0 +0). Extra
key bindings give more flexibility.

- Dragging the titlebar with middle mouse button: moves the window
  without changing it's stacking order.
- Dragging the titlebar + Ctrl key: moves the window without focusing it.
- Dragging the client area or the resizebar + Meta key: moves the window.

Maximizing
----------

Double-clicking the titlebar while holding the Ctrl key resizes the window's
height to full screen.

Double-clicking the titlebar while holding the Shift key resizes the window's
width to full screen.

Double-clicking the titlebar while holding both Ctrl and Shift keys resizes the
window's height and width to full screen. Double-clicking the titlebar while
holding Ctrl or Shift key restores the initial size of the window.

To prevent a maximized window from covering the dock, the "Keep on top" option
must be selected from the dock menu.

Miniaturizing
-------------

Clicking the miniaturize button (the left one on the titlebar) shrinks the
window into a miniwindow with an icon and a title and places it at the bottom
of the screen. Hitting the assigned shortcut does the same. (Default is Meta +
m.)

The miniwindow is different from the application icon in that the miniwindow
cannot be docked.

Double-clicking in the miniwindow restores a miniaturized window.
Double-clicking in an application icon with the middle mouse button restores
all miniaturized and hidden windows of this application.

Resizing
--------

The resizebar, at the bottom of the window, is divided into three regions: left
end region, middle region and right end region.

Depending upon the region you click, the resize operation is constrained to one
direction.

Clicking in the middle region of the resizebar and dragging it vertically
changes the window's height.

Clicking in either the left or right region of the resizebar and dragging it
horizontally changes the window's width.

Dragging with Shift key pressed gives the same result. Clicking in either end
region of the resizebar and dragging it diagonally changes both height and
width.

Key bindings give more options.

- Dragging the window in the client area with the right mouse button + Meta key
  resizes the window.
- Dragging the resizebar with the middle mouse button resizes the window
  without bringing it to the front.
- Dragging the resizebar + Ctrl key resizes the window without focusing it.

Shading
-------

Double-clicking on the titlebar of a window shades it. This means the window
rolls up to it's titlebar. A shaded window has almost the same properties as a
normal window. It can be miniaturized or closed.

From version 0.80.0, you can shade/unshade a window using a mouse wheel on its
titlebar. This of course, assumes your system is able to manage a mouse wheel.
The WMGLOBAL file in you $HOME/GNUstep/Defaults should contain two new
directives : MouseWheelUp and MouseWheelDown.

Hiding
------

Clicking the the miniaturize button (the left one on the titlebar) with the
right mouse button hides the application. Using the middle mouse button unhides
the application, simultaneously opening the windows list menu and selecting the
hidden application. (Pressing both buttons at once with a two buttons mouse
does the same on some OSes.) If this doesn't work, use the F11 key binding (the
default) to open the windows list menu.

Closing
-------

Clicking the close button (the right one on the titlebar) closes the window.
When the close button has a different form (not an X), it means an application
is running in that window. Double-clicking in this close button kills the
application. This can be done too with *Ctrl key + clicking the close button*.

Usually, it's much better to exit an application from inside (through it's
menu, for instance).

Commands menu
-------------

Clicking on the titlebar of a window with the right mouse button opens a menu
containing commands applying to this window. The keyboard shortcut Ctrl + Esc
can replace the click on the titlebar. Esc closes this menu.

List of Commands Menu commands:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

*Maximize/Unmaximize*:

Either maximizes or returns the window to it's initial state.

*Miniaturize*:

Miniaturizes the window (miniwindow). The keyboard shortcut is Meta + m.

*Shade/Unshade*: Shades (or unshades) the window.

*Hide*:

Hides all windows of the application. Clicking on the application icon unhides
the windows.

*Hide Others*:

From version 0.80.1 it is possible to hide all others windows. The window list
menu allows to unhide selecting the window to redisplay.

*Resize/Move*:

When this menu option is selected, the window is ready to be moved or resized
(the little box with coordinates is displayed inside the window). Clicking on
the titlebar deselects the option.

*Select*:

Obviously selects the window which then can be moved or resized... Reselecting
this option deselects the window.

*Move to*:

Allows to move the window to another workspace (if existing!).

*Attributes*:

Opens the attributes panel to edit attributes and options for the window.

Five options are available in this panel: Window specification, Window
attributes, Advanced options, Icon and initial workspace and application
specific.

- Window specification: Defines that the configuration will apply to windows
  having their WM_CLASS property set to the selected name. This is because
  windows can have different names. From version 0.65.0, you can select the
  window to get the right specification.

- Window attributes: selecting the corresponding checkbox allows to:

  - disable titlebar
  - disable resizebar
  - disable close button
  - disable miniaturize button
  - disable border
  - keep on top
  - keep at bottom
  - omnipresent
  - start miniaturized
  - start maximized
  - full screen maximization

- Advanced options: selecting the corresponding checkbox allows to:

  - don't bind keyboard shortcuts
  - don't bind mouse clicks
  - don't show in the window list
  - don't let the window take focus
  - keep inside screen
  - ignore "Hide others"
  - ignore "Save session"
  - emulate application icon

- Icon and initial workspace: allow to

  - choose an icon browsing directories
  - ignore client supplied icon when selecting the checkbox
  - define initial workspace

- Application specific: selecting checkboxes allows to:

  - start hidden or with no application icon
  - collapse application icons (from version 0.65.0)

- From version 0.80.0 a new checkbox is available : "Shared application icon".
  It replaces the "Collapse application icon" checkbox. That is, you can have
  many open windows from the same application with only one appicon. This
  feature is on by default except for some incompatible applications. This
  behavior can be defined for all windows in the Window Specification inspector
  selecting the Defaults for all windows checkbox.

You can revert to the old behavior changing SharedAppIcon to "No" in the
WMWindowAttributes file, either in the global domain or in the local domain :
$HOME/GNUstep/Defaults.

*Options*:

Submenu options allow to:

- to keep the window on top
- to keep the window at bottom
- to keep the window omnipresent
- to set shortcuts for the window

Ten shortcuts are available as soon as they have been set in the keyboard
shortcut dialog. The shortcuts to define are those named "Shortcut for window +
figure". Then, using the defined shortcut gives the focus to the window.

*Close*:

Closes the window

*Kill*:

Kills the application. Usually, an application must be closed from inside (menu
or other means). This option is especially reserved for "emergency" cases.
