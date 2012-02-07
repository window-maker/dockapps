<html xmlns="http://www.w3.org/1999/xhtml">
<head>
  <meta name="generator" content="HTML Tidy for Linux (vers 25 March 2009), see www.w3.org" />

  <title>User Guide: The Workspace</title>
  <meta http-equiv="Content-Type" content="text/html; charset=us-ascii" />
  <link href="title.css" rel="stylesheet" type="text/css" />
</head>

<body>
  <?php
              include("header.php");
              include("dock.php");
      ?>

  <div id="inhalt">
    <table width="880" border="0" cellpadding="1" cellspacing="1">
      <tr>
        <td content="content" colspan="2" valign="top">
          <br />
          <br />
          <br />

          <h2>Chapter 3</h2>

          <h1>The Workspace</h1><br />
          <br />
          <br />
          <a name="3.1"></a>

          <h2><a name="3.1">3.1 Working with Menus</a></h2><a name="3.1"></a> Menus provide a list of commands that you can
          execute.<br />
          <br />

          <center>
            <img src="guide/images/menu.gif" border="0" width="480" height="300" alt="[An Example Menu]" />
          </center><br />
          <br />
          To execute a command listed in a menu, click in the corresponding item. The item will blink telling that the command is
          going to be executed.<br />
          <br />
          Grayed commands are disabled and cannot be executed at that moment. If you click on them nothing will happen.<br />
          <br />
          Some menu entries have a little triangular indicator at the right. Selecting these entries will open a submenu, with a
          new list of commands.<br />
          <br />
          You can use the keyboard to traverse and execute commands in some of the menus. First you must hit the key used to open
          the menu --- like F12 for the root menu --- to enable keyboard traversal of it. Then you can use the Up and Down arrow
          keys to change the current selected item and the Left and Right arrow keys to jump between submenus and parent menus.
          To execute the current selected item press Return. To close the menu or stop menu traversal, press Escape.
          Additionally, pressing the first letter for an menu item, will jump the current selection to that item.<br />
          <br />
          You can make frequently used menus ``stick'' to the workspace by dragging the titlebar of the menu. This will make a
          close button appear in the menu titlebar. If you want to close the menu, just click in that button.<br />
          <br />
          Menus are normally placed on top of other windows and cannot be obscured by them. If you want the menus to be able to
          be obscured by lowering them, double click the menu titlebar while holding the Meta key. Repeat this to make the menus
          not obscurable again.<br />
          <br />
          <br />
          <a name="3.1.1"></a>

          <h3><a name="3.1.1">3.1.1 The Root Window Menu</a></h3><a name="3.1.1"></a> The <i>Root Window Menu</i> or
          <i>Applications Menu</i> has items that allow you to quickly launch applications and do some workspace
          management.<br />
          <br />
          To open this menu, click on the workspace (root window) with the 3rd mouse button or hit the key bound to it (F12 by
          default).<br />
          <br />
          The contents of the applications menu can be configured to hold the applications installed on your system. To learn how
          to configure it, read the section on application menu configuration.<br />
          <br />
          <br />
          <a name="3.1.2"></a>

          <h3><a name="3.1.2">3.1.2 The Window List Menu</a></h3><a name="3.1.2"></a> Clicking in the workspace with the middle
          mouse button will open a menu listing all windows that currently exist, with the workspace in which the window is
          located to its right. The current focused window is marked by a diamond sign next to its name. Clicking in an entry in
          this menu will focus the window, raise it, and change to the workspace where it is located.<br />
          <br />
          <br />
          <br />
          <a name="3.2"></a>

          <h2><a name="3.2">3.2 Working with Applications</a></h2><a name="3.2"></a> In WindowMaker the instance of a running
          application is represented by an application icon. Do not confuse it with the icons (miniwindows in WindowMaker)
          displayed by other window managers when a window is iconified. Application icons and miniwindows can be differentiated
          in that miniwindows have titlebars, application icons do not.<br />
          <br />
          WindowMaker identifies a group of windows as belonging to a single instance of an application through some standard
          hints that the application sets in its windows. Unfortunately, not all applications that exist set these hints,
          preventing some application-specific features from working. These hints are <b>WM.CLASS</b>,<b>WM.COMMAND</b>, and
          <b>WM.CLIENT.LEADER</b> or the group leader in <b>WM.HINTS</b>.<br />
          <br />
          <br />
          <font size="-1">Note: The information about applications contained in this section only applies to versions of
          WindowMaker built without the --enable-single-icon compile time option. This option is unsupported and behaviour when
          it's enabled will not be covered in this text.</font><br />
          <br />
          <br />
          <a name="3.2.1"></a>

          <h3><a name="3.2.1">3.2.1 Hiding an Application</a></h3><a name="3.2.1"></a> If you want to close and application but
          intend to use it later you can <i>hide</i> it. When you hide an application all windows and miniwindows that belong to
          that application will be removed from the screen and hidden into its application icon.<br />
          <br />
          <br />
          <font face="Helvetica"><b>To hide an application</b></font>

          <ul type="disk">
            <li><font face="Helvetica">Click the miniaturize button of any of the windows that belong to the application while
            holding the Control key.<br />
            <br />
            OR<br />
            <br /></font></li>

            <li><font face="Helvetica">Press the keyboard shortcut assigned to it, which is Meta+h in the default
            configuration.<br />
            <br />
            OR<br />
            <br /></font></li>

            <li><font face="Helvetica">User the hide command in the <a href=
            "chap2.php#2.2.9">window commands menu</a> brought up when the
            window titlebar is clicked with the right mouse button.<br />
            <br />
            OR<br />
            <br /></font></li>

            <li><font face="Helvetica">Use the (Un)Hide command in the application icon commands menu brought up when the
            application icon is clicked with the right mouse button.</font></li>
          </ul><br />
          <br />
          <br />
          <font face="Helvetica"><b>To unhide an application</b></font>

          <ul type="disk">
            <li><font face="Helvetica">Double click the application icon with the left mouse button.<br />
            <br />
            OR<br />
            <br /></font></li>

            <li><font face="Helvetica">Use the (Un)Hide command in the application icon commands menu brought up when the
            application icon is clicked with the right mouse button.</font></li>
          </ul><br />
          <br />
          When you unhide an application, all it's windows and miniwindows will brought back, and you will be taken to the last
          workspace in which you worked with that application.<br />
          <br />
          <br />

          <h3><b>Extra Bindings</b></h3>

          <center>
            <table border="0" width="80%" cellspacing="0" cellpadding="0">
              <tbody>
                <tr>
                  <th align="left"><font face="Times New Roman, Times, Times Roman">Action</font></th>

                  <th align="left"><font face="Times New Roman, Times, Times Roman">Effect</font></th>
                </tr>

                <tr>
                  <td colspan="2">
                    <hr />
                  </td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Double-click the application
                  icon while holding the Meta key</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Unhide the clicked application,
                  and hide all other applications that are present in the current workspace.</font></td>
                </tr>

                <tr>
                  <td colspan="2">
                    <hr />
                  </td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Double-click the application
                  icon while holding the Shift key</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Unhide the clicked application
                  in the current workspace</font></td>
                </tr>

                <tr>
                  <td colspan="2">
                    <hr />
                  </td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Double-click the application
                  icon with the middle mouse button</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Unhide the clicked application
                  and deminiaturize all its windows.</font></td>
                </tr>

                <tr>
                  <td colspan="2">
                    <hr />
                  </td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Double-click the window titlebar
                  with the right mouse button while holding the Meta key.</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Hide all applications in the
                  current workspace except for the clicked one</font></td>
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
          There are two other commands in the applications menu related to application hiding:<br />
          <br />

          <center>
            <table border="0" width="90%" cellspacing="0" cellpadding="5">
              <tbody>
                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">Hide others</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Hide all applications in the
                  current workspace, except for the currently active one.</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">Show All</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Unhide all applications that
                  were hidden from the current workspace</font></td>
                </tr>
              </tbody>
            </table>
          </center><br />
          <br />
          <br />
          <br />
          <a name="3.2.2"></a>

          <h3><a name="3.2.2">3.2.2 The Application Icon Menu</a></h3><a name="3.2.2"></a> A menu with commands that will apply
          to the application can be brought up by clicking the application icon with the right mouse button.<br />
          <br />
          The commands available in this menu are:<br />
          <br />

          <center>
            <table border="0" width="90%" cellspacing="0" cellpadding="5">
              <tbody>
                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">Unhide Here</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Unhides the application in the
                  current workspace.</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">(Un)Hide</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Hides the application. Unless
                  the application is already hidden, in which case it will unhide the application and take you to its
                  workspace.</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">Set Icon...</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Opens the icon image selection
                  panel for the application icon.</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">Kill</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Will kill the
                  application.</font></td>
                </tr>
              </tbody>
            </table>
          </center><a name="3.2.3"></a>

          <h3><a name="3.2.3">3.2.3 The Application Dock</a></h3><a name="3.2.3"></a> The application dock is a place where you
          can store frequently used applications for easy and fast access. It is located, by default, on the right side of the
          screen.<br />
          <br />
          You can click the top icon (the one with the GNUstep logo) and drag it downward to remove most of the dock from view.
          You can also drag it sideways to move the entire dock from side of the screen to the other.<br />
          <br />
          A menu similar to the <a href="chap3.php#3.2.2">application icon
          menu</a> is brought up when you click a docked icon with the right mouse button.<br />
          <br />
          To make the dock <i>float</i> over windows (not be coverable by windows), either double-click the top dock icon while
          holding the Meta key, or select the "Floating Dock" option in the dock menu.<br />
          <br />
          <br />
          <b>Starting a docked application</b><br />
          <br />
          To start an application that is docked, double-click its icon. The icon will be briefly highlighted and the application
          will start.<br />
          <br />
          While an application is not running an ellipsis is present in the lower left-hand corner of the icon. This ellipsis
          will disappear when the application is started and reappear when the application is exited.<br />
          <br />
          While the application is running the docked icon will behave just like a normal, undocked application icon, except for
          some extra actions specific to the dock.<br />
          <br />
          <br />
          <font face="Helvetica"><b>To start a docked application:</b></font>

          <ul type="disk">
            <li><font face="Helvetica">Double-click the application icon with the left mouse button.<br />
            <br />
            OR<br />
            <br /></font></li>

            <li><font face="Helvetica">Use the "Launch" command in the dock menu for the icon. If the application is already
            running it will start another instance.<br />
            <br />
            OR<br />
            <br /></font></li>

            <li><font face="Helvetica">Hold the Control key while double-clicking the icon to start another instance of the
            application.</font></li>
          </ul><br />
          <br />
          If a new instance of an already running application is started it will get a new application icon.<br />
          <br />
          <br />
          <b>Customizing the dock</b><br />
          <br />
          To add new applications to the dock, you can click an application icon and drag it onto the dock. When a ghost image of
          the icon appears you can release the mouse button and the icon will be docked.<br />
          <br />
          To reorder the docked applications, drag an icon to an empty slot and move the icons around as you want.<br />
          <br />
          To remove a docked application, drag it from the dock and release the mouse button when the ghost image disappears. To
          remove the icon of an application that is running, hold the Meta key while dragging it.<br />
          <br />
          <br />
          <b>Configuring the docked application</b><br />
          <br />
          To change the settings of a docked application, select the "Settings..." item in the dock menu for that icon. A
          settings panel for that icon will appear.<br />
          <br />

          <center>
            <img src="guide/imagdockapppanel.gif" border="0" width="297" height="369" alt="[Docked Application Settings Panel]" />
          </center><br />
          <br />
          In the <i>Application path and arguments</i> field, the path for the application and its arguments can be changed. Note
          that you can't change the application that is represented in the icon or change anything that would cause the
          application name to be changed. For example, if the icon is for <b>xterm</b> you can't change the field's value to
          <b>ghostview</b>; or if the icon is for <b>xterm -name vi</b>, you can't change it to <b>xterm -name pine</b>. Also
          note that you cannot use shell commands, such as out put redirectors. (&gt;, &gt;&gt; etc.)<br />
          <br />
          <br />
          <a name="3.3"></a>

          <h2><a name="3.3">3.3 Working with Workspaces</a></h2><a name="3.3"></a><br />
          <a name="3.3.1"></a>

          <h3><a name="3.3.1">3.3.1 The Workspaces Menu</a></h3><a name="3.3.1"></a> The <i>Workspaces Menu</i> allows you to
          create, switch, destroy and rename workspaces.<br />
          <br />
          It has the following items:<br />
          <br />

          <center>
            <table border="0" width="90%" cellspacing="0" cellpadding="5">
              <tbody>
                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">[New]</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Creates a new workspace and
                  automatically switches to it</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">[Destroy Last]</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Destroys the last workspace
                  unless it is occupied</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">[Workspaces]</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Each workspace has a
                  corresponding item in the Workspaces menu. Clicking in one of these entries will switch from the current
                  workspace to the selected workspace.</font></td>
                </tr>
              </tbody>
            </table>
          </center>The current active workspace is indicated by a small indicator at the left of the workspace item.<br />
          <br />

          <center>
            <img src="guide/images/wsmenu.gif" border="0" width="350" height="200" alt="[Workspace Menu]" />
          </center><br />
          <br />
          To change the name of a workspace you must first ``stick'' the menu. Then Control click in the item corresponding to
          the workspace you want to rename. The item will turn into a editable text field where you can edit the workspace name.
          To finish editing the workspace name, press Return; to cancel it, press Escape.<br />
          <br />
          There is a limit of 16 characters on the length of the workspace name.<br />
          <br />
          <br />
          An example Workspace menu being edited:

          <center>
            <img src="guide/images/wsmenued.gif" border="0" width="101" height="103" alt=
            "[Workspace Menu: Editing a Workspace name]" />
          </center><br />
          <br />
          <a name="3.3.2"></a>

          <h3><a name="3.3.2">3.3.2 The workspace clip</a></h3><a name="3.3.2"></a> [This section was unavailable in the
          original, and thus is not here]<br />
          <br />
          <br />
          <br />
          <br />
          <br />
        </td>
      </tr>
    </table>
  </div>
</body>
</html>
