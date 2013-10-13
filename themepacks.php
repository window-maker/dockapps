<html xmlns="http://www.w3.org/1999/xhtml">
<head>
  <meta name="generator" content="HTML Tidy for Linux (vers 25 March 2009), see www.w3.org" />

  <title>Window Maker: Themes</title>
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

  
<h1>ThemePacks HOWTO</h1>

<h2>Themes (Theme Packs) For Window Maker</h2>
   
<b>Note:</b> the information contained in this file is only valid for themes
in the <tt>.themed</tt> (for theme directory) format, supported in 
Window Maker 0.50.0 or newer. See <a href="theme-HOWTO.php">here</a> for 
information on themes for WindowMaker-0.20.3 and earlier.
<br><br><br>


<h2>How To Install a Theme Pack</h2>

To install a theme, unpack your theme into your <tt>WindowMaker</tt> directory 
(the same as old-style themes), usually <tt>~/GNUstep/Library/WindowMaker</tt>
<br><br>
<tt>cd ~/GNUstep/Library/WindowMaker</tt><br>
<tt>gzip -dc "xyztheme.tar.gz" | tar xvf -</tt>
<br><br>
You can also do this in your system-wide <tt>WindowMaker</tt> directory (usually
<tt>/usr/local/share/WindowMaker</tt>) to have the themes be available to all 
your users. This will probably need to be done with root access.
<br><br><br>


<h2>How To Load a Theme</h2>
 After installing a theme, it will automatically show up in your menu under <b>Appearance -> Themes -> ThemeName</b>.
(unless of course you have manually changed your menu to remove this) If you have your Themes menu already opened and
pinned to your desktop, you may need to close it and reopen it to have it show the new theme.
<br><br>
To manually load the new theme from the command line, use the <tt>setstyle</tt> command. Example:
<br><br>
<tt>setstyle xyztheme.themed</tt>
<br><br>
Note that if you move the directory of the theme (for example, from
<tt>~/GNUstep/Library/WindowMaker/Themes</tt> to <tt>/usr/local/share/WindowMaker/Themes</tt>)
you will have to reload that theme so that path information is updated.
<br><br><br>


<h2>How To Make a Theme Pack</h2>
To create a theme pack from your current configuration, use the <tt>getstyle</tt>
utility with the <tt>-p</tt> flag. Example:
<br><br>
<tt>getstyle -p ~/GNUstep/Library/WindowMaker/Themes/MyTheme</tt>
<br><br>
This will create a theme pack (a new directory in either the current
directory or a directory you specify) named <tt>MyTheme.themed</tt>, containing
everything it requires, including all pixmap files. In this example,
the new theme pack would be made in your <tt>themes</tt> directory and be 
immediately available in your "Themes" menu.
<br><br>
Additionally, you can put a text file named <tt>MyTheme.lsm</tt> in the <tt>MyTheme.themed</tt> 
directory. This file can contain info like copyrights, credits or whatever.
<br><br>
To distribute your theme, just make a <tt>.tar.gz</tt> of the <tt>.themed</tt> directory.
This is preferably done from the same directory that you unpack the themes
from to maintain consistancy with the old theme format.
<br><br>
Example:
<br><br>
<tt>cd ~/GNUstep/Library/WindowMaker</tt><br>
<tt>tar cvf MyTheme.tar Themes/MyTheme.themed</tt><br>
<tt>gzip MyTheme.tar</tt>
<br><br><br>


<h2>How To Delete a Theme Pack</h2>
Just remove the <tt>.themed</tt> directory. Example:
<br><br>
<tt>cd ~/GNUstep/Library/WindowMaker/Themes</tt><br>
<tt>rm -fr themename.themed</tt>
<br><br><br>


<h2>How To Save Disk Space</h2>
If you have more than 1 theme that use the same huge background image,
you can delete all the duplicated files and then create hard links in
place of them. For example, if you have:
<br><br>
<tt>theme1.themed/back.jpg</tt>
<br><br>
<tt>theme2.themed/backimage.jpg</tt>
<br><br>
<tt>theme3.themed/back.jpg</tt>
<br><br>
and all three files contain the same image, you can do:
<br><br>
<tt>rm theme2.themed/backimage.jpg</tt><br>
<tt>rm theme3.themed/back.jpg</tt><br>
<tt>ln theme1.themed/back.jpg theme2.themed/backimage.jpg</tt><br>
<tt>ln theme1.themed/back.jpg theme3.themed/back.jpg</tt>
<p/>
<p/>
&nbsp;

      </td>
      </tr>
    </table>

  <?php include("footer.php"); ?>

</body>
</html>
