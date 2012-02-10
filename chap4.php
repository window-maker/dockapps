<html xmlns="http://www.w3.org/1999/xhtml">
<head>
  <meta name="generator" content="HTML Tidy for Linux (vers 25 March 2009), see www.w3.org" />

  <title>User Guide: Configuration</title>
  <meta http-equiv="Content-Type" content="text/html; charset=us-ascii" />
  <link href="title.css" rel="stylesheet" type="text/css" />
</head>

<body>
  <?php include("dock.php");  ?>
  <?php include("header.php"); ?>

    <table class="inner" border="0" cellpadding="1" cellspacing="1">
      <tr>
        <td content="content" colspan="2" valign="top">
          <br />
          <br />
          <br />

          <h2>Chapter 4</h2>

          <h1>Configuring Window Maker</h1><br />
          <br />
          <a name="4.1"></a>

          <h2><a name="4.1">4.1 The Defaults System</a></h2><a name="4.1"></a><br />
          WindowMaker uses a defaults database for storing various information, like configurations and other data that must be
          kept between sessions (like the list of applications of a saved session). The defaults database is stored as
          <i>property lists</i> in the $(HOME)/GNUstep/Defaults directory. Each file in the $(HOME)/GNUstep/Defaults directory
          contains data that belongs to a specific <i>domain</i>.<br />
          <br />
          Any application can use the defaults database to store its information. Generally an application will have one or more
          <i>domains</i> that belong to it.<br />
          <br />
          <br />
          <a name="4.1.1"></a>

          <h3><a name="4.1.1">4.1.1 Property list File Format</a></h3><a name="4.1.1"></a><br />
          <br />
          The syntax of the property list is simple, but, if you need to change it manually you must take care not to leave any
          syntax errors.<br />
          <br />
          The EBNF for the property list is the following:<br />
          <br />

          <h3><b>Description of the syntax of a property list in the Bacchus Naur Form (BNF)</b></h3>

          <center>
            <table border="1" width="80%" cellspacing="0" cellpadding="5">
              <tbody>
                <tr>
                  <td align="center" valign="middle">
                    <table border="0" width="100%" cellspacing="0" cellpadding="5">
                      <tbody>
                        <tr>
                          <td align="left" valign="top"><font face=
                          "Times New Roman, Times, Times Roman">&lt;object&gt;</font></td>

                          <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">::=</font></td>

                          <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">&lt;string&gt; |
                          &lt;data&gt; | &lt;array&gt; | &lt;dictionary&gt;</font></td>
                        </tr>

                        <tr>
                          <td align="left" valign="top"><font face=
                          "Times New Roman, Times, Times Roman">&lt;string&gt;</font></td>

                          <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">::=</font></td>

                          <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">text with
                          <i>non-alphanumeric characters</i> | alphanumeric text</font></td>
                        </tr>

                        <tr>
                          <td align="left" valign="top"><font face=
                          "Times New Roman, Times, Times Roman">&lt;array&gt;</font></td>

                          <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">::=</font></td>

                          <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">`(' [ &lt;object&gt; {
                          `,' &lt;object&gt; }* ] `)'</font></td>
                        </tr>

                        <tr>
                          <td align="left" valign="top"><font face=
                          "Times New Roman, Times, Times Roman">&lt;dictionary&gt;</font></td>

                          <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">::=</font></td>

                          <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">`{' [
                          &lt;keyval_pair&gt; { `,' &lt;keyval_pair&gt; }* ] `}'</font></td>
                        </tr>

                        <tr>
                          <td align="left" valign="top"><font face=
                          "Times New Roman, Times, Times Roman">&lt;keyval_pair&gt;</font></td>

                          <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">::=</font></td>

                          <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">&lt;string&gt; `='
                          &lt;object&gt; `;'</font></td>
                        </tr>
                      </tbody>
                    </table>
                  </td>
                </tr>
              </tbody>
            </table>
          </center><br />
          <br />

          <h3><b>Example property list file</b></h3>
          <pre>
<font face="Courier New, Courier"> {
        "*" = {
                Icon = "defaultAppIcon.xpm";
        };
        "xterm.XTerm" = {
                Icon = "xterm.xpm";
        };
        xconsole = {
                Omnipresent = YES;
                NoTitlebar = YES;
                KeepOnTop = NO;
        };
 }
</font>
</pre>The property list above is a dictionary with 3 dictionaries inside. The first is keyed by ``*'', the second by
``XTerm.xterm'' and the last by ``xconsole''.<br />
          <br />
          Note that all strings that have non-alphabetic or numeric characters (like a dot ``.'' or the asterisk ``*'' are
          enclosed by double quotes. Strings with only alphanumeric characters may or may not be enclosed in double quotes, as
          they will not make any difference.<br />
          <br />
          Here is another example:<br />
          <br />
          <pre>
<font face="Courier New, Courier">{
        FTitleBack = ( hgradient, gray, "#112233" );
}
</font>
</pre><br />
          <br />
          The property list in the example above contains an array with 3 elements with a key named ``FTitleBack''.<br />
          <br />
          Except for cases like file names and paths, all value strings are case insensitive, i.e.: YES = Yes = yes = yEs<br />
          <br />
          <a name="Value Types"></a>

          <h3><a name="Value Types">4.1.2 Value Types</a></h3><a name="Value Types"></a><br />
          Here is a description of some of the types of values that an option might have:<br />
          <br />

          <center>
            <table border="0" width="80%" cellspacing="0" cellpadding="0">
              <tbody>
                <tr>
                  <th align="left"><font face="Times New Roman, Times, Times Roman">Type</font></th>

                  <th align="left"><font face="Times New Roman, Times, Times Roman">Value</font></th>
                </tr>

                <tr>
                  <td colspan="2">
                    <hr />
                  </td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">YES or NO</font></td>
                </tr>

                <tr>
                  <td colspan="2">
                    <hr />
                  </td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">integer</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">any integer number, usually
                  limited by a range that will be indicated</font></td>
                </tr>

                <tr>
                  <td colspan="2">
                    <hr />
                  </td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">positive integer</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">any integer number greater than
                  or equal to zero (0)</font></td>
                </tr>

                <tr>
                  <td colspan="2">
                    <hr />
                  </td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">speed</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">UltraFast, Fast, Medium, Slow,
                  or VerySlow</font></td>
                </tr>

                <tr>
                  <td colspan="2">
                    <hr />
                  </td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">mouse button</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Left, Middle, Right, Button1,
                  Button2, Button3, Button4, or Button5</font></td>
                </tr>

                <tr>
                  <td colspan="2">
                    <hr />
                  </td>
                </tr>
              </tbody>
            </table>
          </center><br />
          <br />
          <br />
          <a name="4.1.3"></a>

          <h3><a name="4.1.3">4.1.3 Preferences</a></h3><a name="4.1.3"></a> General preference options are stored in the
          <i>WindowMaker</i> domain; i.e. the $(HOME)/GNUstep/Defaults/WindowMaker file.<br />
          <br />
          Changes in preference options will automatically affect the current WindowMaker session, without a restart. Some
          options, however, require a restart of WindowMaker before they take effect. Such options are marked with a * .<br />
          <br />
          Note that values marked as <i>Default</i> are values that are assumed if the option is not specified, instead of
          <i>factory default</i> values that are set in the preference file.<br />
          <br />

          <center>
            <table border="0" width="95%" cellspacing="0" cellpadding="5">
              <tbody>
                <tr>
                  <th align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Option</font></th>

                  <th align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Value</font></th>

                  <th align="leftt" valign="top"><font face="Times New Roman, Times, Times Roman">Description</font></th>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">PixmapPath</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">list of directories separated by
                  ":" (default: depends on the system)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">A list of directories where
                  pixmaps can be found. The pixmaps for things like icons, are searched in these paths in order of
                  appearance.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">*NoDithering</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean (default:
                  NO)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Disable internal dithering of
                  images. Not recommended for displays with less than 8 bits per pixel.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">*ColormapSize</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">integer number &gt; 1 (default:
                  4)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Number of colors for each of the
                  red, green and blue components to be used for the dithering colormap. This value must be greater than 1 and
                  smaller than 6 for 8bpp displays. It only makes sense on PseudoColor displays. This option has not effect on
                  TrueColor displays. Larger values result in better appearance, but leaves less colors for other
                  applications.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">*ModifierKey</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">modifier key name (default:
                  Mod1)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">The key to use as the modifier
                  being referred as Meta in this manual, like Meta dragging a window to move it. Valid values are Alt, Meta,
                  Super, Hyper, Mod1, Mod2, Mod3, Mod4, Mod5.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">UseSaveUnders</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean (default:
                  NO)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Use <i>saveunders</i> in
                  WindowMaker windows. This can improve performance but increases memory usage. It also can cause problems with
                  refreshing in some applications.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">DisableClip</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean (default:
                  NO)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Will remove the application Clip
                  from the workspace.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">DisableDock</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean (default:
                  NO)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Will remove the application Dock
                  from the workspace</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">Superfluous</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean (default:
                  NO)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Enable extra animations and
                  other cosmetic things that might increase peak memory and CPU usage.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">SaveSessionOnExit</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean (default:
                  NO)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Automatically save the state of
                  the session when exiting WindowMaker.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">*IconSize</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">integer &gt; 4 (default:
                  64)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">The size of application icons
                  and miniwindows.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">OpaqueMove</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean (default:
                  NO)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Whether the whole window should
                  be moved while dragging it, or, if only it's frame should be dragged.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">FocusMode</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Manual or CickToFocus, Auto or
                  FocusFollowsMouse, SemiAuto or Sloppy (default: ClickToFocus)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">The mode of input focus setting.
                  Refer to section <a href="chap2.php#2.2.1">2.2.1, Focusing a
                  Window</a></font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">IgnoreFocusClick</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean (default:
                  NO)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Whether the mouse click use to
                  focus a window should be ignore or treated normally.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">AutoFocus</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean (default:
                  NO)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Whether newly created windows
                  should receive input focus. Do not confuse with FocusMode=Auto.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">RaiseDelay</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">integer number (default:
                  0)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">How many tenths of a second to
                  wait before raising a window in Auto or Semi-Auto focus mode. 0 disables this feature.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">DoubleClickTime</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">integer number (default:
                  250)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">If two mouse clicks occur in
                  this interval of time, it will be considered a double click.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">ColorMapMode</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Manual or ClickToFocus, Auto or
                  FocusFollowsMouse (default: auto)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">The mode of colormap setting. In
                  <i>Manual</i> or <i>ClickToFocus</i> mode, the colormap is set to the one belonging to the current focused
                  window. In <i>Auto</i> or <i>FocusFollowsMouse</i> mode, the colormap is set to the one belonging to the window
                  under the pointer.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">CirculateRaise</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean (default:
                  NO)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Whether the window should be
                  raised when circulating. (focus the next or previous window through the keyboard)</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">OnTopTransients</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean (default:
                  NO)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Whether transient windows should
                  always be placed over their owners</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">WindowPlacement</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">auto, cascade, manual, or random
                  (default: cascade)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Sets placement mode for new
                  windows. <i>Auto</i> places the window automatically in the first open space found in the workspace.
                  <i>Cascade</i> places the window in incrementing positions starting from the the top-left corner of the
                  workspace. <i>Manual</i> allows you to place the window interactively with the mouse. <i>Random</i> paces the
                  window randomly in the workspace.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">WindowPlaceOrigin</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">(X,Y) where X and Y are integer
                  numbers (default: (0,0))</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Sets the offset, from the
                  top-left corner of the screen, to place windows. In non-manual WindowPlacement modes windows will not be placed
                  above or to the left of this point.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">AutoArrangeIcons</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean (default:
                  NO)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Whether icons should be
                  automatically arranged</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">ResizeDisplay</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">center, corner, floating, or
                  line (default: corner)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Selects the type or position of
                  the box that shows the window size when a window is being resized. <i>center</i> places the box in the center
                  of the workspace, <i>corner</i> places it in the top-left corner of the workspace, <i>floating</i> places it in
                  the center of the window being resized and <i>line</i> draws the current window size over the workspace, like
                  in a technical drawing.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">MoveDisplay</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">center, corner or floating
                  (default: corner)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Selects the type or position of
                  the box that shows the window position when a window is being moved. The value meanings are the same as for the
                  ResizeDisplay option.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">AlignSubmenus</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean (default:
                  NO)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Whether submenus should be
                  aligned vertically with their parent menus.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">WrapMenus</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean (default:
                  NO)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Whether submenus should be
                  placed to the right of their parent menus when they don't fit the screen. Note that menus placed off the screen
                  can be scrolled.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">ScrollableMenus</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean (default:
                  NO)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Whether menus that are not fully
                  inside the screen should automatically scroll when the pointer is over them and near the border of the
                  screen.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">MenuScrollSpeed</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">speed (default:
                  medium)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">The scrolling speed of
                  menus.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">DontLinkWorkspaces</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean (default:
                  NO)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Do not automatically switch to
                  the next or previous workspace when a window is dragged to the edge of the screen.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">NoWindowUnderDock</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean (default:
                  NO)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">When maximizing windows, limit
                  their sizes so that they will not be covered by the dock.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">NoWindowOverIcons</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean (default:
                  NO)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">When maximizing windows, limit
                  their sizes so that they will cover miniwindows and application icons.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">StickyIcons</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean (default:
                  NO)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Whether miniwindows should be
                  present in all workspaces.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">CycleWorkspaces</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean (default:
                  NO)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Set to YES if you want windows
                  that are dragged past the last workspace to be moved to the first workspace, and vice-versa.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">AdvanceToNewWorkspace</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean (default:
                  NO)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Whether windows dragged past the
                  last workspace should create a new workspace.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">DisableAnimations</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean (default:
                  NO)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Whether animations, like the one
                  done during minimization, should be disabled.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">IconSlideSpeed</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">speed (default:
                  medium)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">The speed of icons when they are
                  being slid across the workspace.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">ShadeSpeed</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">speed (default:
                  medium)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">The speed of the shading
                  animation.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">DisableSound</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean (default:
                  NO)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Whether sound support in
                  WindowMaker should be disabled</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">*DisableWSMouseActions</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean (default:
                  NO)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Whether actions in the workspace
                  triggered by mouse-clicks should be disabled. This allows the use of file and desktop managers that place icons
                  on the root window (such as KDE).</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">SelectWindowMouseButton</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">mouse button (default:
                  left)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">The mouse button that activates
                  selection of multiple windows in the workspace.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">WindowListMouseButton</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">mouse button (default:
                  middle)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">The mouse button that opens the
                  window list menu in the workspace.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">ApplicationMenuMouseButton</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">mouse button (default:
                  right)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">The mouse button that opens the
                  applications menu in the workspace.</font></td>
                </tr>
              </tbody>
            </table>
          </center><br />
          <br />

          <h3><b>Appearance Options</b></h3><br />
          Fonts are specified in the X Logical Font Description format. You can cut and paste these names from programs like
          <font face="Courier New, Courier">xfontsel</font>.<br />
          <br />
          Colors are specified as color names in the standard X format. This can be any color name shown by the <font face=
          "Courier New, Courier">showrgb</font> program (like black, white or gray) or a color value in the #rrggbb format, where
          rr, gg and bb is the intensity of the color component (like #ff0000 for pure red or #000080 for medium blue). Note that
          color names in the #rrggbb format must be enclosed with double quotes.<br />
          <br />
          Textures are specified as an array, where the first element specifies the texture type followed by a variable number of
          arguments.<br />
          <br />
          Valid texture types are:<br />
          <br />

          <center>
            <table border="0" width="90%" cellspacing="0" cellpadding="5">
              <tbody>
                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">(solid, color)</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">the texture is a simple solid
                  color.</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">(dgradient, color1,
                  color2)</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">the texture is a diagonal
                  gradient rendered from the top-left corner to the bottom-right corner. The first argument (color1) is the color
                  for the top-left corner and the second (color2) is for the bottom-right corner.</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">(hgradient, color1,
                  color2)</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">the texture is a horizontal
                  gradient rendered from the left edge to the right edge. The first argument (color1) is the color for the left
                  edge and the second (color2) is for the right edge.</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">(vgradient, color1,
                  color2)</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">the texture is a vertical
                  gradient rendered from the top edge to the bottom edge. The first argument (color1) is the color for the top
                  edge and the second (color2) is for the bottom edge.</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">(mdgradient, color1,
                  color2,...,color<i>n</i>)</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">this is equivalent to drgadient,
                  but you can specify more than two colors</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">(mhgradient, color1,
                  color2,...,color<i>n</i>)</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">this is equivalent to hrgadient,
                  but you can specify more than two colors</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">(mvgradient, color1,
                  color2,...,color<i>n</i>)</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">this is equivalent to vrgadient,
                  but you can specify more than two colors</font></td>
                </tr>
              </tbody>
            </table>
          </center><br />
          <br />
          <b>Examples:</b>

          <center>
            <table border="0" width="80%" cellspacing="0" cellpadding="5">
              <tbody>
                <tr>
                  <td align="center" valign="middle"><img src="guide/images/texsolid.gif" border="0" width="151" height="21" alt=
                  "[Solid Color]" /></td>

                  <th align="left" valign="top"><font face="Times New Roman, Times, Times Roman">(solid, gray)</font></th>
                </tr>

                <tr>
                  <td align="center" valign="middle"><img src="guide/images/texdgrad.gif" border="0" width="151" height="21" alt=
                  "[Diagoonal Gradient]" /></td>

                  <th align="left" valign="top"><font face="Times New Roman, Times, Times Roman">(dgradient, gray80,
                  gray20)</font></th>
                </tr>

                <tr>
                  <td align="center" valign="middle"><img src="guide/images/texhgrad.gif" border="0" width="151" height="21" alt=
                  "[Horizontal Gradient]" /></td>

                  <th align="left" valign="top"><font face="Times New Roman, Times, Times Roman">(hgradient, gray80,
                  gray20)</font></th>
                </tr>

                <tr>
                  <td align="center" valign="middle"><img src="guide/images/texvgrad.gif" border="0" width="151" height="21" alt=
                  "[Vertical Gradient]" /></td>

                  <th align="left" valign="top"><font face="Times New Roman, Times, Times Roman">(vgradient, gray80,
                  gray20)</font></th>
                </tr>
              </tbody>
            </table>
          </center><br />
          <br />
          <br />
          <br />

          <center>
            <table border="0" width="95%" cellspacing="0" cellpadding="5">
              <tbody>
                <tr>
                  <th align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Option</font></th>

                  <th align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Value</font></th>

                  <th align="leftt" valign="top"><font face="Times New Roman, Times, Times Roman">Description</font></th>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">*NewStyle</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean (default:
                  NO)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Selects between N*XTSTEP style
                  buttons in the titlebar and a newer style of buttons.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">WidgetColor</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">(solid, color) where color is a
                  color name (default: (solid, grey))</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Chooses the color to be used in
                  titlebar buttons if NewStyle=No;</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">WorkspaceBack</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">a texture or none (default:
                  none)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Default texture for the
                  workspace background. Note the <i>dgradient</i> and <i>mdgradient</i> textures can take a lot of time to be
                  rendered.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">IconBack</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">texture (default: (solid,
                  grey))</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Texture for the background of
                  icons and miniwindows.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">FTitleBack</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">texture (default: (solid,
                  black))</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Texture for the focused window
                  titlebar.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">PTitleBack</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">texture (default: (solid,
                  "#616161"))</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Texture for the titlebar of the
                  parent window of the currently focused transient window</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">UTitleBack</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">texture (default: (solid,
                  gray))</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Texture for unfocused window
                  titlebars.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">MenuTitleBack</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">texture (default: (solid,
                  black))</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Texture for menu
                  titlebars.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">MenuTextBack</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">texture (default: (solid,
                  gray))</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Texture for menu
                  items</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">FTitleColor</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">color (default:
                  white)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">The color of the text in the
                  focused window titlebar.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">PTitleColor</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">color (default:
                  white)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Color for the text in the
                  titlebar of the parent window of the currently focused transient.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">UTitleColor</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">color (default:
                  black)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">The color for the text in the
                  titlebar of unfocused windows.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">MenuTitleColor</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">color (default:
                  white)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Color for the text in menu
                  titlebars</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">MenuTextColor</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">color (default:
                  black)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Color for the text in menu
                  items</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">HighlightColor</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">color (default:
                  white)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Color for the highlighted item
                  in menus.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">HighlightTextColor</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">color (default:
                  black)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Color for the highlighted item
                  text in menus.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">MenuDisabledColor</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">color (default:
                  "#616161")</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Color for the text of disabled
                  menu items.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">ClipTitleColor</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">color (default:
                  black)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Color for the text in the
                  clip.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">CClipTitleColor</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">color (default:
                  "#454045")</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Color for the text in the
                  collapsed clip.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">WindowTitleFont</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">font (default: Helvetica bold
                  12)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Font for the text in window
                  titlebars.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">MenuTitleFont</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">font (default: Helvetica bold
                  12)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Font for the text in menu
                  titlebars)</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">MenuTextFont</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">font (default: Helvetica medium
                  12)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Font for the text in menu
                  items</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">IconTitleFont</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">font (default: Helvetica medium
                  8)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Font for the text in miniwindow
                  titlebars.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">ClipTitleFont</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">font (default: Helvetica bold
                  10)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Font for the text in the
                  clip.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">Displayfont</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">font (default: Helvetica medium
                  12)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Font for the text information in
                  windows, like the size of windows during resize.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">TitleJustify</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">center, left, or right (default:
                  center)</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Justification of the text in
                  window titlebars.</font></td>
                </tr>
              </tbody>
            </table>
          </center><br />
          <br />

          <h3><b>Keyboard Bindings</b></h3><br />
          <br />
          Keyboard shortcut specifications are in the form:<br />
          <br />
          <pre>
<font face="Courier New, Courier">
        [&lt;modifier key names&gt; + ] &lt;key name&gt;
</font>
</pre><br />
          <br />
          Where <i>modifier key names</i> specify an optional modifier key, like Meta or Shift. Any number of modifier keys might
          be specified. The <i>key name</i> is the actual key that will trigger the action bound to the option.<br />
          <br />
          Examples:<br />
          <br />

          <table border="0" width="80%" cellspacing="0" cellpadding="5">
            <tbody>
              <tr>
                <th align="right">[F10]</th>

                <td align="left">Means the F10 key.</td>
              </tr>

              <tr>
                <th align="right"><font face="Times New Roman, Times, Times Roman">Meta+TAB</font></th>

                <td align="left"><font face="Times New Roman, Times, Times Roman">Means the TAB key with the Meta modifier key
                pressed at the same time.</font></td>
              </tr>

              <tr>
                <th align="right"><font face="Times New Roman, Times, Times Roman">Meta+Shift+TAB</font></th>

                <td align="left"><font face="Times New Roman, Times, Times Roman">Means the TAB key with the Meta and Shift
                modifier keys pressed at the same time.</font></td>
              </tr>
            </tbody>
          </table><br />
          <br />
          Key names can be found at /usr/X11R6/include/X11/keysymdef.h The <b>XK_</b> prefixes must be ignored (if key name is
          <b>XK_</b>Return use Return).<br />
          <br />

          <center>
            <table border="0" width="95%" cellspacing="0" cellpadding="5">
              <tbody>
                <tr>
                  <th align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Option</font></th>

                  <th align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Default Value</font></th>

                  <th align="leftt" valign="top"><font face="Times New Roman, Times, Times Roman">Description</font></th>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">RootMenuKey</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Opens the <a href=
                  "chap3.php#3.1.1">root window menu</a> at the current
                  position of the mouse pointer.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">WindowListKey</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Opens the <a href=
                  "chap3.php#3.1.2">window list menu</a> menu at the current
                  position of the mouse pointer.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">WindowMenuKey</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Opens the <a href=
                  "chap2.php#2.2.9">window commands menu</a> for the currently
                  focused window.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">MiniaturizeKey</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Miniaturizes the currently
                  focused window.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">HideKey</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Hides the currently active
                  application.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">CloseKey</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Closes the current focused
                  window</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">MaximizeKey</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Maxmizes the currently focused
                  window.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">VMaximizeKey</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Vertically Maximizes the
                  currently focused window.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">RaiseKey</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Meta+Up</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Raises the currently focused
                  window.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">LowerKey</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Meta+Down</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Lowers the currently focused
                  window.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">RaiseLowerKey</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Raises the window under the
                  pointer, or lowers it if it is already raised.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">ShadeKey</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Shades the currently focused
                  window.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">SelectKey</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Selects current focused
                  window.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">FocusNextKey</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Switch focus to next
                  window.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">FocusPrevKey</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Switch focus to previous
                  window.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">NextWorkspaceKey</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Switches to next
                  workspace.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">PrevWorkspaceKey</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Switches to previous
                  workspace.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">NextWorkspaceLayerKey</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Switches to the next group of 10
                  workspaces.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">PrevWorkspaceLayerKey</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Switches to the previous group
                  of 10 workspaces.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">Workspace1Key</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Switches to workspace
                  1.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">Workspace2Key</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Switches to workspace 2,
                  creating it if it does not exist.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">Workspace3Key</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Switches to workspace 3,
                  creating it if it does not exist.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">Workspace4Key</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Switches to workspace 4,
                  creating it if it does not exist.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">Workspace5Key</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Switches to workspace 5,
                  creating it if it does not exist.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">Workspace6Key</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Switches to workspace 6,
                  creating it if it does not exist.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">Workspace7Key</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Switches to workspace 7,
                  creating it if it does not exist.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">Workspace8Key</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Switches to workspace 8,
                  creating it if it does not exist.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">Workspace9Key</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Switches to workspace 9,
                  creating it if it does not exist.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">Workspace10Key</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Switches to workspace 10,
                  creating it if it does not exist.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">ClipLowerKey</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Lowers the clip.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">ClipLowerKey</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Lowers the clip.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">ClipRaiseLowerKEy</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">None</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Raises the clip, or lowers it if
                  it is already raised.</font></td>
                </tr>
              </tbody>
            </table>
          </center><br />
          <br />
          <br />
          <br />
          <a name="4.1.4"></a>

          <h3><a name="4.1.4">4.1.4 Window Attributes</a></h3><a name="4.1.4"></a><br />
          <br />
          Window attributes are stored in the $(HOME)/GNUstep/Defaults/WMWindowAttributes file.<br />
          <br />
          The contents of this file is a dictionary of attribute dictionaries keyed by window names. Like this:<br />
          <br />
          <pre>
<font face="Courier New, Courier">
{
        "*" = {
                Icon = "defaultAppIcon.xpm";
        };
        "xterm.XTerm" = {
                Icon = "xterm.xpm";
        };
        xconsole = {
                Omnipresent = YES;
                NoTitlebar = YES;
                KeepOnTop = NO;
        };
}
</font>
</pre>Window names are in the form<sup>1</sup>:<br />
          <br />
          &lt;window instance name&gt;.&lt;window class name&gt;<br />
          <br />
          OR<br />
          <br />
          &lt;window instance name&gt;<br />
          <br />
          OR<br />
          <br />
          &lt;window class name&gt;<br />
          <br />
          <br />
          Placing an asterisk as the window name means that the values set for that key are to be used as default values for all
          windows. So, since xconsole does not specify an Icon attribute, it will use the default value, which in the above
          example is defaultAppIcon.xpm.<br />
          <br />
          <hr />
          <font size="-1"><sup>1</sup> You can get the values for these information by running the <font face=
          "Courier New, Courier">xprop</font> utility on the desired window. When you do that, it will show the following line,
          among other things:<br />
          <br />
          <font face="Courier New, Courier">WM_CLASS(STRING) = "xterm", "XTerm"</font><br />
          <br />
          The first string (xterm) is the window instance name and the second (XTerm) the window class name.}</font>
          <hr />

          <h3><b>Options:</b></h3><br />
          <br />
          The default is NO for all options<br />
          <br />
          <br />

          <center>
            <table border="0" width="95%" cellspacing="0" cellpadding="5">
              <tbody>
                <tr>
                  <th align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Option</font></th>

                  <th align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Value</font></th>

                  <th align="leftt" valign="top"><font face="Times New Roman, Times, Times Roman">Description</font></th>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">Icon</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">pixmap file name</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Assigns a pixmap image to be
                  used as the icon for that window.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">NoTitleBar</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Disables the titlebar in the
                  window.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">NoResizeBar</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Disables the resizebar in the
                  window.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">NoMiniaturizeButton</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Remove the miniaturize
                  button.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">NoCloseButton</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Remove the close
                  button.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">NoHideOthers</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Do not hide the window, or the
                  application to which the window belongs when a <i>Hide Others</i> command is issued.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">NoMouseBindings</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Do not grab mouse buttons in
                  that window. This means that actions like a Meta-click on the window will be caught by the application instead
                  of WindowMaker.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">NoKeyBindings</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Do not grab keys in that window.
                  This means that keystrokes that would normally be intercepted by WindowMaker (because they are bound to some
                  action), like Meta+Up, will be passed to the application.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">NoAppIcon</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Do not create application icon
                  for the window. This is useful for some applications that incorrectly get more than one application
                  icon.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">KeepOnTop</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Always keep the window over
                  other normal windows.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">Omnipresent</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Make the window be present in
                  all workspaces, AKA sticky window.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">SkipWindowList</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Do not list the window in the
                  <a href="chap3.php#3.12">window list menu</a>.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">KeepInsideScreen</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Always keep the window inside
                  the visible are of the screen.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">Unfocusable</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">boolean</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Do not let the window be
                  focused.</font></td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Courier New, Courier">StartWorkspace</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Workspace number or
                  name</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Make the window always be
                  initially shown in the indicated workspace.</font></td>
                </tr>
              </tbody>
            </table>
          </center><br />
          <br />
          <a name="4.1.5"></a>

          <h3><a name="4.1.5">4.1.5 Applications Menu</a></h3><a name="4.1.5"></a> The applications menu (AKA: Root Menu) can be
          defined in one of two distinct ways:<br />
          <br />

          <ul type="disk">
            <li>In the form of an array in property list format, in $(HOME)/GNUstep/Defaults/WMRootMenu</li>

            <li>In the form of a text file, whose location is present in $(HOME)/GNUstep/Defaults/WMRootMenu</li>
          </ul><br />
          <br />
          <br />
        </td>
      </tr>
    </table>

  <?php include("footer.php"); ?>

</body>
</html>
