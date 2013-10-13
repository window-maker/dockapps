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


       Here's a quick guide to tar'ing your theme up:<br>
       cd to your <tt>~/GNUstep/Library/WindowMaker/</tt> directory, and type something like
       the following: (replacing the file names appropriately for your theme)
       </font>
       <p>

       <ol>
         <li> <tt>tar -cvf Foo.tar Foo.lsm Backgrounds/FooBG.jpg Pixmaps/FooTile.xpm 
              Icons/FooIcon.xpm Themes/Foo</tt>
              <br><br>
	   <li> <tt>gzip -9 Foo.tar</tt>
              <br><br>
       </ol>

       This should leave you with a nice new <tt>Foo.tar.gz</tt> theme which includes (in this case),
       the theme file (<tt>Themes/Foo</tt>), the background wallpaper (<tt>Backgrounds/FooBG.jpg</tt>), 
       a tile (<tt>Pixmaps/FooTile.xpm</tt>), an icon for the dock (<tt>Icons/FooIcon.xpm</tt>) 
       and a readme file (<tt>Foo.lsm</tt>).<br>
       LSM files for your themes are a must. The format of the LSM file is
       easy. Click <a href="example-lsm.txt">here</a> for an example. 
       <p>
       If you're having trouble, try typing '<tt>man tar</tt>' or ask for help on irc 
       in the #WindowMaker channel on EFnet. If all else fails, you can 
       <a href="wmaker-dev.php">contact us</a> and we'll try and help you out,
       but please try the other methods first. Thanks.
       <p>
       Also, your theme file should NOT include your <tt>~/GNUstep/Defaults/WindowMaker</tt> 
       file! This will overwrite the person who downloads the theme's keybindings 
       and other personalized settings. 
       Also do NOT include any files other those from the directories I mentioned 
       above. For example, do NOT include your <tt>menu</tt> file or anything from your
       <tt>~/GNUstep/Defaults/</tt> directory. ONLY include files from the directories 
       listed above and the readme.lsm file.


 </td>
      </tr>
    </table>

  <?php include("footer.php"); ?>

</body>
</html>
