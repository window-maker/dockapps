<html xmlns="http://www.w3.org/1999/xhtml">
<head>
  <meta name="generator" content="HTML Tidy for Linux (vers 25 March 2009), see www.w3.org" />

  <title>Window Maker: News</title>
  <meta http-equiv="Content-Type" content="text/html; charset=us-ascii" />
  <link href="../title.css" rel="stylesheet" type="text/css" />
</head>

<body>
  <?php
              include("../header.php");
              include("../dock.php");
      ?>

  <div id="inhalt">
    <table width="880" border="0" cellpadding="1" cellspacing="1">
      <tr>
        <td content="content" colspan="2" valign="top">
          <h3>Introduction</h3>

          <p>This manual describes the usage and configuration of the WindowMaker window manager. It is intended for both users
          who never used the X Window System and for users who have experience with other window managers.</p>

          <p>How to Read this guide If you never have used a X window manager, you should read all of this guide, as it contains
          detailed instructions for new users.</p>

          <p>Text in sans serif font, indicate instructions you must follow to accomplish a given task. If you're out of time (or
          patience), you should at least read text in these parts.</p>

          <p>You can ignore the text in Extra Bindings boxes while you're getting familiar with WindowMaker. Once you've got
          familiar with it, you can read the text in these boxes to learn more ways to accomplish tasks.</p>

          <h1>1.1 What is a window manager?</h1>

          <p>If you come from the Windows or MacOS world, you might be confused about all these things like window managers, X
          windows etc.</p>

          <p>In the Unix world, the task of providing a graphical user interface (GUI) is normally divided by 3 different
          components:</p>

          <p>the window server; the window manager and the user interface toolkit. The window server is standard and is usually
          the X Window System or some vendor provided compatible version of it. The X Window System, or X for short, is a window
          server. It's function is to provide a portable and high-level access to devices like keyboard, mouse and video display.
          It allows applications to show graphical information on the display through rectangular areas called windows.</p>

          <p>Most user interface objects, like buttons, menus and scrollers are made of windows. The top level windows displayed
          by applications are named windows as well. These objects are not provided by the window server. These must be made by
          the application program or by the user interface toolkit.</p>

          <p>For more information, read the manual page for X(1) and the documentation for Xlib.</p>

          <p>The primary function of the window manager is to control the layout of top level windows on screen. WindowMaker is a
          window manager. It provides a titlebar and a resizebar to change window layout, application menus to launch
          applications and execute special commands, application icons, miniwindows and an application dock. They will be
          explained in more detail in the following chapters.</p>

          <p>The user interface toolkit is a library or collection of libraries that provide an API for application developers to
          program the interfaces for their applications. Toolkits generally provide controls like buttons, menus, radio-buttons
          etc to be used for program interaction. There are currently many of these toolkits available for X. Motif
          &#226;&#8222;&#162;, OpenLook &#226;&#8222;&#162;, and Athena are examples of toolkits.</p>

          <p>All other features normally found in other operating systems, like file managers, are implemented as separate
          programs and are not directly related to the window manager.</p>
        </td>
      </tr>
    </table>
  </div>
</body>
</html>
