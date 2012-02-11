<html xmlns="http://www.w3.org/1999/xhtml">
<head>
  <meta name="generator" content="HTML Tidy for Linux (vers 25 March 2009), see www.w3.org" />

  <title>Window Maker: Development</title>
  <meta http-equiv="Content-Type" content="text/html; charset=us-ascii" />
  <link href="title.css" rel="stylesheet" type="text/css" />
</head>

<body>
  <?php include("dock.php");  ?>
  <?php include("header.php"); ?>

  <div>
    <table class="inner" border="0" cellpadding="1" cellspacing="1">
      <tr>
        <td content="content" colspan="2" valign="top">
          <p>Here are some pieces of information regarding development in Window Maker.</p>

          <h3>Source code versioning system</h3>

          <p>The source code for Window Maker is contained in a <a href="http://git-scm.com/" target="_blank">git</a> repository located <a href=
          "http://repo.or.cz/w/wmaker-crm.git" target="_blank">here</a>. To obtain a full-fledged copy of the repository do this:<br /></p>

          <p>git clone git://repo.or.cz/wmaker-crm.git<br /></p>

	  <p> There are two main branches in the repository, called 'master' and 'next'. The purpose of the 'next' branch is to add
	  and extra layer of testing before the patches hit the 'master' branch. It is rebased when needed. The 'master' branch
	  should ideally never be rebased -- if it is, run to the nearest anti-nuclear bunker.</p>

          <h3>Submitting patches</h3>

          <p>The Window Maker source code follows the
          <a href="http://git.kernel.org/?p=linux/kernel/git/torvalds/linux.git;a=blob_plain;f=Documentation/CodingStyle;hb=HEAD" target="_blank">
          coding style of the linux kernel</a>. Respect it when submitting patches.</p>

          <p>If you are not familiar with git, take a look at the <a href="http://git-scm.com/" target="_blank">git homepage</a>
          -- it contains the kind of documentation you need to get started. You should also read the file contained
          in the Window Maker repository <a href=
          "http://repo.or.cz/w/wmaker-crm.git/blob/HEAD:/The-perfect-Window-Maker-patch.txt" target="_blank">The perfect Window Maker patch</a>
          which gives you further details about patches to Window Maker.</p>

          <p>If your patch is really good and important, feel free to send it to the mailing list at
          wmaker-dev@lists.windowmaker.org anyway in whatever form most suits you. But please consider the fact that patches sent
          in odd formats induce extra work for who applies them.</p>

          <p>Last but not least, patches doing code cleanups are <strong>STRONGLY</strong> encouraged.</p>

          <h3>Git repository for dockapps</h3>

          <p>There is also a <a href="http://repo.or.cz/w/dockapps.git" target="_blank">git repository</a> containing a few dockapps which
          apparently have no maintainers anymore. Patches for those dockapps (or to include more apps) can also be sent to
          wmaker-dev@lists.windowmaker.org.</p>

          <h3>Some sources of information</h3>

          <ul>
            <li>
              <p><a href="wings.php">The Window Maker WINGs library</a>.</p>
            </li>

            <li>
              <p><a href="http://tronche.com/gui/x/xlib/" target="_blank">The Xlib Manual</a></p>
            </li>
          </ul><br />
          <br />
          <br />
        </td>
      </tr>
    </table>

  <?php include("footer.php"); ?>

</body>
</html>
