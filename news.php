<html xmlns="http://www.w3.org/1999/xhtml">
<head>
  <meta name="generator" content="HTML Tidy for Linux (vers 25 March 2009), see www.w3.org" />

  <title>Window Maker: News</title>
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

          <h3>Version 0.95.1 released</h3>

          <p>Window Maker 0.95.1 was released on January 29th 2012.</p>

          <p>The last official Window Maker release was version 0.92.0 from 2006, and version 0.95.1 contains many bug fixes and
          also a few new features. Window Maker 0.92.0 was already very stable, so most of the bug fixes are related to odd
          situations. One random example is the fix which avoids a segfault when creating more than 81
          workspaces, as reported on youtube <a href="http://www.youtube.com/watch?v=fkNJZvKwmhE">here</a>.</p>

          <p>Another bug-fix of peculiar nature is this <a href=
          "http://repo.or.cz/w/wmaker-crm.git/commit/c91bb1ba1360006c568db37438779e525868cf17">one</a>, which happened once every
          49 days -- more precisely 2^32 milliseconds.</p>

          <p>A lot of effort was put into cleaning up the code, with lots of code removal and tidying things up. The following
          output should give you an idea of the development in the last cycle:</p>
          <pre>
[mafra@Pilar:wmaker.git]$ git diff --shortstat wmaker-0.92.0+..wmaker-0.95.1
 592 files changed, 118361 insertions(+), 133342 deletions(-)
[mafra@Pilar:wmaker.git]$ git diff --shortstat 688a56e8ab67b..wmaker-0.95.1
 566 files changed, 37676 insertions(+), 41817 deletions(-)
</pre>The first shortstat is really everything, including the (huge) patch generated in this <a href=
"http://repo.or.cz/w/wmaker-crm.git/commit/688a56e8ab67b56550e2874d9d7423f0d435bfd9">commit</a> from 2009, which changed the old
sources to the linux kernel coding style. The second shortstat contains the summary of development afterwards -- but included
is the addition of a debian folder with files summing around ~20k lines. The full diffstat for the second command can be seen
<a href="fulldiffstat.php">here</a>.

          <h2>New features and highlights</h2>

          <p>This list is really very incomplete, but should give a first-order approximation to what are the new features in
          this release. For a bit more of details see the NEWS file included in the sources, although it also misses a lot of
          things. For the truly curious among you, reading through <code>git log</code> is the complete source of
          information.</p>

          <ul>
            <li>
              <p>Left/right window maximization, making them occupy the left/right area of the screen</p>
            </li>

            <li>
              <p>Maximus (a.k.a tiled) maximization. Maximizes windows such that it occupies the largest area without overlapping
              other windows</p>
            </li>

            <li>
              <p>New mouse-resizing functionality. Windows can now be resized vertically (horizontally) using MOD+Wheel
              (CTRL+Wheel)</p>
            </li>

            <li>
              <p>History and TAB completion in the run dialog</p>
            </li>

	<li>
		<p> Preliminary XRandR support (needs a bit more work to be bug-free; not compiled in by default.
		Use --enable-xrandr if you want to test it).</p>
	</li>

            <li>
              <p>A WPrefs option to make icons bounce when their respective applications want attention</p>
            </li>

            <li>
              <p>New applications (wmgenmenu and wmmenugen) to generate the root menu automatically by looking which applications
              you have on your $PATH</p>
            </li>

            <li>
              <p>Automatic detection of configuration changes. Linux users whose kernel supports the <a href=
              "http://en.wikipedia.org/wiki/Inotify">inotify</a> mechanism have their configuration changes
              detected automatically without polling, reducing the number of CPU wakeups.</p>
            </li>

            <li>
              <p>Dockapps can now receive special treatment by setting their WM_CLASS to "DockApp"</p>
            </li>

            <li>
              <p>And many trivial things which reduce little annoyances one might have. For example, an option was added to
              control whether or not Window Maker should do automatic workspace switching to satisfy a focus request from a
              window located in another workspace.</p>
            </li>

	<li>
		<p> (For developers). The addition of a debian/ folder which allows the creation of a debian package for wmaker using the
		git sources.</p>
          </ul>

          <p><br />
	<div align="center">
          <img src="v0_95_1.png" alt="Info v0.95.1" width="382" height="257" /></p>
	</div>
        </td>
      </tr>
    </table>
  </div>
</body>
</html>
