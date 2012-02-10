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

          <p>The source code for Window Maker is contained in a <a href="http://git-scm.com/">git</a> repository located <a href=
          "http://repo.or.cz/w/wmaker-crm.git">here</a>. Git is very convenient for our purposes and you can find more
          information about it on the mentioned link. To obtain a full-fledged copy of the repository do this:<br /></p>

          <p>git clone git://repo.or.cz/wmaker-crm.git<br /></p>

          <h3>Submitting patches</h3>

          <p>The source code in the above git repository follows the coding style of the linux kernel. Please take a look
          <a href="http://git.kernel.org/?p=linux/kernel/git/torvalds/linux.git;a=blob_plain;f=Documentation/CodingStyle;hb=HEAD">
          here</a> and try to respect it when submitting patches. This is really important.</p>

          <p>If you want to contribute patches to Window Maker and you are not familiar with git, please take a look at the git
          homepage above - it contains the kind of documentation you need to get started. You should also read the file contained
          in the Window Maker repository <a href=
          "http://repo.or.cz/w/wmaker-crm.git/blob/HEAD:/The-perfect-Window-Maker-patch.txt">The perfect Window Maker patch</a>
          which gives you further details about patches to Window Maker.</p>

          <p>If your patch is really good and important, feel free to send it to the mailing list
          wmaker-dev@lists.windowmaker.org anyway in whatever form most suits you. But please consider the fact that patches sent
          in odd formats induce extra work for who applies them.</p>

          <p>Last but not least, patches doing code cleanups are <strong>STRONGLY</strong> encouraged. Please go ahead! The risks
          of introducing regressions when doing cleanups exists, but if you also follow the practice of having a patch for each
          unrelated change you do, the eventuall regressions can be easily found and reverted.</p>

          <h3>Git repository for dockapps</h3>

          <p>There is also a <a href="http://repo.or.cz/w/dockapps.git">git repository</a> containing a few dockapps which
          apparently have no maintainers anymore. Patches for those dockapps (or to include more apps) can also be sent to
          wmaker-dev@lists.windowmaker.org.</p>

          <h3>Some sources of information</h3>

          <ul>
            <li>
              <p><a href="wings.php">The Window Maker WINGs library</a>.</p>
            </li>

            <li>
              <p><a href="http://tronche.com/gui/x/xlib/">The Xlib Manual</a></p>
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
