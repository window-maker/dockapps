<html xmlns="http://www.w3.org/1999/xhtml">
<head>
  <meta name="generator" content="HTML Tidy for Linux (vers 25 March 2009), see www.w3.org" />

  <title>User Guide: Window</title>
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

          <h2>Chapter 2</h2>

          <h1>Windows</h1><br />
          <br />
          <br />

          <h2><a name="2.1">2.1 Anatomy of a Window</a></h2>Generally an application will have the following layout:<br />
          <br />

          <center>
            <img src="guide/images/anatomy.gif" border="0" width="426" height="344" alt="[Anatomy of a Window]" /><br />
            <br />

            <table border="0" width="90%" cellspacing="0" cellpadding="5">
              <tbody>
                <tr>
                  <td align="right" valign="top"><br />
                  <font face="Times New Roman, Times, Times Roman"><b>Titlebar</b></font></td>

                  <td align="left"><br />
                  <font face="Times New Roman, Times, Times Roman">The titlebar presents the name of the application, document or
                  window. It's color indicates the keyboard focus state and type of the window. You can use it to move, activate,
                  raise, lower and access the window commands menu.</font></td>
                </tr>

                <tr>
                  <td align="right" valign="top"><font face="Times New Roman, Times, Times Roman"><b>Miniaturize
                  button.</b></font></td>

                  <td align="left"><font face="Times New Roman, Times, Times Roman">You can click on the miniaturize button to
                  miniaturize/iconify a window or click it with the <b>Meta</b> key pressed to hide the application.</font></td>
                </tr>

                <tr>
                  <td align="right" valign="top"><br />
                  <font face="Times New Roman, Times, Times Roman"><b>Close Button.</b></font></td>

                  <td align="left"><br />
                  <font face="Times New Roman, Times, Times Roman">The close button can be used to close a window or kill the
                  application, if the application can't understand the close message.</font></td>
                </tr>

                <tr>
                  <td align="right" valign="top"><br />
                  <font face="Times New Roman, Times, Times Roman"><b>Resizebar.</b></font></td>

                  <td align="left"><br />
                  <font face="Times New Roman, Times, Times Roman">You use the resizebar to (surprise!) resize a
                  window.</font></td>
                </tr>

                <tr>
                  <td align="right" valign="top"><br />
                  <font face="Times New Roman, Times, Times Roman"><b>Client Area.</b></font></td>

                  <td align="left"><br />
                  <font face="Times New Roman, Times, Times Roman">The client area is where the application show it's
                  information. If the window if inactive, you can click on it to activate it.</font></td>
                </tr>
              </tbody>
            </table>
          </center><br />
          <br />
          <br />
          <a name="2.2"></a>

          <h2><a name="2.2">2.2 Working With Windows</a></h2><a name="2.2"></a><br />
          <br />
          <a name="2.2.1"></a>

          <h3><a name="2.2.1">2.2.1 Focusing a Window</a></h3><a name="2.2.1"></a><br />
          Windows can be in two states: <i>focused</i> , or <i>unfocused.</i> The focused window (also called the key or active
          window) has a black titlebar and is the window that receives keyboard input, ie: where you can type text. Usually it's
          the window where you work on. Only one window may be focused at a time. Unfocused windows have a light gray titlebar.
          Some applications have a special type of window, called dialog windows transient windows or panels. When these windows
          are active, the window that owns them (the main window) get a dark gray titlebar. As soon as the dialog window is
          closed, the focus is returned to the owner window.<br />
          <br />
          <br />
          The image below shows an active Open File panel and it's owner window.<br />
          <br />

          <center>
            <img src="guide/images/focus.gif" border="0" width="574" height="360" alt="[Focused, Unfocused, and Parent Window]" />
          </center><br />
          <br />
          There are three styles of window focusing:<br />
          <br />
          <b>Click-to-Focus</b>,or manual focus mode. In click-to-focus mode, you explicitly choose the window that should be
          focused. This is the default mode.<br />
          <br />
          <b>Focus-Follow-Mouse</b>,or auto-focus mode. In this mode, the focused window is chosen based on the position of the
          mouse pointer. The window below the mouse pointer is always the focused window.<br />
          <br />
          <b>Sloppy-Focus</b>,or semi-auto-focus mode. This is similar to the focus-follow-mouse mode, but if you move the
          pointer from a window to the root window, the window will not loose focus.<br />
          <br />
          You can choose between these modes with the <i>FocusMode</i> option<br />
          <br />
          <br />
          <br />
          <font face="Helvetica"><b>To focus a window in click-to-focus mode:</b></font>

          <ul type="disc">
            <li><font face="Helvetica">Click on the titlebar, resizebar or in the client area of the window with the left or
            right mouse button.<br />
            <br />
            OR<br />
            <br /></font></li>

            <li><font face="Helvetica">Click on the titlebar with the middle mouse button. This will focus the window without
            bringing it to the front.<br />
            <br />
            OR<br />
            <br /></font></li>

            <li><font face="Helvetica">Open the window list menu and select the window to focus.</font></li>
          </ul><br />
          When you click in the client area of an inactive window to set the focus, the click is normally processed by the
          application. If you find this behaviour a little confusing, you can make the application ignore this click by using the
          <i>IgnoreFocusClick</i> option.<br />
          <br />
          <br />
          <font face="Helvetica"><b>To focus a window in focus-follow-mouse mode:</b></font>

          <ul type="disc">
            <li><font face="Helvetica">Move the pointer over the window you want to focus.</font></li>
          </ul><br />
          <br />
          <br />
          <br />
          <a name="2.2.2"></a>

          <h3><a name="2.2.2">2.2.2 Reordering Overlapping Windows</a></h3><a name="2.2.2"></a> Windows can overlap other
          windows, making some windows be over or in front of others.<br />
          <br />
          <br />
          <font face="Helvetica"><b>To bring a window to the front:</b><br /></font>

          <ul type="disc">
            <li><font face="Helvetica">Click on the titlebar or resizebar of the desired window with the left mouse button.<br />
            <br />
            OR<br />
            <br /></font></li>

            <li><font face="Helvetica">Select the desired window from the Window List menu.</font></li>
          </ul><br />
          <br />
          Dialog/transient windows are always placed over their owner windows, unless the <i>OnTopTransients</i> option is
          disabled. Some windows have a special attribute that allow them be permanently over normal windows. You can make
          specific windows have this attribute use the <i>AlwaysOnTop</i> window option or set it in the Window Inspector
          panel.<br />
          <br />
          <br />

          <h3><b>Extra Bindings</b></h3>

          <center>
            <table border="0" width="80%" cellspacing="0" cellpadding="5">
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
                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Meta-Click on the window
                  titlebar. with the left mouse button</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Sends the window to the
                  back.</font></td>
                </tr>

                <tr>
                  <td colspan="2">
                    <hr />
                  </td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Meta-Click on the Client Area of
                  the window with the left mouse button.</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Brings the window to the front
                  and focuses it.</font></td>
                </tr>

                <tr>
                  <td colspan="2">
                    <hr />
                  </td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Hold the Meta key and press the
                  Up Arrow key</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Brings the current focused
                  window to the front.</font></td>
                </tr>

                <tr>
                  <td colspan="2">
                    <hr />
                  </td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Hold the Meta key and press the
                  Down Arrow key</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Sends the current focused window
                  to the back.</font></td>
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
          <a name="2.2.3"></a>

          <h3><a name="2.2.3">2.2.3 Moving a Window</a></h3><a name="2.2.3"></a> To move the window around the screen, drag the
          window through it's titlebar with the left mouse button pressed. This will also bring the window to the front and focus
          the window.<br />
          <br />
          <br />
          <font face="Helvetica"><b>To move a window:</b></font>

          <ul type="disc">
            <li><font face="Helvetica">Click on the titlebar of the window you want to move with the left mouse button and drag
            it with the button pressed.</font></li>
          </ul>While you move the window, a little box will appear in the screen, indicating the current window position in
          pixels, relative to the top left corner of the screen. You can change the location of this position box by hitting the
          Shift key during the move operation.<br />
          <br />
          In some rare occasions, it is possible for a window to be placed off screen. This can happen with some buggy
          applications. To bring a window back to the visible screen area, select the window in the Window List menu. You can
          prevent windows from doing that with the <i>DontMoveOff</i> window attribute.<br />
          <br />
          <br />

          <h3><b>Extra Bindings</b></h3>

          <center>
            <table border="0" width="80%" cellspacing="0" cellpadding="5">
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
                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Drag the titlebar with the
                  middle mouse button</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Move the window without changing
                  it's stacking order.</font></td>
                </tr>

                <tr>
                  <td colspan="2">
                    <hr />
                  </td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Drag the titlebar while holding
                  the Control key</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Move the window without focusing
                  it.</font></td>
                </tr>

                <tr>
                  <td colspan="2">
                    <hr />
                  </td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Drag the client area or
                  resizebar while holding the Meta key</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Move the window.</font></td>
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
          <br />
          <a name="2.2.4"></a>

          <h3><a name="2.2.4">2.2.4 Resizing a Window</a></h3><a name="2.2.4"></a> The size of a window can be adjusted by
          dragging the resizebar.<br />
          <br />

          <center>
            <img src="guide/images/resizebar.gif" border="0" width="470" height="47" alt="[A Resizebar]" />
          </center><br />
          <br />
          Depending on the place you click to drag the resizebar, the resize operation is constrained to a direction.<br />
          <br />
          <br />
          <font face="Helvetica"><b>To resize a window</b></font>

          <ul type="disc">
            <li><font face="Helvetica">To change the window's height, click in the middle region of the resizebar and drag it
            vertically.<br />
            <br /></font></li>

            <li><font face="Helvetica">To change the window's width, click in either end regions of the resizebar and drag it
            horizontally.<br />
            <br /></font></li>

            <li><font face="Helvetica">To change both height and width at the same time, click in either end regions of the
            resizebar and drag it diagonally.</font></li>
          </ul><br />
          <br />

          <h3><b>Extra Bindings</b></h3>

          <center>
            <table border="0" width="80%" cellspacing="0" cellpadding="5">
              <tbody>
                <tr>
                  <th align="left"><font face="Times New Roman, Times, Times Roman"><b>Action</b></font></th>

                  <th align="left"><font face="Times New Roman, Times, Times Roman"><b>Effect</b></font></th>
                </tr>

                <tr>
                  <td colspan="2">
                    <hr />
                  </td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Drag the window in the client
                  area with the Right mouse button, while holding the Meta key</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Resizes the window.</font></td>
                </tr>

                <tr>
                  <td colspan="2">
                    <hr />
                  </td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Drag the resizebar with the
                  middle mouse button</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Resize the window without
                  bringing it to the front</font></td>
                </tr>

                <tr>
                  <td colspan="2">
                    <hr />
                  </td>
                </tr>

                <tr>
                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Drag the resizebar while holding
                  the Control key</font></td>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Resize the window without
                  focusing it.</font></td>
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
          <a name="2.2.5"></a>

          <h3><a name="2.2.5">2.2.5 Miniaturizing a Window</a></h3><a name="2.2.5"></a><br />
          If you want to temporarily get rid of a window, you can miniaturize it. When miniaturizing a window, it will shrink
          into a miniwindow with a icon and a title that is placed at the bottom of the screen.<br />
          <br />

          <center>
            <img src="guide/imagtitle.gif" border="0" width="200" height="50" alt="[A Titlebar]" />
          </center><br />
          <br />

          <center>
            <table border="0" cellspacing="0" cellpadding="5" width="40%">
              <tbody>
                <tr>
                  <td align="left"><img src="guide/images/mini.gif" border="0" width="64" height="64" alt="[A Mini-window]" /></td>

                  <td align="left"><font face="Times New Roman, Times, Times Roman">A mini-window</font></td>
                </tr>
              </tbody>
            </table>
          </center><br />
          <br />
          You can move the miniwindow around the screen by dragging it. Unlike application icons, miniwindows cannot be
          docked.<br />
          <br />
          To restore a window from it's miniwindow, double click the miniwindow. The window will be restored in the current
          workspace, with the same position, size and contents as it had before miniaturization.<br />
          <br />
          <br />
          <font face="Helvetica"><b>To miniaturize a window:</b></font>

          <ul type="disc">
            <li><font face="Helvetica">Click on the miniaturize button.<br />
            <br />
            OR<br />
            <br /></font></li>

            <li><font face="Helvetica">Use the keyboard shortcut assigned to this action, Meta+m in the default
            configuration.</font></li>
          </ul><br />
          <br />
          You can also restore all miniaturized and hidden windows of a given application by double clicking in it's application
          icon with the middle mouse button.<br />
          <br />
          <br />
          <a name="2.2.6"></a>

          <h3><a name="2.2.6">2.2.6 Shading a Window</a></h3><a name="2.2.6"></a> If you want to temporarily get rid of a window,
          an option for it's miniaturization is to <i>shade</i> it. When you shade a window, the window rolls up to it's
          titlebar. You can do almost everything you do with a normal window with shaded windows, like miniaturizing or closing
          it.<br />
          <br />
          <br />
          <br />
          <br />
          <font face="Helvetica"><b>To shade a window:</b></font>

          <ul type="disc">
            <li><font face="Helvetica">Double Click on the titlebar of the window.</font></li>
          </ul>

          <center>
            <img src="guide/images/shade.gif" border="0" width="287" height="279" alt="[A Shaded window]" />
          </center><br />
          <br />
          <br />
          <a name="2.2.7"></a>

          <h3><a name="2.2.7">2.2.7 Closing a Window</a></h3><a name="2.2.7"></a> After finishing work in a window, you can close
          it to completely get rid of it. When you close a window, it is removed from the screen and can no longer be restored.
          So, before closing a window, be sure you have saved any work you were doing on it.<br />
          <br />

          <center>
            <img src="guide/imagtitle2.gif" border="0" width="200" height="50" alt="A Titlebar with a close button" />
          </center><br />
          <br />
          Some windows will have a close button with some dots around it. These windows can't be closed normally and the only way
          to get rid of them is by exiting the application. You should try exiting from inside the application (through it's
          menus or buttons) when possible. Otherwise you can force WindowMaker to ``kill'' the application.<br />
          <br />
          <br />
          <font face="Helvetica"><b>To force the closure of a window (by killing the application):</b></font>

          <ul type="disc">
            <li><font face="Helvetica">Hold the Control key and click on the close button.<br />
            <br />
            OR<br />
            <br /></font></li>

            <li><font face="Helvetica">Double click the close button.</font></li>
          </ul><br />
          <br />
          It is also possible to kill applications that can be normally closed by clicking the close button while holding the
          Control key.<br />
          <br />
          <br />
          <a name="2.2.8"></a>

          <h3><a name="2.2.8">2.2.8 Maximizing a Window</a></h3><a name="2.2.8"></a> If you want to resize a window to occupy the
          whole screen, you can maximize the window. When you unmaximize it, the window will be restored to the same position and
          size it was before maximized.<br />
          <br />
          <br />
          <font face="Helvetica"><b>To maximize a window:</b></font>

          <ul type="disc">
            <li><font face="Helvetica">Hold the Control key and double click on the window titlebar to resize the window's height
            to full screen.<br />
            <br />
            OR<br />
            <br /></font></li>

            <li><font face="Helvetica">Hold the Shift key and double click on the window titlebar to resize the window's width to
            full screen.<br />
            <br />
            OR<br />
            <br /></font></li>

            <li><font face="Helvetica">Hold both the Control and Shift keys and double click on the window titlebar to resize
            both window's height and width to full screen.</font></li>
          </ul><br />
          <br />
          <br />
          <font face="Helvetica"><b>To restore the size of a maximized window:</b></font>

          <ul type="disc">
            <li><font face="Helvetica">Hold the Control OR Shift key and double click on the window titlebar.</font></li>
          </ul><br />
          <br />
          You can select whether the window should be maximized to the whole screen or if the position of the Dock should be
          accounted for by setting the <i>WinDock</i> option.<br />
          <br />
          <br />
          <br />
          <a name="2.2.9"></a>

          <h3><a name="2.2.9">2.2.9 The Window Commands Menu</a></h3><a name="2.2.9"></a> Clicking on the titlebar of a window
          with the right mouse button will open a menu containing commands that will apply to that window. The menu can also be
          opened through the keyboard with the Control+Escape key, by default.<br />
          <br />

          <center>
            <table border="0" width="90%" cellspacing="0" cellpadding="5">
              <tbody>
                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">(Un)Maximize</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Will either maximize the window
                  horizontally and vertically, or, if the window is a;ready maximized, restore the window to the size it was
                  prior to being maximized.</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">Miniaturize</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Will miniaturize the
                  window.</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">(Un)Shade</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Will shade the window, or
                  unshade it if it is already shaded.</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">Hide</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Will hide all the windows of the
                  application</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">Hide Others</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Will hide all current
                  applications except the current one</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">Move To</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Allows you to move the window to
                  a different workspace</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">Attributes...</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Opens the Window Attributes
                  Inspector (see section <a href="chap2.php#2.3">2.3</a>
                  )</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">Close</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Will close the
                  window</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">Kill</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Will kill the application. Use
                  this option only if the application does not provide means to close it normally, or in extreme
                  cases.</font></td>
                </tr>
              </tbody>
            </table>
          </center><a name="2.3"></a>

          <h2><a name="2.3">2.3 The Window Attributes Inspector</a></h2><a name="2.3"></a> <a name="2.3.1"></a>

          <h3><a name="2.3.1">2.3.1 Window Specification</a></h3><a name="2.3.1"></a> This panel Allows you to specify the
          WM_CLASS that WindowMaker should use to identify the window whose attributes you are setting.<br />
          <br />

          <center>
            <img src="guide/images/wiaspec.gif" border="0" width="262" height="374" alt=
            "[Window Attributes Inspector: Window Specification]" />
          </center><br />
          <br />
          <br />
          <a name="2.3.2"></a>

          <h3><a name="2.3.2">2.3.2 Window Attributes</a></h3><a name="2.3.2"></a> This panel lets you set the attributes for the
          selected window.<br />
          <br />

          <center>
            <img src="guide/images/wiaattrib.gif" border="0" width="262" height="374" alt=
            "[Window Attributes Inspector: Window Attributes]" />
          </center><br />
          <br />

          <center>
            <table border="0" width="90%" cellspacing="0" cellpadding="5">
              <tbody>
                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">Disable titlebar</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Causes the titlebar for the
                  selected window not to be displayed</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">Disable resizebar</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Causes the resizebar for the
                  selected window not to be displayed</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">Disable close
                  button</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Causes the close button for the
                  selected window not to be displayed</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">Disable miniaturize
                  button</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Causes the miniaturize button
                  for the selected window not to be displayed</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">Keep on Top</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Causes the selected window to
                  stay on top of all other windows</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">Omnipresent</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Causes the selected window to be
                  displayed in all workspaces</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">Start miniaturized</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Causes the selected window to
                  start miniaturized</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">Skip window list</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Causes the select window to be
                  skipped when cycling through the window list.</font></td>
                </tr>
              </tbody>
            </table>
          </center><br />
          <br />
          <br />
          <a name="2.3.3"></a>

          <h3><a name="2.3.3">2.3.3 Advanced Options</a></h3><a name="2.3.3"></a><br />
          <br />

          <center>
            <img src="guide/images/wiaadvanced.gif" border="0" width="262" height="374" alt=
            "[Window Attributes Inspector: Advanced Options]" />
          </center><br />
          <br />

          <center>
            <table border="0" width="90%" cellspacing="0" cellpadding="5">
              <tbody>
                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">Ignore HideOthers</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Causes the selected window to
                  remain visible when <b>HideOthers</b> is selected from the <a href=
                  "chap2.php#2.2.9">Window Commands Menu</a></font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">Don't bind keyboard
                  shortcuts</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Causes the selected window to
                  receive ALL keyboard events</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">Don't bind mouse
                  clicks</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Causes the selected window to
                  receive all mouse-click events</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">Keep Inside Screen</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Causes the selected window not
                  to be able to place itself off the screen</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">Don't let it take
                  focus</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Causes the selected window not
                  to be able to take input focus</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">Don't Save Session</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Causes the state of the selected
                  window not to be saved when a session is saved. (either when quitting WindowMaker, or when done
                  manually.)</font></td>
                </tr>

                <tr>
                  <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">Emulate Application
                  Icon</font></th>

                  <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Emulates an Application Icon for
                  "broken" applications</font></td>
                </tr>
              </tbody>
            </table>
          </center><br />
          <br />
          <br />
          <a name="2.3.4"></a>

          <h3><a name="2.3.4">2.3.4 Icon and Initial Workspace</a></h3><a name="2.3.4"></a> This panel allows you to
          <b>browse</b> for, and <b>update</b> the <b>mini-window image</b> for the selected window, as well as setting the
          <b>initial workspace.</b><br />
          <br />

          <center>
            <img src="guide/images/wiaiandiw.gif" border="0" width="262" height="374" alt=
            "[Window Attributes Inspector: Icon and Initia Workspace]" />
          </center><br />
          <br />
          <br />
          <br />
          <br />
          <a name="2.3.5"></a>

          <h3><a name="2.3.5">2.3.5 Application Specific</a></h3><a name="2.3.5"></a> Attributes specific to the selected
          application<br />
          <br />

          <center>
            <img src="guide/images/wiaappspec.gif" border="0" width="262" height="374" alt=
            "[Window Attributes Inspector: Icon and Initia Workspace]" />
          </center><br />
          <br />

          <table border="0" width="90%" cellspacing="0" cellpadding="5">
            <tbody>
              <tr>
                <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">Start hidden</font></th>

                <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Starts the selected application in
                a hidden state</font></td>
              </tr>

              <tr>
                <th align="right" valign="top"><font face="Times New Roman, Times, Times Roman">No application icon</font></th>

                <td align="left" valign="top"><font face="Times New Roman, Times, Times Roman">Disables the application icon for
                the selected application</font></td>
              </tr>
            </tbody>
          </table><br />
          <br />
          <br />
        </td>
      </tr>
    </table>
  </div>
</body>
</html>
