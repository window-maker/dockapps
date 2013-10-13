<html xmlns="http://www.w3.org/1999/xhtml">
<head>
  <meta name="generator" content="HTML Tidy for Linux (vers 25 March 2009), see www.w3.org" />

  <title>Window Maker: Home</title>
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
          <br />
          <br />

<?
   $chapter = $_GET["chapter"];
   if ($chapter == "") {
?>

          <h1>FAQ</h1>

          <p>Have questions about Window Maker? If so, look no further. Below is our collection of Frequently Asked Questions and
          their corresponding answers. Many of these have been adapted from the <a href=
          "http://web.archive.org/web/20030401182339/http://www.dpo.uab.edu/~grapeape/wmfaq.html">original FAQ</a> by Chris
          Green. Questions are routinely taken and added in from the mailing lists and IRC forums.</p>

          <ol>
            <li><a href="FAQ.php?chapter=1">Introduction to Window Maker</a></li>

            <li><a href="FAQ.php?chapter=2">Installing Window Maker</a></li>

            <li><a href="FAQ.php?chapter=3">Configuring Window Maker</a></li>

            <li><a href="FAQ.php?chapter=4">Using Window Maker</a></li>

            <li><a href="FAQ.php?chapter=5">Application Compatibility</a></li>

            <li><a href="FAQ.php?chapter=6">Themes and Dockapps</a></li>

            <li><a href="FAQ.php?chapter=7">Miscellaneous Questions</a></li>

            <li><a href="FAQ.php?chapter=8">Troubleshooting Tips</a></li>

            <li><a href="FAQ.php?chapter=9">Programming for Window Maker</a></li>
          </ol>
<?
}
if ($chapter == 1) {
?>
<h1>Chapter 1: Introduction to Window Maker</h1>
<UL type="disc">
<B>1.1</B> &nbsp; <A href="#30">What is Window Maker?</A><BR>
<B>1.2</B> &nbsp; <A href="#31">Where can I get Window Maker?</A><BR>
<B>1.3</B> &nbsp; <A href="#32">Where are the mailing lists and archives?</A><BR>
<B>1.4</B> &nbsp; <A href="#33">Where can I find more documentation?</A><BR>
<B>1.5</B> &nbsp; <A href="#34">What is an appicon?</A><BR>
<B>1.6</B> &nbsp; <A href="#35">How can I get a question added to the FAQ?</A><BR>
<B>1.7</B> &nbsp; <A href="#36">How do I report bugs?</A><BR>
<B>1.8</B> &nbsp; <A href="#37">Is there an anomymous cvs server?</A><BR>
<B>1.9</B> &nbsp; <A href="#38">Where can I find the Window Maker IRC channel?</A><BR>
<B>1.10</B> &nbsp; <A href="#39">What is the difference between appicons, mini-windows, and minimized applications?</A><BR>
<B>1.11</B> &nbsp; <A href="#126">How do I make sense of Window Maker's version number scheme?</A><BR>
</UL>
<BR><HR><BR>
<A name="30"></A>
<h2>1.1 &nbsp;What is Window Maker?</h2>
Window Maker is an X11 window manager 
originally designed to provide integration 
support for the GNUstep Desktop Environment. 
In every way possible, it reproduces the 
elegant look and feel of the NEXTSTEP[tm] 
user interface. It is fast, feature rich, 
easy to configure, and easy to use. It is 
also free software and part of the GNU 
Project, with contributions being made by 
programmers from around the world
<A name="31"></A>
<h2>1.2 &nbsp;Where can I get Window Maker?</h2>
Window Maker can be obtained from the 
official website, http://windowmaker.org/,
or from various mirror sites listed at
http://windowmaker.org/mirrors.html
<A name="32"></A>
<h2>1.3 &nbsp;Where are the mailing lists and archives?</h2>
All information regarding the Window Maker 
mailing lists can be found at http://windowmaker.org/lists.html
<A name="33"></A>
<h2>1.4 &nbsp;Where can I find more documentation?</h2>
Additional documentation can be found in
the Window Maker source distribution, or 
at http://windowmaker.org/documentation.html
<A name="34"></A>
<h2>1.5 &nbsp;What is an appicon?</h2>
An appicon is the icon produced by an 
application that initially is in the 
bottom left corner of the screen while 
an application is running. For an example, 
run xterm and notice the icon in the 
corner (make sure that you use xterm and 
not a default rxvt when testing, because 
many versions of rxvt do not properly set 
their window attributes).<BR><BR>

For a more indepth discussion of how an 
appicon relates to Window Maker, see 
question 1.10
<A name="35"></A>
<h2>1.6 &nbsp;How can I get a question added to the FAQ?</h2>
For now, the best method is to E-mail your
question to faq@windowmaker.org.  We are
working on a web-based submission form
to our FAQ system, which will enable 
users to submit questions for review.
<A name="36"></A>
<h2>1.7 &nbsp;How do I report bugs?</h2>
You can look at the BUGFORM file in 
the source distribution of Window Maker. 
Alternatively, you can use the 
Window Maker Bug Tracker at 
http://windowmaker.org/cgi-bin/bugs
<A name="37"></A>
<h2>1.8 &nbsp;Is there an anomymous cvs server?</h2>
Yes there is.  To check out from cvs, first
<P>
<PRE>
	export CVSROOT=":pserver:anoncvs@cvs.windowmaker.org:/cvsroot"
        cvs login
</PRE>
There is no password, so simply hit enter when prompted.
<P>
Then issue the following command ("wm" is the name of the module):
<P>
<PRE>
	cvs -z3 checkout -d WindowMaker wm
</PRE>
To update your source tree, cd to the WindowMaker directory and type
<P>
<PRE>
	cvs -z3 update -dP
</PRE>
inside the WindowMaker directory.
<P>

For more detailed CVS instructions, please
visit http://windowmaker.org/development-cvs.html
<A name="38"></A>
<h2>1.9 &nbsp;Where can I find the Window Maker IRC channel?</h2>
The official Window Maker IRC channel can be accessed by connecting to irc.windowmaker.org on port 6667,
and joining #WindowMaker

<A name="39"></A>
<h2>1.10 &nbsp;What is the difference between appicons, mini-windows, and minimized applications?</h2>
Thanks to Jim Knoble for this answer:
<P>
Many window managers are capable of turning large windows into
smaller `icons' which represent the window yet don't take as much
screen real estate.  We're all familiar with that model.
<P>
Window Maker has two kinds of these icons.  One kind is created when
an application---technically, a window group---is started.  It
represents the entire application and is called an `appicon'.  Such
icons are square tiles containing only the picture which represents
the application; they have no titles.
<P>
The second kind of icon in Window Maker is created when a particular
window (possibly one belonging to an application displaying more than
one window) is `miniaturized' (which is the same action as
`minimizing' or `iconifying' in other window management models) using
the miniaturization button on the window's titlebar.  These
miniaturized windows are called `miniwindows' and can normally be
distinguished from appicons by their small titlebar at the top of the
tile.
<A name="126"></A>
<h2>1.11 &nbsp;How do I make sense of Window Maker's version number scheme?</h2>
The numbering scheme is relatively simple,
and is in the format of three numbers separated
by dots.  The first number is the "major" revision 
number.  The second is the "minor" revision number.
And finally, the third is the "patch level" number.
<P>
To put this all into perspective, let's examine the
version number "0.65.1".  This number signifies
that there has not been a major revision release,
that its minor revision is newer than the previous
one (0.64.x), and that it's on the first patch level
after the 0.65.0 release.  This still might be
confusing, so go away with this in mind: numbers
ending in .0 tend to be new feature releases
but less stable than .1, .2, .3 patch level releases, 
the latter of which are used to fix bugs.
<P>
It is generally safe to go with the highest numbered
patch release.

<?
}
if ($chapter==2) {
?>

<h1>Chapter 2: Installing Window Maker</h1>
<UL type="disc">
<B>2.1</B> &nbsp; <A href="#41">Why are no icons showing up after installing Window Maker?</A><BR>
<B>2.2</B> &nbsp; <A href="#42">How do I make Window Maker link against libtiff?</A><BR>
<B>2.3</B> &nbsp; <A href="#44">How do I switch CDE's window manager to use WindowMaker?</A><BR>
<B>2.4</B> &nbsp; <A href="#46">Do I need to rerun wmaker.inst with every new version of Window Maker?</A><BR>
<B>2.5</B> &nbsp; <A href="#47">Why am I only getting a root menu with xterm and exit items?</A><BR>
<B>2.6</B> &nbsp; <A href="#48">How do I get Window Maker to use more than 16 colors on my SGI Indy Workstation?</A><BR>
<B>2.7</B> &nbsp; <A href="#49">Using WindowMaker with Solaris 2.6 CDE</A><BR>
<B>2.8</B> &nbsp; <A href="#51">How do I install Window Maker on a Solaris box?</A><BR>
<B>2.9</B> &nbsp; <A href="#52">How do I fix an error such as libwraster.so.1: cannot open shared object file?</A><BR>
<B>2.10</B> &nbsp; <A href="#53">How do I fix an error dealing with aclocal: configure.in: 15: macro 'AM_PROG_LIBTOOL' not found in library?</A><BR>
<B>2.11</B> &nbsp; <A href="#55">When I run wmaker, it quits complaining about '__register_frame_info'</A><BR>
<B>2.12</B> &nbsp; <A href="#56">How do I make libjpeg link against Window Maker?</A><BR>
<B>2.13</B> &nbsp; <A href="#57">How do I start Window Maker after running wmaker.inst?</A><BR>
<B>2.14</B> &nbsp; <A href="#58">How do I make libpng link against Window Maker?</A><BR>
<B>2.15</B> &nbsp; <A href="#59">How do I make giflib or libungif to link against Window Maker?</A><BR>
<B>2.16</B> &nbsp; <A href="#60">How do I fix an error similar to "wrlib: could not allocate shared memory segment: invalid argument"</A><BR>
<B>2.17</B> &nbsp; <A href="#61">How do I add Window Maker to the Solaris dtlogin screen?</A><BR>
<B>2.18</B> &nbsp; <A href="#127">What happened to libPropList?</A><BR>
</UL>
</FONT>
<BR><HR><BR>
<A name="41"></A>
<h2>2.1 &nbsp;Why are no icons showing up after installing Window Maker?</h2>
As of WindowMaker version 0.15.0, the default setup includes .tiff icons which
require you to have compiled Window Maker with libtiff support. For assistance
on compiling libtiff, see the following question.
<A name="42"></A>
<h2>2.2 &nbsp;How do I make Window Maker link against libtiff?</h2>
Many UNIX operating systems have 
difficulty finding third party libraries
by default.  Unfortunately, there are too
many of these to include instructions for
them all.
<P>
In general, you will want to ensure the
latest version of libtiff is installed (see
ftp://www.libtiff.org).  Typically on non-Linux
systems, libtiff will be located under 
/usr/local, with includes and libs in those
respective sub-directories.
<P>
Often, it will be necessary to add /usr/local/lib
to the system's LD_LIBRARY_PATH environment
variable (especially so on Solaris, but see 'man ld'
for details on your platform).  Furthermore,
it is possible to supply special flags to the
configure script to help it find where the libraries
are.  An example is given below:<BR>
<P>
<PRE>
./configure --with-libs-from="-L/usr/local/lib" \
	    --with-incs-from="-I/usr/local/include"
</PRE>

<P>
Also, you will want to make sure you're using GNU make 
(gmake) for the Window Maker compile.
<A name="44"></A>
<h2>2.3 &nbsp;How do I switch CDE's window manager to use WindowMaker?</h2>
Method 1:
<P>
Peter Ilberg gives us this answer:
<P>
Install WM wherever you want it, mine is in /opt/WindowMaker-0.16.0 (eg. use
./configure --prefix=/opt/WindowMaker-0.16.0). Run the install script wmaker.inst in
your home directory.
<P>
Add the following two lines to .dtprofile in your home directory:
<P>
<PRE>
	SESSIONTYPE=xdm; export SESSIONTYPE
	PATH=:/usr/contrib/bin/X11:$PATH:.; export PATH
</PRE>
This tells CDE to go looking for an .xinitrc/.xsession instead of using the default
environment.
<P>
Make your .xsession/.xinitrc executable (VERY IMPORTANT, wmaker.inst did NOT
do this automatically for me) using eg.
<P>
<PRE>
	chmod ugo+x .xsession
</PRE>
Your .xsession/.xinitrc should look something like this:
<P>
<PRE>
	#!/bin/sh
</PRE>
<PRE>
	&lt;some other init stuff that you want/need&gt;
</PRE>
<PRE>
	exec wmaker
</PRE>
Things to try if it doesn't work: (somewhat fuzzy and random) 
<P>
This should do it although I did have problems sometimes initially which I fixed by
randomly trying absolute pathes for wmaker in .xsession/.xinitrc and/or making the
dtprofile/.xinitrc/etc executable. It helps logging in on the console (select from CDE
login screen) and start X manually using "X". If it works that way it should work when
logging into the CDE environment. Remember to Check your paths! 
<P>
If it doesn't work, you can also substitute some other window manager for wmaker in
the .xinitrc and see if that works. If it does you know at least that .xinitrc is getting
called/executed, so your WM path is wrong or not set. 
<P>
Method 2:
<P>
Thomas Hanselman gave this alternative answer (via Peter Ilberg):
<P>
Build and install WM wherever you want, as described in Method 1. You can install
and run WM just fine from your home directory. That's what I'm doing, since I don't
have root access at work :(. Then, in your Xdefaults file in your home directory, add
the following line:
<P>
<PRE>
	Dtsession*wmStartupCommand: &lt;path to WindowMaker executable&gt;
</PRE>
Then, log out, and log back in, and, unless I've forgotten a step (or this is a custom
Nortel thing), you should be in Window Maker heaven ;).
<P>
Difference between the methods: (according to Thomas) 
<P>
I've been told that the difference between setting the resource and Peter's method is
that if you override the window manager with the resouce, you still get the CDE
resources read into the resource database (so you still have your color settings &amp;
such from CDE), whereas with Peter's, the CDE resource don't get read into the
database. I don't know if this is true or not, however. Also, another thing to note with
Window Maker and HP-UX 10.20 -- if you select "Exit Session" from the WM root
menu, WindowMaker and all of your applications are killed, but you may not be
logged out. Again, this might be an artifact from my work environment, or the way I
start Window Maker.
<P>
Owen Stenseth adds: 
<P>
When using this method it is possible to exit Window Maker cleanly by using the
dtaction command. I use the following in my Window Maker menu:
<P>
<PRE>
            "Exit Session"      EXEC dtaction ExitSession
</PRE>
<P>
The only problem I have at the moment is I seem to get multiple copies of asclock
running when I log in again.
<A name="46"></A>
<h2>2.4 &nbsp;Do I need to rerun wmaker.inst with every new version of Window Maker?</h2>
Dan Pascu reveals the answer:
<P>
If this is necessary, it will be listed in the NEWS
file included in the source distribution.
Since 0.15.x, the domain files have been changed
in such a way that re-running wmaker.inst is
redundant. The user config files are by default
merged in with the global ones normally located
in /usr/local/share/WindowMaker/Defaults.
So, even if new options are added, they should
be automatically added to the environment.
<A name="47"></A>
<h2>2.5 &nbsp;Why am I only getting a root menu with xterm and exit items?</h2>
Most likely, the problem is that Window Maker can not find a copy of the C pre
processor in a directory such as /lib. The file /lib/cpp should be a symbolic link to
whatever C compiler's cpp you are using. For example:
<P>
<PRE>
	cpp -&gt; /usr/bin/cpp-2.95*
</PRE>
Another possibility is your /usr/X11/lib/X11/xinit/xinitrc is a 
broken symlink. Either create a new symlink, or do something like:
<P>
<PRE>
	$ cp /usr/X11/lib/X11/xinit/xinitrc.fvwm2 \
		/usr/X11/lib/X11/xinit/xinitrc.wmaker
</PRE>
<PRE>
	$ ln -sf /usr/X11/lib/X11/xinit/xinitrc.wmaker \
		/usr/X11/lib/X11/xinit/xinitrc
</PRE>
then just edit /usr/X11/lib/X11/xinit/xinitrc and replace the exec of 'fvwm2'
by '/usr/local/bin/wmaker' (should be somewhere towards the end of the file,
most probably the very last line).
<P>
Thanks to Tomas Szepe for the second part.
<A name="48"></A>
<h2>2.6 &nbsp;How do I get Window Maker to use more than 16 colors on my SGI Indy Workstation?</h2>
Thanks to Peter H. Choufor this answer:
<P>
By default, the SGI X Server uses 8-bit Pseudocolor mode.
To change it, edit (as root) the file /usr/lib/X11/xdm/Xservers.
Change it to read:
<P>
<PRE>
    :0 secure /usr/bin/X11/X -bs -c -class TrueColor -depth 24
</PRE>
<A name="49"></A>
<h2>2.7 &nbsp;Using WindowMaker with Solaris 2.6 CDE</h2>
Thanks to Rob Funk for this answer:
<P>
Assuming you installed Window Maker according to the README's that come
with the source, all you need to run Window Maker on a Solaris box is
an entry in the .xinitrc.  This should work for any version.  When
you run wmaker.inst the first time, allow it to make changes
to the .xinitrc file.  Mine looks like this:
<P>
<PRE>
 #!/bin/sh
 # Window Maker Default .xinitrc
 exec /usr/local/bin/wmaker
</PRE>
Believe it or not, that's all that it takes.  This, in fact, runs Window Maker
instead of OpenWindows.  In order to choose Window Maker, you simply choose
"OpenWindows Desktop" in the "Options - Session" Menus  And Choose "CDE
Desktop" if you want CDE.
<P>
The color schemes and settings for Window Maker are seperate from CDE.
I tested on a SPARC 10, but I assume Solaris x86 would work also.
<P>
(webmaster note: It works fine on Solaris x86)
<A name="51"></A>
<h2>2.8 &nbsp;How do I install Window Maker on a Solaris box?</h2>
Here are some hints from John Kemp:
<P>
Installing Window Maker on a Solaris 2.6 box might require
one or two little hints.  Here you are (this was on a system
running xdm by the way, but similar suggestions apply otherwise):
<P>
1) /usr/openwin/lib/X11/xdm/Xservers like this:
<P>
:0 local /usr/openwin/bin/X -dev /dev/fb defdepth 24 defclass TrueColor
<P>
2) Turn off shm in the WindowMaker configure:
<P>
./configure --disable-shm
<P>
3) might have to modify your LD_LIBRARY_PATH:, or make "wmaker"
<PRE>
   a script that does it for you (mv wmaker wmaker.exe):
</PRE>
LD_LIBRARY_PATH=/usr/local/lib:/usr/local/X11/lib:/usr/lib:/usr/openwin/lib
export LD_LIBRARY_PATH
/usr/local/bin/wmaker.exe $*
<P>
The real key is the "--disable-shm".
<P>
(webmaster note: Window Maker should work fine with
 SHM enabled, at least it does under Solaris 8.  Try
 the default first, and then use this if you run into
 problems with it)
<A name="52"></A>
<h2>2.9 &nbsp;How do I fix an error such as libwraster.so.1: cannot open shared object file?</h2>
If you have an error when running Window Maker such as
<P>
<PRE>
	libwraster.so.1: cannot open shared object file
</PRE>
These are the instructions for Linux.
<P>
First, make sure that /usr/local/lib ( or whatever directory you installed  Window Maker to)
is listed in your /etc/ld.so.conf ). You need to run ldconfig so the new shared libraries will be loaded.
After running ldconfig as root, the linker should properly load the libraries.
You need to run this every time you update Window Maker.
<P>
Thanks to Joseph Czapiga, the BSD procedure for adding shared
library directories is
<P>
<PRE>
	ldconfig -m /usr/local/lib  (m means merge)
</PRE>
<A name="53"></A>
<h2>2.10 &nbsp;How do I fix an error dealing with aclocal: configure.in: 15: macro 'AM_PROG_LIBTOOL' not found in library?</h2>
You need to install libtool.
It also must be a libtool different from version 1.2b ( shipped with redhat 5.2 ).
You can get libtool from ftp.gnu.org/pub/gnu
Make sure the autoconf and automake versions you have installed are at
least:
autoconf 2.12
automake 1.3
libtool 1.2
<P>
<PRE>
 From Blaine Horrocks:
</PRE>
"You can also work around this problem on RedHat5.2 by copying the
distributed aclocal.m4 to acinclude.m4 before running configure for the
first time.   Configure works fine and doing the make succeeds."
<A name="55"></A>
<h2>2.11 &nbsp;When I run wmaker, it quits complaining about '__register_frame_info'</h2>
This is related to having compiled Window Maker on a system whose libraries were compiled
by egcs or gcc 2.8.0, and then using the binaries on a system whose libraries were compiled 
by gcc 2.7.2.x
<P>
Try compiling Window Maker with the newer gcc or recompile your system libraries
with the older gcc.  It's generally a bad idea to mix and match.
<A name="56"></A>
<h2>2.12 &nbsp;How do I make libjpeg link against Window Maker?</h2>
The newest jpeg libs are availible at http://www.ijg.org
<P>
How many of you have seen that darned "lib reports 62 caller expects 61" type
of error?  Here are some answers that will possibly help you out.
<P>
First things first. As always, make sure there are not older copies of libjpeg
floating around on your system. ]Some distributions by default come
with an old libjpeg.so.1 in the /usr/X11R6/lib/ directory. This can simply be
deleted. Or if something complains after you delete it, recompile it if you
can to look for the new lib in the right place, or if that fails, as a last
resort, you might add a symlink to the new lib like so:
ln -s /usr/local/lib/libjpeg.so.6.0.2 libjpeg.so.1
<P>
Note that you should use your system's version of ldconfig 
to properly manage your library cache (or other appropriate 
mechanism).
<P>
On Linux, this would mean having /usr/local/lib in 
/etc/ld.so.conf and then running ldconfig.
<P>
Now on to the error.  This is basically caused 
by your application having been compiled to
dynamically use the libjpeg.so shared library. 
When you install a new lib and then try to run 
your program again, it expects the lib it was 
compiled against, in this case the older 
libjpeg.so.6.0.1 and instead finds libjpeg.so.6.0.2 
and reports the error.
<P>
The fix is actually rather simple. Along with adding a 
libjpeg.so.6 symlink like so (just in case):
ln -s libjpeg.so.6.0.2 libjpeg.so.6 where you installed
 your new lib, you simply need to recompile your app to
link it against the new library.
<P>
Also, make sure to use GNU make for the Window Maker compile.
<A name="57"></A>
<h2>2.13 &nbsp;How do I start Window Maker after running wmaker.inst?</h2>
As of version 0.53.0, the wmaker.inst script will modify your X startup script
( .xinitrc or .Xclients or .Xsession )  to do something thats (hopefully) 
appropriate.
<P>
In order to run wmaker, a user needs to have an ~/.xinitrc
file consisting of something similar to
<P>
<PRE>
	#!/bin/sh
	exec wmaker
</PRE>
<P>
This will vary from system to system, but the existance
of an .xinitrc file will generally override the system
defaults.
<A name="58"></A>
<h2>2.14 &nbsp;How do I make libpng link against Window Maker?</h2>
The newest png libs are availible at http://www.libpng.org/pub/png/libpng.html
<P>
You should also get the newest zlib libs from
http://www.gzip.org
<P>
Generally, the same rules apply here as with libjpeg. Make sure there are no
older versions of the necessary libs floating around on your system, then try
to configure and make again.
<P>
Also, make sure to use GNU make (gmake) for the Window Maker compile.
<A name="59"></A>
<h2>2.15 &nbsp;How do I make giflib or libungif to link against Window Maker?</h2>
The newest versions of both these libraries are available at
http://prtr-13.ucsc.edu/~badger/software/


Users have had a few problems with giflib... it seems that the install process didn't
install the header file libgif.h, so although the Window Maker configure found
the lib (libgif.so.x), when you actually try to compile, it fails when it looks for
the header to include the make.  One solution is to simply copy it from the libgif 
source directory to the local system include directory. (/usr/local/include/ on many
systems).
<P>
Also, make sure to use GNU make (gmake) for the Window Maker compile.
<A name="60"></A>
<h2>2.16 &nbsp;How do I fix an error similar to "wrlib: could not allocate shared memory segment: invalid argument"</h2>
This relates to a shared memory problem on
Solaris. Usually one can't see it - but it is visible if X
is started from command line (or fail-safe session for that
matter). In any of the cases, on the stderr you get an error
message like this:
<P>
<PRE>
	 "wrlib: could not allocate shared memory segment: invalid argument"
</PRE>
This one is generated by wrlib if Window Maker is compiled with
shared-memory usage enabled (which is the default). The explanation
is that Solaris by default comes with a shared memory segment size 
of maximum 1 M. What happends is that if you have a really-really
cool(tm) background, it is usually much bigger than that 1 M segment
of shared memory. To see your defaults relating the IPC settings check
the output of the "sysdef" command (look for IPC Shared Memory). There 
you should see the maximum allocable size for a shared memory segment. If 
it is less than 5 M you should really increase it by adding the following 
line in your /etc/system file: 
<P>
<PRE>
	set  shmsys:shminfo_shmmax=20971520 
</PRE>
<P>
*) Make sure you don't already have this value set.  If you do,
simply increase the value.  In case you have a much 
bigger value, stick to what you have, because you should have 
no problems with it.<BR>
*)  The value allows a maximum segment size of 20 M, which really
should be enough for anyone.  If not, try using a smaller background
image!<BR>
*)  Make sure you spell the line <I>exactly</I> as shown, otherwise
at boot time the kernel will complain of not finding such a module
name and will not set a thing about it!<BR>
*)  Make sure you don't delete other lines or modify them "beyond
recognition", for evil things may happen at boot time.<BR>
<P>
After adding this to your /etc/system you need to reboot
in order for the new limit to take effect.
Also, you may want to check the new limit just 
to make sure it has been set.
<P>
Thanks to Bogdan Iamandei for this answer.
<A name="61"></A>
<h2>2.17 &nbsp;How do I add Window Maker to the Solaris dtlogin screen?</h2>
The two files that
determine alternate window managers are:
<P>
<PRE>
	/usr/dt/config/C/Xresources.d/Xresources.*
	/usr/dt/config/Xsession.*
</PRE>
If you look in there, you'll find Xresources.ow and Xsession.ow,
respectively.  All you need are two files that set up Window Maker (or
any other window manager) in a similar fashion, calling them
Xresources.wm and Xsession.wm (or whichever extension you prefer).
<P>
Here is an example setup:
<P>
<P>
<PRE>
	#
	**************************************************************************
	#
	# Window Maker config file
        # Mike Bland &lt;mbland@cmu.edu&gt;
     	#
	# /usr/dt/config/C/Xresources.d/Xresources.wm
	#
	# used by dtlogin
	#
	#
	**************************************************************************
</PRE>
<PRE>
	Dtlogin*altDtsIncrement:        True
</PRE>
<PRE>
	Dtlogin*altDtName:      Window Maker
	Dtlogin*altDtKey:       /usr/local/bin/wmaker
	Dtlogin*altDtStart:     /usr/dt/config/Xsession.wm
	#Dtlogin*altDtLogo:     /usr/local/share/logos/WM_logo.xpm
</PRE>
<PRE>
	# Once I get a logo ready, I'll add it to the dtlogin screen by
	uncommenting
	# the last line.
</PRE>
<P>
And this example script:
<P>
<PRE>
	#!/bin/ksh
	#
	**************************************************************************
	#
	# Window Maker startup script
        # Mike Bland &lt;mbland@cmu.edu&gt;</A>
	# /usr/dt/config/Xsession.wm
	#
	# used by dtlogin
	#
	#
	**************************************************************************
</PRE>
<PRE>
	. /usr/local/etc/.profile       # Sources the file containing necessary
	                                # environment variables (especially
	                                # LD_LIBRARY_PATH=/usr/local/lib:...);
        	                        # make sure it's executable.
</PRE>
<PRE>
	WINDOW_MANAGER=/usr/local/bin/wmaker
</PRE>
<PRE>
	export WINDOW_MANAGER
</PRE>
<PRE>
	/usr/local/bin/wmaker
</PRE>
<A name="127"></A>
<h2>2.18 &nbsp;What happened to libPropList?</h2>
The libPropList dependency has been removed as of Window Maker version 0.70.0,
and is replaced by cleaner, more robust code in the WINGs toolkit.  This new code maintains existing proplist compatibility,
so there are no visable changes for users, and existing file formats will work as they did before.

<P>

For developers, there is a proplist-compat.h header that provides a mapping between the old and new function names.  See the comments in this file for further instructions.<BR>
</BLOCKQUOTE>

<?
}
if ($chapter == 3) {
?>
<h1>Chapter 3: Configuring Window Maker</h1>
<UL type="disc">
<B>3.1</B> &nbsp; <A href="#73">What are those files inside my ~/GNUstep directory?</A><BR>
<B>3.2</B> &nbsp; <A href="#74">How do I enable the normal X sloppy focus mode?</A><BR>
<B>3.3</B> &nbsp; <A href="#75">How do I get my auto-arrange icons to work?</A><BR>
<B>3.4</B> &nbsp; <A href="#76">How do I get my Meta-Tab to cycle through windows correctly?</A><BR>
<B>3.5</B> &nbsp; <A href="#77">How do I get a tile background for my appicons (those things in the dock)?</A><BR>
<B>3.6</B> &nbsp; <A href="#78">How do you dock &lt;insert program here&gt; that doesn't have an appicon in the new version of WindowMaker?</A><BR>
<B>3.7</B> &nbsp; <A href="#79">How do I get x11amp to not have a title bar ( or any other program for that matter )?</A><BR>
<B>3.8</B> &nbsp; <A href="#80">How do I set a pixmap background?</A><BR>
<B>3.9</B> &nbsp; <A href="#81">Can I put pixmaps in my root menu and title bars?</A><BR>
<B>3.10</B> &nbsp; <A href="#82">How do I get my Minimize Icon to look like the triangle I see in screenshots?</A><BR>
<B>3.11</B> &nbsp; <A href="#83">Why does Netscape have a black and white Icon when I minimize it?</A><BR>
<B>3.12</B> &nbsp; <A href="#84">How do I get superfluous bells and whistles working?</A><BR>
<B>3.13</B> &nbsp; <A href="#85">How do I get the classic NeXT(tm)-like style back?</A><BR>
<B>3.14</B> &nbsp; <A href="#86">How do I get the window menu with only a two button mouse?</A><BR>
<B>3.15</B> &nbsp; <A href="#87">How do I edit my root menu?</A><BR>
<B>3.16</B> &nbsp; <A href="#88">WPrefs disappeared from the Dock!  How do I get it back?</A><BR>
<B>3.17</B> &nbsp; <A href="#89">How can I define my own Icon for a program? (instead of the Icon the Application Supplies?)</A><BR>
<B>3.18</B> &nbsp; <A href="#90">How do I turn off the workspace titles between workspaces?</A><BR>
<B>3.19</B> &nbsp; <A href="#91">How do I add dynamic items to my root menu?</A><BR>
<B>3.20</B> &nbsp; <A href="#92">How do I remove or hide appicons?</A><BR>
<B>3.21</B> &nbsp; <A href="#93">I disabled my titlebar. How can I get it back?</A><BR>
<B>3.22</B> &nbsp; <A href="#94">How do I remove ALT+Mouse1 from the action Window Maker grabs for an application?</A><BR>
<B>3.23</B> &nbsp; <A href="#95">How do I configure the Dock and Clip to use less space on a small screen?</A><BR>
<B>3.24</B> &nbsp; <A href="#128">Why do dashes not work as menu entries?</A><BR>
</UL>
</FONT>
<BR><HR><BR>
<A name="73"></A>
<h2>3.1 &nbsp;What are those files inside my ~/GNUstep directory?</h2>
Here is a synopsis of the files in ~/GNUstep
<P>
~/GNUstep/WindowMaker/WindowMaker is main config file. This 
file controls options such as keybindings, fonts,
pixmaps, and focus modes. 
<P>
~/GNUstep/WindowMaker/WMWindowAttributes controls the 
"attributes" for individual applications and appicons. 
Options such as what icon to use are set here. For the
most part, this is now best accessed via a right click 
on a title bar of an application and selecting 
"Attributes" 
<P>
~/GNUstep/Defaults/WMState is the file that is 
automatically generated and contains the current 
dock settings. It is not recommended to edit this 
file by hand.
<P>
~/GNUstep/Defaults/WMRootMenu specifies what file to use 
as the root menu. In Window Maker 0.19.0 and higher, this
file should be replaced by plmenu from ~/GNUstep/Defaults/WindowMaker
so that one can use WPrefs.app to edit the menu.
<P>
~/GNUstep/Library/WindowMaker/menu is used to change your root menu, 
if you are using the old menu style.
<A name="74"></A>
<h2>3.2 &nbsp;How do I enable the normal X sloppy focus mode?</h2>
If you are using WPrefs, you can choose the ``Window Focus Prefrences''
tab and then select the ``Input Focus Mode'' Slider.
<P>
Scroll Down and choose ``Sloppy'' Focus Mode.
<P>
You may also use a text editor on
<P>
<PRE>
	~/GNUstep/Defaults/WindowMaker
</PRE>
and change the following:
<P>
<PRE>
	FocusMode = sloppy;
</PRE>
<A name="75"></A>
<h2>3.3 &nbsp;How do I get my auto-arrange icons to work?</h2>
In WPrefs, choose the ``Icon Prefrences Tab'' and select the
``Auto Arrange Icons'' Checkbox.
<P>
Or in
<PRE>
	~/GNUstep/Defaults/WindowMaker
</PRE>
set
<PRE>
	AutoArrangeIcons=YES;
</PRE>
and the icons should now auto-arrange.
<A name="76"></A>
<h2>3.4 &nbsp;How do I get my Meta-Tab to cycle through windows correctly?</h2>
To use WPrefs to modify these, choose the ``Ergonomic Prefrences'' tab and
check ``Raise window when switching focus with keyboard (Circulate Raise)''
<P>
Or you can use a text editor to make sure that these settings are in your
~/GNUstep/Defaults/WindowMaker file:
<P>
<PRE>
	CirculateRaise = YES;
	RaiseDelay = 1;
</PRE>
As of 0.61.0, MS Window's Style application tabbing is supported by default.
<A name="77"></A>
<h2>3.5 &nbsp;How do I get a tile background for my appicons (those things in the dock)?</h2>
These can all be adjusted by the ``Appearance Preferences'' tab in
WPrefs.
<P>
Select the tile and then choose the edit texture dialog. Then you may
choose any of the different tile background options in the
The old text editor method is provided below for convience.
<P>
You need to change one line in your '~/GNUstep/Defaults/WindowMaker' file.
<P>
<PRE>
	IconBack = (spixmap, tile.black.xpm, white);
</PRE>
The last parameter is the color that fills in any transparent
parts of your icon.
<A name="78"></A>
<h2>3.6 &nbsp;How do you dock &lt;insert program here&gt; that doesn't have an appicon in the new version of WindowMaker?</h2>
There is now an option available to emulate appicons so that Window Maker
can dock just about anything now.  To dock a misbehaving application, right click 
on the title bar and select the attributes menu.  Next, select the pull down menu's 
"Advanced Options" item.  Under the ``Advanced Options'' menu, select the ``Emulate 
Application Icon'' Option then Save, Apply and close the dialog.
<P>
This should allow you do dock the program normally.
<P>
Dan Pascu adds:
<P>
Emulate Appicon does exactly the same as dockit. So if Emulate Appicon
does not work, dockit will not work either.
For such apps you can do nothing. They are badly coded (they do not
set the instance.class hints). For these Attributes are also not
available, since attributes apply to an instance and/or class hint.
<P>
Note: Dockit was previously distributed with Window Maker and was launched
from the top dock icon.
<P>
Elliott Potter adds:
<P>
There's another way to dock applications that misbehave ... I've only
done this with a couple of things (Adobe AcroRead is the only one I
remember at the moment).
<P>
If Attributes -> Advanced Options -> Emulate Application Icon doesn't
work:
<P>
Dock another application to the clip, where you want your application to
go.  I used gv, but anything you can dock will work.
Quit WindowMaker
Edit ~/GNUstep/Defaults/WMState
If you're docking to the clip, scroll down to the Workspaces section.
When you find whatever you docked, you'll see:
<PRE>
	{
	  Command = gv;
	  Name = GV.gv;
	  AutoLaunch = No;
	  Forced = No;
	  BuggyApplication = No;
	  Position = "6,0"
	  Omnipresent = No;
	  DropCommand = "gv %d";
	},
</PRE>
Edit it to use the info for your new application:
<PRE>
	{
	  Command = acroread;		# use the full pathname if you have to
	  Name = acroread.acroread;
	  AutoLaunch = No;
	  Forced = No;
	  BuggyApplication = No;
	  Position = "6,0"
	  Omnipresent = No;
	  DropCommand = "acroread %s";
	},
</PRE>
Then edit WMWindowAttributes, and add a line for your application's
icon...you can edit the line that was inserted, or make a new one - I
just make a new one:
acroread.acroread = {Icon = pdf.tiff;};
<P>
<P>
Then re-start WindowMaker, and your icon should be there!  You can move
it around like any other docked app now, but the Attributes section
still won't work.
<A name="79"></A>
<h2>3.7 &nbsp;How do I get x11amp to not have a title bar ( or any other program for that matter )?</h2>
Right Click on the title bar and go to the attributes menu. Click on Window Attributes and click
the the Disable titlebar and Disable Resizebar options. Click Save, and then click
Apply then close the Attributes panel.
<P>
By Default, to get back to the attributes menu, use the key combination Control-Esc.
<A name="80"></A>
<h2>3.8 &nbsp;How do I set a pixmap background?</h2>
Here is the in depth explanation straight from the NEWS file:
<P>
wmsetbg now accepts the following options:
<PRE>
	usage: wmsetbg [-options] image
	options:
	-d
              dither image
	-m
              match colors
	-t
              tile image
	-s
              scale image (default)
	-u
              update Window Maker domain database
	-D &lt;domain&gt;
              update &lt;domain&gt; database
	-c &lt;cpc&gt;
              colors per channel to use
</PRE>
<P>
<P>
By default, it will try to guess if dithering is needed or not and proceed
accordingly.
Using -d or -m will force it to dither or match colors.
<P>
Dithering for more than 15bpp is generally not needed, and will only result
in a slower processing.
Don't use dithering except when needed, because it is slower. Else rely on
wmsetbg which will detect if dithering is needed and use it.
<P>
-u
<PRE>
   will update the WorkspaceBack in the default database
   domain file in ~/GNUstep/Defaults/WindowMaker, and let Window
   Maker refresh the screen. Please note that this option only
   works under Window Maker, and will have no effect under
   other window managers, since it rely on Window Maker to
   update the image after it reads the updated defaults
   database.
</PRE>
-D
<PRE>
   &lt;domain&gt; is same as above, but will update the domain
   &lt;domain&gt; instead of the default Window Maker domain.
</PRE>
-c
<PRE>
   &lt;cpc&gt; will set the color per channel to use. Only needed for
   PseudoColor visuals. Window Maker will automatically pass
   the value read from the Window Maker domain database.
</PRE>
<P>
<P>
The following line is straight from your WindowMaker-0.15.x
~/GNUstep/Library/WindowMaker/menu file and should all be on one line.
<P>
"Images" OPEN_MENU BACKGROUNDS_DIR
~/GNUstep/Library/WindowMaker/Backgrounds WITH wmsetbg -u -t
<P>
This should give you an idea on how to add other entries for different image
directories. See the help info at the top of the
~/GNUstep/Library/WindowMaker/menu file for more information.
<P>
If you for some reason would like to set your background image with XV, for
instance to use an image format not yet supported by wmsetbg or to use one
of XV's special modes, edit the file ~/GNUstep/Library/WindowMaker/autostart
and insert the line
<P>
<PRE>
	xv -root -quit -maxpect ~/background.jpg
</PRE>
or
<PRE>
	xv -root -quit -max ~/background.jpg
</PRE>
you can also try variations of this to get different tiling and other effects
(where X is a number 1-9 I believe):
'xv -root -quit -rmodeX ~/background.jpg'
<P>
If you would like xv functionality in your menu, heres a nice little tip from
Alfredo:
<P>
Add the following line to your ~/GNUstep/Library/WindowMaker/menu file. (all on
one line)
<P>
"More Backgrounds" OPEN_MENU /home/whoever/backgrounds xv -root -maxpect -quit
<A name="81"></A>
<h2>3.9 &nbsp;Can I put pixmaps in my root menu and title bars?</h2>
Put the pixmaps in a directory that is located in your pixmap path set
on ``Search Path Configuration'' Tab.
<P>
Then switch ``Appearance Preferences'' tab and select what widget you would
to adjust under the ``Texture'' tab. Click edit. Chose an image texture format
and then search for the texture.
<P>
You can use a similar procedure for any type of menu editing.
<P>
You can use png, gif, ppm, tiff, jpeg and xpm images interchangeably in
Window Maker if you have compiled in support for those formats.
<A name="82"></A>
<h2>3.10 &nbsp;How do I get my Minimize Icon to look like the triangle I see in screenshots?</h2>
This involves a minor source tweak. Instructions are available at
http://largo.windowmaker.org/tips.php#titlebar_icons
<A name="83"></A>
<h2>3.11 &nbsp;Why does Netscape have a black and white Icon when I minimize it?</h2>
Craig Maloney  has this answer:
<P>
If you happen to --enable-openlook at compile time,
Netscape (and presumably other apps as well) believe 
they're running under OLVWM, and minimise with 
monochrome icons. Once compiled without OpenLook support,
Netscape minimizes with the correct icon.
<A name="84"></A>
<h2>3.12 &nbsp;How do I get superfluous bells and whistles working?</h2>
Open WPrefs and go under  the ``Other Configurations'' tab. Under
``Animations and Sound'', depress the Superfluous tab.
<P>
Alternatively, you may add
<PRE>
	Superfluous=YES;
</PRE>
to your ~/GNUstep/Defaults/Windowmaker file.
</PRE>
<A name="85"></A>
<h2>3.13 &nbsp;How do I get the classic NeXT(tm)-like style back?</h2>
Open WPrefs and go under the ``Other Configurations'' tab. Under ''Title Bar Style'',
select the classic look.
<P>
Or you can add
<PRE>
	NewStyle=NO;
</PRE>
to your ~/GNUstep/Defaults/Windowmaker file.
<A name="86"></A>
<h2>3.14 &nbsp;How do I get the window menu with only a two button mouse?</h2>
In WPrefs, under ``Mouse Prefrences'', the mouse actions can be mapped
to a button of choice.
<P>
Jim Noble  explains another way to do this:
<P>
If you've got a two-button mouse under some versions of Solaris x86, there's no way
(that I'm aware of) to emulate a 3-button mouse. The right button can be either MB2
or MB3, but chording doesn't work.
<P>
<PRE>
	ApplicationMenuMouseButton = Left;
</PRE>
and
<PRE>
	WindowListMouseButton = Right;
</PRE>
in ~/GNUstep/Defaults/WindowMaker ought to allow the left button to activate the root
menu, and the right button (as MB2) to activate the windows menu.
<A name="87"></A>
<h2>3.15 &nbsp;How do I edit my root menu?</h2>
You can now use WPrefs.app ( its appicon looks like a heart rate meter
with a GNUStep icon backgroud ). Note that this will replace any oldstyle
menus and there is no way to convert the oldstyle menu to the
new libproplist style menu.
<P>
For old style menus, edit the file
<P>
<PRE>
	~/GNUstep/Library/WindowMaker/menu
</PRE>
and save your changes.  Window Maker should detect the change and
automatically update.  If you are having a problem getting it to reload the menu,
try
<P>
<PRE>
	touch menu
</PRE>
to force the modification time into the future.
<A name="88"></A>
<h2>3.16 &nbsp;WPrefs disappeared from the Dock!  How do I get it back?</h2>
Pascal Hofstee  offers this answer:
<P>
You should just start it from a terminal by supplying it's FULL path-name,
which is usually the following:
<P>
<PRE>
    /usr/local/GNUstep/Apps/WPrefs.app/WPrefs
</PRE>

At this point, a new appicon should be generated
which can be placed back into the Dock.
<A name="89"></A>
<h2>3.17 &nbsp;How can I define my own Icon for a program? (instead of the Icon the Application Supplies?)</h2>
You can right click on the titlebar of the running app and choose the
"Attributes..." option, then click on the "Ignore client supplied icon"
checkbox. Click "Apply", "Save" and close the Attributes Editor.
<P>
Another method is to edit ~/GNUstep/Defaults/WMWindowAttributes by hand and
use the AlwaysUserIcon=YES; option for the app. For example:
<P>
xmcd = { Icon = "Radio.xpm";
AlwaysUserIcon=Yes;
};
<A name="90"></A>
<h2>3.18 &nbsp;How do I turn off the workspace titles between workspaces?</h2>
In Window Maker 0.60.0, an option was added to turn this off.
<P>
By editing 
<PRE>
	~/GNUstep/Defaults/WindowMaker
</PRE>
insert or modify the key
<P>
<PRE>
	WorkspaceNameDisplayPosition = none;
</PRE>
Other valid options for this include
center/top/bottom/topleft/topright/bottomleft/bottomright;
<A name="91"></A>
<h2>3.19 &nbsp;How do I add dynamic items to my root menu?</h2>
A few programs are floating about, notably wkdemenu.pl that can produce
output from other menu styles. In order to get WindowMaker to launch the process everytime
you want to use the menu, use something like
<P>
<PRE>
	  ("External Menu", OPEN_MENU, "| bob.sh")
</PRE>
in a proplist style menu. You can tell if you have a proplist style menu if you can 
edit it with WPrefs.
<P>
You can do this directly in WPrefs by going to the menu editor, adding an "external menu",
and then clicking the "ask guru button" and filling in the process name.
<P>
Thanks to Igor P. Roboul 
<A name="92"></A>
<h2>3.20 &nbsp;How do I remove or hide appicons?</h2>
There are two options here, and you need to consider which one you prefer.
Read both of these before you decide.
<P>
First, if you do not want to use the clip or dock at all, you can launch wmaker with
with
<P>
<PRE>
	wmaker --no-clip --no-dock
</PRE>
and then in
<P>
<PRE>
	~/GNUstep/Defaults/WMWindowAttributes
</PRE>
add
<P>
<PRE>
  "*" = {NoAppIcon=Yes;};
</PRE>
The problem with this method is if you use the dock for dockapps, it renders them
with out an appicon to write to.
An alternative method if you are willing to let the clip be on your desktop is
to right click on the clip > clip options > auto attract.
Double click the clip so that it is grayed and all appicons will be hidden.
Then you can hide the clip behind the dock so that it is out of your way.
This will allow appicons to work.
<A name="93"></A>
<h2>3.21 &nbsp;I disabled my titlebar. How can I get it back?</h2>
Thanks to  Jim Knoble for this answer
<P>
Set the focus to the window and then use the keystroke assigned to the
titlebar menu.  If you're not sure what the keystroke is, you can find
out using WPrefs:  in the keyboard section, select the `Open window
commands menu' item in the list of actions.  The keystroke assigned to
it ought to appear in the `Shortcut' area'.
<P>
Typically it is Control-Esc or F10 in older version of WindowMaker.
<A name="94"></A>
<h2>3.22 &nbsp;How do I remove ALT+Mouse1 from the action Window Maker grabs for an application?</h2>
Do [Button3Down] (for righthanded mouse users, [RightButtonDown]) on
the titlebar of the desired window.  Choose ``Attributes...''.  In the
Attributes inspector, choose ``Advanced Options''.  Check ``Don't Bind
Mouse Clicks''.  Apply or Save as desired, then close the Attributes
inspector.
<P>
The result is that [Alt+Button1] (which usually grabs a window to move
it around), [Alt+Button2] (which usually grabs a window to move it
around without changing the window stacking order), and [Alt+Button3]
(which usually resizes a window) all get passed to the application
instead of performing their usual action.
<A name="95"></A>
<h2>3.23 &nbsp;How do I configure the Dock and Clip to use less space on a small screen?</h2>
This answer is current as of WindowMaker-0.61.1.
<P>
For the Clip, either:
<P>
(a) Disable the Clip from WPrefs (panel number 7), or<BR>
(b) Hide the Clip under the Dock (for example, in the upper righth
and corner of the screen).
<P>
[b] is probably more useful on desktops with limited space, since you
can still set the Clip to attract app-icons so they don't clutter your
desktop.
<P>
For the Dock, try the following:
<P>
(1) Exit Window Maker.<BR>
(2) Log in via a text console or using a different window manager.<BR>
(3) Edit ~/GNUstep/Defaults/WMState using your favorite text editor
(for example, vi, emacs, or pico).<BR>
(4) Find the `Applications' part of the `Dock' structure.  Find the
item with `Position = "0,0";'.  Change the `Command' item to the
command you want the top tile to launch.  Change the `Name' item
to the "&lt;instance&gt;.&lt;class&gt;" name of the application you just made
the Command item start (for example, if `Command' is `"xedit"',
then `Name' should be `xedit.Xedit').<BR>
(5) Save the WMState file.<BR>
(6) Start an X session with Window Maker.<BR>
(7) Check that the top tile starts the command you told it to.  (You
should still also be able to move the Dock up and down using
[LeftDrag] on the top tile.)<BR>
(8) You can configure the tile (including autolaunch and the
drop-command) in the regular manner ([RightButtonDown] on the
tile and choose `Settings...' from the resulting menu).
<A name="128"></A>
<h2>3.24 &nbsp;Why do dashes not work as menu entries?</h2>
If you wish to use a - as part of a menu item name, you must enclose the name in double quotes.  This will only apply if you're editing the ~/GNUstep/Defaults/WMRootMenu file manually, as it is handled properly within WPrefs.

<pre>
This will work:

(ssh,
("us-gw", EXEC, "Eterm -e ssh us-gw"),

This will not:

(ssh,
(us-gw, EXEC, "Eterm -e ssh us-gw"),
</pre>

<P>
Thanks to Martin Sillence for pointing this out.<BR>
</BLOCKQUOTE>
<?
}
if ($chapter==4) {
?>
<h1>Chapter 4: Using Window Maker</h1>
<UL type="disc">
<B>4.1</B> &nbsp; <A href="#62">How do add new icons to the Dock?</A><BR>
<B>4.2</B> &nbsp; <A href="#63">What is the difference between the Exit and Exit Session Option?</A><BR>
<B>4.3</B> &nbsp; <A href="#64">How do I "dock" icons on the clip?</A><BR>
<B>4.4</B> &nbsp; <A href="#65">Why do none of my key bindings (ie: Alt+#) work in Window Maker?</A><BR>
<B>4.5</B> &nbsp; <A href="#66">How do I rename workspaces?</A><BR>
<B>4.6</B> &nbsp; <A href="#67">How can I resize a window if the window is larger than my current desktop?</A><BR>
<B>4.7</B> &nbsp; <A href="#68">How do I "undock" appicons?</A><BR>
<B>4.8</B> &nbsp; <A href="#69">I docked an application but when I run it the button is permanently shaded and I can't run new instances.</A><BR>
<B>4.9</B> &nbsp; <A href="#70">When I run wmaker it complains about not being able to load any fonts.</A><BR>
<B>4.10</B> &nbsp; <A href="#71">When I set the root background with wmsetbg by hand it works, but when I do that from the configuration files it doesnt!</A><BR>
<B>4.11</B> &nbsp; <A href="#72">What is the purpose of being able to draw a box on the root menu with a left click?</A><BR>
</UL>
</FONT>
<BR><HR><BR>
<A name="62"></A>
<h2>4.1 &nbsp;How do add new icons to the Dock?</h2>
First, launch an application. If an icon (henceforth called an ``appicon'')
appears in the bottom left corner of the screen, left click and drag it over
near the Dock.  You will see a slightly opaque square of where the
Dock will place the appicon.  When you do, release the mouse
button and the appicon should now be in the Dock.  
<P>
Next, right click on the desktop to bring up the menu.
Select Workspace -> Save Session to make this permanent.
<A name="63"></A>
<h2>4.2 &nbsp;What is the difference between the Exit and Exit Session Option?</h2>
Another answer from Dan Pascu:
<P>
"Exit will exit wmaker, but can leave other X apps running, provided that it was
not the last app launched in the .xinitrc (for instance, if you had exec wmaker, 
followed by exec xterm, exiting wmaker using 'Exit' will leave the xterm
running so you could start another window manager, etc.)  This is accomplished 
because X will not shutdown unless all X apps are closed.
<P>
Exit session will exit wmaker, but will also
close all running apps, thus the X server will be closed
too."
<A name="64"></A>
<h2>4.3 &nbsp;How do I "dock" icons on the clip?</h2>
Just drag icons near it like you would for the dock. If you are having a problem
docking icons, you should try moving the clip away from the dock.
<A name="65"></A>
<h2>4.4 &nbsp;Why do none of my key bindings (ie: Alt+#) work in Window Maker?</h2>
If you are using XFree86, make sure scroll lock and numlock are off or no bindings
will work (XFree bug). You can try using the XFree86 Numlock Hack by editing the
line #undef NUMLOCK_HACK in $(WindowMaker)/src/wconfig.h and changing it to
#define NUMLOCK_HACK.
<P>
With the release of 0.18.0, this hack is now working and hopefully no
one will have to ask this question again.
<A name="66"></A>
<h2>4.5 &nbsp;How do I rename workspaces?</h2>
Right click to bring up the root menu.  Go under
the Workspaces menu item and hold the control key down.
Next, click on the workspace entry you
would like to rename, type the name, and
press enter.
<A name="67"></A>
<h2>4.6 &nbsp;How can I resize a window if the window is larger than my current desktop?</h2>
David Reviejo best summed up this answer:
<P>
"Maybe you know:
Alt+Left click and drag
to move the window.
<P>
Try this:
Alt+Right click and drag
to resize (by moving the nearest window corner)
<P>
Another move/resize tip: while you are moving or resizing a window, you
can change the move/resize mode by pressing the SHIFT key."
<A name="68"></A>
<h2>4.7 &nbsp;How do I "undock" appicons?</h2>
If the program is not running, just drag the icon to the middle of your
desktop and watch it disappear.  If the program is running, hold down Meta and drag the icon off the dock.
<A name="69"></A>
<h2>4.8 &nbsp;I docked an application but when I run it the button is permanently shaded and I can't run new instances.</h2>
You probably docked the application with dockit. To fix it
remove the icon and use the "Emulate Application Icon" checkbox in
the Advanced Options section of the Attributes panel for the window.
Then restart the application to get the application icon you
must use to dock the application.
It can also mean that you did something you shouldn't, which is
changing the program that is ran from the docked icon. For example,
if you docked rxvt you must NOT change it to xterm, for example.
You also can't do any changes that might alter the contents of
the WM_CLASS hint for the window, like the -name parameter for
xterm, rxvt and other programs.
<A name="70"></A>
<h2>4.9 &nbsp;When I run wmaker it complains about not being able to load any fonts.</h2>
Check if the locale settings are correct. If you're not sure what to
do, unset the LANG environment variable before running wmaker.
<P>
TODO: give complete explanation
<A name="71"></A>
<h2>4.10 &nbsp;When I set the root background with wmsetbg by hand it works, but when I do that from the configuration files it doesnt!</h2>
If you set the root background with wmsetbg by hand, it will obviously
find the image, since you have explicitly specified it by hand. But if you
simply put it in
<PRE>
	~/GNUstep/Defaults/WindowMaker
</PRE>
in some option like WorkspaceBack, it will not find the image because Window Maker can't read
your mind to figure where you put the image. So, to fix it, you have to
either place the full path for the image in the texture specification or put
the path for the directory you put your background images in the PixmapPath
option. You can also put all your background images in places like
<PRE>
	~/GNUstep/Library/WindowMaker/Backgrounds
</PRE>
or
<PRE>
	/usr/local/share/WindowMaker/Backgrounds
</PRE>
David Green says that another possibility is that you have two copies of the worker programs:
wmsetbg (and possibly setstyle) and the wrong one is in the path first.
<A name="72"></A>
<h2>4.11 &nbsp;What is the purpose of being able to draw a box on the root menu with a left click?</h2>
Its purpose is two-fold.
<P>
First, it is used to select multiple windows on a desktop at a time.  When these
windows are selected, they can be moved around on your desktop and will retain
their relative positions.
<P>
Second, once selected, they are persistent through desktop changes.  So it
is useful for moving large numbers of windows between desktops.
<P>
You can also select windows with shift+click.<BR>
</BLOCKQUOTE>
<?
}
if ($chapter==5) {
?>
<h1>Chapter 5: Application Compatibility</h1>
<UL type="disc">
<B>5.1</B> &nbsp; <A href="#96">How do I assign gimp an appicon?</A><BR>
<B>5.2</B> &nbsp; <A href="#97">How do I get an appicon for XEmacs 20.3+?</A><BR>
<B>5.3</B> &nbsp; <A href="#98">Where do I get the nifty clock program I always see on people's desktops?</A><BR>
<B>5.4</B> &nbsp; <A href="#99">How do you dock asclock?</A><BR>
<B>5.5</B> &nbsp; <A href="#100">Where can I get more dockapps?</A><BR>
<B>5.6</B> &nbsp; <A href="#101">How do I get an appicon for rxvt so I can dock it?</A><BR>
<B>5.7</B> &nbsp; <A href="#102">How do I allow Alt+# to work in an rxvt/xterm session?</A><BR>
<B>5.8</B> &nbsp; <A href="#103">How do I get different icons for different rxvt's and xterms?</A><BR>
<B>5.9</B> &nbsp; <A href="#104">How do I launch multiple instances of XTerm from one appicon?</A><BR>
<B>5.10</B> &nbsp; <A href="#105">Window Maker breaks scilab.</A><BR>
<B>5.11</B> &nbsp; <A href="#106">How do I get a transparent xterm/rxvt/xconsole?</A><BR>
<B>5.12</B> &nbsp; <A href="#107">How do I dock an arbitrary console application like mutt?</A><BR>
<B>5.13</B> &nbsp; <A href="#108">How do I get an appicon for Netscape?</A><BR>
<B>5.14</B> &nbsp; <A href="#109">How can I dock an application from a remote machine using ssh?</A><BR>
<B>5.15</B> &nbsp; <A href="#110">How do you make an omnipresent window not take focus whenever switching workspaces?</A><BR>
</UL>
</FONT>
<BR><HR><BR>
<A name="96"></A>
<h2>5.1 &nbsp;How do I assign gimp an appicon?</h2>
You can enter the following line in WMWindowAttributes:
<P>
<PRE>
	gimp={Icon="gimp.tiff";};
</PRE>
Window Maker now can assign Icons from within the windowmanager. To do so, right
click on the title bar of an app, click on the droplist->Icon and WorkSpace entry, enter the icon file name (
make sure this is in your pixmap path ), click update, apply, and then save.
<A name="97"></A>
<h2>5.2 &nbsp;How do I get an appicon for XEmacs 20.3+?</h2>
Thanks to Michael Hafner for this answer.
<P>
You don't need to patch the XEmacs code, just run
<P>
<PRE>
	./configure --with-session=yes (in addition to any other options you use)
</PRE>
in your XEmacs 20.3+ sourcedir and rebuild it. Then XEmacs shows an appicon
when running and you can easily dock it.
<A name="98"></A>
<h2>5.3 &nbsp;Where do I get the nifty clock program I always see on people's desktops?</h2>
It's called asclock. Once included with Window Maker, it now is available
at <A HREF="ftp://ftp.windowmaker.org/pub/contrib/srcs/apps/asclock.tgz">ftp://ftp.windowmaker.org/pub/contrib/srcs/apps/asclock.tgz</A>
<P>
asclock was written by Beat Christen and used to have its own website, which 
seems to have disappeared.  However, references to it exist all over 
the place, and can be found by searching <A href=http://www.google.com/search?q=asclock%22>Google</A>.
Beat Christen wrote this awhile back:
<P>
"Please note that the asclock-gtk version 2.0 beta 4
(asclock-gtk-2.0b4.tar.gz) does not have the -d switch yet and that the
asclock-xlib-2.1b2.tar.gz does not have the shaped asclock builtin."
<P>
A wonderful alternative to asclock is Jim Knoble's 
<A href="http://www.ntrnet.net/~jmknoble/WindowMaker/wmclock/">wmclock</A>.
It duplicates asclock and adds some much needed improvements.
<A name="99"></A>
<h2>5.4 &nbsp;How do you dock asclock?</h2>
It is highly recommended that you use the asclock mentioned previously in question 5.3.  The asclock that is typically included in AfterStep will not properly dock with Window Maker.  At this point, there are at least four or five different versions of asclock floating about.
<P>
For older versions such as asclock-classic , use a command line similar to
<P>
<PRE>
	asclock -shape -iconic -12 &amp;
</PRE>
For newer versions such as asclock-xlib 2.0 and asclock-gtk use
<P>
<PRE>
	asclock -shape -iconic -12 -d &amp;
</PRE>
Drag it from the top right corner of the clock to the dock. Right click on the icon and
select autolaunch.
<P>
In order to make asclock launch every time you start Window Maker, right click on the
outer edge of the border for asclock until a menu appears. Select the "Settings" item
and then select the "Lauch this Program Automatically" option then select the "OK"
button.
<P>
If you get an error such as sh: /dev/console: Permission denied, login as root, cd to /dev/ and run
<PRE>
	./MAKEDEV console
</PRE>
<A name="100"></A>
<h2>5.5 &nbsp;Where can I get more dockapps?</h2>
The Window Maker team got tired of people E-mailing constantly asking where the websites for obscure dockapps disappeared to.  So we've created the ultimate dockapps community website.
Visit <a href="http://dockapps.org/">dockapps.org</a> for the latest, up-to-date links, information, and download for Window Maker and related dockapps.
<P>
Another large index of dockapp links is available at <A HREF="http://www.bensinclair.com/dockapp">http://www.bensinclair.com/dockapp</A> .  The downside to this is that they're only links, so if someone stops maintaining a dockapp, or their web hosting provider cuts them off, you won't be able to get to it.  Still, Ben Sinclair's site was the first big "dockapp warehouse" site, so we give credit where credit is due. :)

<A name="101"></A>
<h2>5.6 &nbsp;How do I get an appicon for rxvt so I can dock it?</h2>
The default rxvt that comes with most distributions is an outdated version of rxvt. The
newest development version of rxvt is availible from
<A HREF="ftp://ftp.math.fu-berlin.de/pub/rxvt/devel">ftp://ftp.math.fu-berlin.de/pub/rxvt/devel</A>/. As of the time of this writing, the version is
2.4.7 and it natively produces an appicon without a patch.
<P>
John Eikenberry has also created an rpm which is available from
<A HREF="ftp://ftp.coe.uga.edu/users/jae/windowmaker">ftp://ftp.coe.uga.edu/users/jae/windowmaker</A>
<A name="102"></A>
<h2>5.7 &nbsp;How do I allow Alt+# to work in an rxvt/xterm session?</h2>
First, Launch a unique instance of rxvt or xterm. This can be done using the -N
option of rxvt.
<P>
rxvt -name foo -e irc
<P>
Then, go to the Attributes menu ( right click on titlebar -&gt; Attributes) / Advanced
Options and enable "Don't Bind Keyboard shortcuts". Click Save and Apply and you
should be able to run your session without the shortcuts.
<A name="103"></A>
<h2>5.8 &nbsp;How do I get different icons for different rxvt's and xterms?</h2>
The hint is the -name option for xterm or rxvt. This will allow you to change the exact
WM_CLASS in the attributes menu and assign a unique icon.
<P>
<PRE>
	rxvt -name foo -title Testing
</PRE>
Then Right click on the title bar to bring up the attributes menu, and you will be able
to edit the properties for foo.XTerm (ie: assign a unique icon).
<A name="104"></A>
<h2>5.9 &nbsp;How do I launch multiple instances of XTerm from one appicon?</h2>
Thanks for the update by Sara C. Pickett:
<P>
The easiest way to accomplish this is to dock XTerm as normal.  Then
Go to the Attributes menu -> Application Specific and select no application icon
for XTerm.
<P>
Then right-click on the docked appicon and select settings.  Change the
Application Path with arguments section to
<P>
<PRE>
	'/bin/sh -c "exec xterm &amp;"'
</PRE>
<A name="105"></A>
<h2>5.10 &nbsp;Window Maker breaks scilab.</h2>
If you refer to the problem of the "graphics" window of scilab not showing
up in Window Maker, this is caused by a bug in scilab. You can see the
cause of the problem by yourself, by running xprop on the graphic window:
WM_NORMAL_HINTS(WM_SIZE_HINTS):
<PRE>
                user specified location: 136679205, 1074468360
                user specified size: 400 by 300
                program specified minimum size: 400 by 300
</PRE>
Now, when scilab opens it's window, Window Maker nicely does exactly what it
is told, that is, map the window at position 136679205, 1074468360 which
obviously falls outside the screen no matter how big is your monitor ;)
<P>
Meanwhile, the workaround for this is to open the window list menu
(click on the root window with the middle mouse button) and click
on the ScilabGraphic entry. The window should be brought to your
reach. Then, open the window commands menu (right click on window's
titlebar) and open the Attributes panel. Go to the "Advanced Options"
section, check the "Keep inside screen" option and save.
<P>
If you can recompile Scilab, this came from a Scilab developer:
<P>
replace
<PRE>
 size_hints.flags = USPosition | USSize | PMinSize;
</PRE>
with
<PRE>
 size_hints.flags = /** USPosition |**/ USSize | PMinSize;
</PRE>
in routines/xsci/jpc_SGraph.c
<A name="106"></A>
<h2>5.11 &nbsp;How do I get a transparent xterm/rxvt/xconsole?</h2>
You need a terminal emulator that has support for transparency, like
Eterm, the latest rxvt, wterm, aterm or gnome-terminal.
<P>
You can find these programs on <A HREF="http://www.freshmeat.net/">http://www.freshmeat.net</A>
<A name="107"></A>
<h2>5.12 &nbsp;How do I dock an arbitrary console application like mutt?</h2>
There are two key things to do if you want a program (such as mutt) to
be able to start in a terminal window from the Dock or the Clip:
<P>
(1) Make the terminal window start the program you want to run
instead of a shell.  Both xterm and rxvt (and its descendants)
are capable of doing this.  For example:
<P>
<PRE>
        xterm -e mutt
        rxvt -e mutt
	gnome-terminal -e mutt
</PRE>
(2) Convince Window Maker that the resulting terminal window is not a
regular terminal window, but rather some other program instance.
Both xterm and rxvt are also capable of doing this.  Make sure 
that -e is the last command option. For example:
<P>
<PRE>
        xterm -name muttTerm -e mutt 
        rxvt -name muttTerm -e mutt
	gnome-terminal --name=muttTerm -e mutt
</PRE>
This causes the instance of the terminal window that you start to
have an &lt;instance-name&gt;.&lt;class-name&gt; pair of `muttTerm.XTerm'
(usually rxvt's class is also XTerm; don't know about its 
descendants, such as wterm and Eterm).
<P>
Do not use spaces or periods in the instance name.  For example,
these are BAD instance names:
<P>
<PRE>
       xterm -name mutt.term -e mutt 
       rxvt -name 'mutt term' -e mutt
</PRE>
Window Maker will not like you if you use them.
<P>
With a different instance name, you can now do the following:
<P>
- Dock the resulting appicon in the dock, or clip it to the Clip.
<P>
- Assign a different icon and different window properties to
the `special' terminal window running your program (make 
sure you choose the exact `muttTerm.XTerm' window
specification in the Attributes editor).
<P>
- Specify different resource settings for muttTerm in your
~/.Xdefaults file (e.g., different default foreground and
background colors).
<P>
There are a few other non-key things you can do to complete the process: 
<P>
(3) Tell the terminal window to display a more meaningful or prettier
title and icon title than what gets put there due to `-e'.  For 
example:
<P>
<PRE>
        rxvt -title 'Mail (mutt)' -n 'Mail' -name muttTerm -e mutt 
</PRE>
Xterm works the same way.
<P>
(4) These are getting to be a lot of command-line options.  Make a 
wrapper script to use so you don't have to remember them all:
<P>
<PRE>
        mkdir ~/bin
        cat &gt;~/bin/muttTerm &lt;&lt;EOF
        #!/bin/sh
        rxvt -title 'Mail (mutt)' -n 'Mail' -name muttTerm -e mutt
        EOF
        chmod +x ~/bin/muttTerm
</PRE>
Now you can do the same thing as that really long command in [3] 
above using the simple:
<P>
<PRE>
        ~/bin/muttTerm
</PRE>
If you put ~/bin in your PATH, you can use the even simpler:
<P>
<PRE>
        muttTerm
</PRE>
(5) If you want to be sly, you can change the docked muttTerm to use
your new wrapper script instead of the really long command; then,
when you want to change anything in the really long command
except for the instance name, you can just change the wrapper
script, and it's done.  Here's the procedure: 
<P>
(a) [RightButtonDown] on the muttTerm dock tile
<P>
(b) Choose `Settings...'
<P>
(c) Replace the text in the `Application path and arguments'
field with the following: 
<P>
<PRE>
              muttTerm
</PRE>
(d) Choose `OK'
<P>
Note that Window Maker needs to know that ~/bin is on your PATH
for this to work; you may need to exit your X session and start
it again.
<P>
To change the instance name of the terminal window (e.g., from 
`muttTerm' to `mailTerm' or `blah' or
`terminalWindowRunningMutt'), you need to do the following 
<P>
(e) Change your muttTerm script 
(f) Undock your old muttTerm
<P>
(g) Run your muttTerm script
<P>
(h) Dock the resulting terminal window
<P>
(i) Do the stuff in [a] through [d] above again.
<P>
Good luck.
<P>
<P>
Thanks to Jim Knoble for this answer.
<A name="108"></A>
<h2>5.13 &nbsp;How do I get an appicon for Netscape?</h2>
If you are not using one of the latest Navigators, you can
<P>
1) Right click on the title bar
<P>
2) Click ``Attributes''
<P>
3) Select ``Advanced Options'' from the pull down menu
<P>
4) Select ``Emulate Application Icon''
<P>
5) Click Save
<P>
and older netscapes should now produce an application icon.
<P>
If you are using a newer rpm from Redhat Linux,  try running
<PRE>
	grep irix `which netscape`
</PRE>
This seems to have been introduced in their 4.7 update.  Comment out 
irix-session management restart netscape.  Alternatively, you may run
either
<P>
<PRE>
	/usr/lib/netscape/netscape-communicator
</PRE>
or
<PRE>
	/usr/lib/netscape/netscape-navigator
</PRE>
depending on which rpms you have installed.
<A name="109"></A>
<h2>5.14 &nbsp;How can I dock an application from a remote machine using ssh?</h2>
This answer asumes that you have already set up RSA
authentication using ``ssh-keygen''.  To be able to launch applications
<PRE>
 without being prompted for the password, you can use ``ssh-agent'' and
</PRE>
``ssh-add'' as follows.
<P>
With the addition to ~/.xsession of
<P>
<PRE>
	eval `ssh-agent`
	ssh-add /dev/null
</PRE>
just before
<PRE>
	exec wmaker
</PRE>
Then ssh will no longer prompt for the RSA-key passphrase.
The ``/dev/null'' argument to ``ssh-add'' causes it to use the 
``ssh-askpass'' graphical dialog.
<P>
The following procedure shows how to dock a remote xterm 
using ``ssh''.  This procedure should work well for any well-behaved
X11 application, including most Dock applets.
<P>
1) From a terminal window, start an ssh session with ``xterm'' as the command:
<P>
<PRE>
        ssh -a -C -X remote.example.net "xterm -name blah"
</PRE>
(The '-a' switch turns off agent forwarding, for security reasins and
the '-X' switch turns on X11 forwarding, required for the remote xterm
to run. The -C option turns on compression, very
useful for things such as X)
<P>
2) When the remote xterm appears, find the appicon.  If it's not
already in the Clip, drag it there.
<P>
3) [RightButtonDown] on the appicon and choose 'Settings...' from
the menu.  Note that the 'Application path and arguments' field
contains only:
<P>
<PRE>
        xterm -name blah
</PRE>
Change that to:
<P>
<PRE>
        ssh -a -C -X remote.example.net "xterm -name blah"
</PRE>
The backslashes and double quotes are critical.  Change the
contents of 'Command for files dropped with DND' in the same
fashion, putting '%d' inside the double quotes.
<P>
If you wish, change the icon so that you can recognize the tile
easily.  Press 'OK'.
<P>
4) [RightButtonDown] on the appicon again and choose 'Keep Icon(s)'.
<P>
5) Exit the remote xterm.  The new Clip tile should remain, with the
three dots at the lower lefthand corner to indicate the app is no
longer running.
<P>
6) [DoubleClick] on the new Clip tile.  You should get the remote
xterm again after a short while, depending on the speed of your
network and of the remote machine.
<P>
<P>
7) You may either leave the remote application in the Clip, or drag it
to the Dock.
<P>
[NOTE:  You should be wary of docking
something like ``wminet'' or ``wmnet'' in the manner, since you may
create a feedback loop by causing additional network traffic, which
the program monitors, causing yet more network traffic... ]
<A name="110"></A>
<h2>5.15 &nbsp;How do you make an omnipresent window not take focus whenever switching workspaces?</h2>
Typically, on applications like xmms, they are set to omnipresent so
they will appear on every workspace. This causes the app to often get
the focus unintentionally when switching workspaces.
<P>
To remedy this,
<P>
1) Bring up the ``Attributes'' menu.  You can do this by [Right Clicking]
on the title bar and seletcing ``Attributes''.  Alternatively, you may
hit 'Control+ESC' at the same time to bring up the title bar menu on apps
that do not have a title bar.
<P>
2) In the ``Window Attributes'' menu, select ``Skip Window List''
<P>
3) Push ``Save'' and then hit the close dialog window icon in the upper
right corner of the window frame.
<P>
Now the window will not take focus when switching workspaces.
<P>
[NOTE: this will also make the window non-focusable via keyboard window
switching. The only way to shift focus to the window is via the mouse. ]<BR>
</BLOCKQUOTE>
<?
}
if ($chapter==6) {
?>
<h1>Chapter 6: Themes and Dockapps</h1>
<UL type="disc">
<B>6.1</B> &nbsp; <A href="#121">What exactly are themes?</A><BR>
<B>6.2</B> &nbsp; <A href="#122">How do I install a theme?</A><BR>
<B>6.3</B> &nbsp; <A href="#123">Why do my themes not load the background?</A><BR>
<B>6.4</B> &nbsp; <A href="#124">How do I make a Theme?</A><BR>
<B>6.5</B> &nbsp; <A href="#125">I untarred a theme in ~/GNUstep/Library/WindowMaker like the README says,but it doesnt show up in the menu!</A><BR>
</UL>
</FONT>
<BR><HR><BR>
<A name="121"></A>
<h2>6.1 &nbsp;What exactly are themes?</h2>
Themes are a great aspect of Window Maker allowing a user to simply
save the entire 'look' of their desktop in an archive to distribute freely among
friends, fellow users and/or the whole net in general. :)
<P>
See the <a href="themepacks.php">theme-HOWTO</a> for an in-depth walk-through on making a Theme archive.
<A name="122"></A>
<h2>6.2 &nbsp;How do I install a theme?</h2>
This should be as simple as untarring the Theme.tar.gz into one of two
places. You can untar it to the global /usr/local/share/WindowMaker/* directory,
and have it be accessable to all users, or you can untar it to your own
~/GNUstep/Library/WindowMaker/ directory for your own personal use.
<P>
Use your favorite variation of the following:
<P>
<PRE>
	gzip -dc "Theme.tar.gz" | tar xvf -
</PRE>
Note that directory may differ on different systems
<A name="123"></A>
<h2>6.3 &nbsp;Why do my themes not load the background?</h2>
Likely you have not compiled Window Maker with support for the background
image format, usually JPEG.
<P>
You can check this by the following command
<P>
<PRE>
	ldd `which wmaker`
</PRE>
If libjpeg is not listed, you will need to install libjpeg that is available
from ftp.windowmaker.org
<A name="124"></A>
<h2>6.4 &nbsp;How do I make a Theme?</h2>
Please see the <a href="themepacks.php">theme-HOWTO</a> for details on making both new and old style themes (and the differences
between the two), here is a short summary on making old style themes.
Also, read the README.themes file included with the Window Maker
distribution in the WindowMaker/ directory.
<P>
In this walk-through when I use WindowMaker/, it can refer to the global
/usr/local/share/WindowMaker/ directory or the users own
~/GNUstep/Library/WindowMaker/ directory. 
<P>
To make a Theme.tar.gz, these are the steps I take:
<P>
1. Optionally create a README for your theme in WindowMaker/, call it
something like "ThemeName.txt"
<P>
2. Use the following command to add the Theme files to your .tar file.
<P>
<PRE>
     tar cvf ThemeName.tar ThemeName.txt Themes/ThemeName 
     Backgrounds/ThemeNameBG.jpg Backgrounds/ThemeNameTile.xpm
</PRE>
You can add as many more images as you need from the
appropriate directories under WindowMaker/ following that general
idea. You can even optionally add an IconSets/ThemeName.iconset and
it's associated icons to your theme in the same manner. This should
be stated in your README if you decide to include these.
<P>
3. Then gzip your .tar file to make your ThemeName.tar.gz file with this command:
<P>
<PRE>
    gzip -9 ThemeName.tar
</PRE>
4. Now give it to your friends!
<A name="125"></A>
<h2>6.5 &nbsp;I untarred a theme in ~/GNUstep/Library/WindowMaker like the README says,but it doesnt show up in the menu!</h2>
Make sure the OPEN_MENU command for the Themes entry in your menu has
the path for your personal themes directory included in it. To be sure,
add
<PRE>
	#define USER_THEMES_DIR ~/GNUstep/Library/WindowMaker/Themes
</PRE>
in your wmmacros file.<BR>
</BLOCKQUOTE>
<?
}
if ($chapter==7) {
?>
<h1>Chapter 7: Miscellaneous Questions</h1>
<UL type="disc">
<B>7.1</B> &nbsp; <A href="#115">Is there a pager for Window Maker?</A><BR>
<B>7.2</B> &nbsp; <A href="#116">How do I use getstyle and setstyle?</A><BR>
<B>7.3</B> &nbsp; <A href="#117">Why was libPropList removed from the distribution?</A><BR>
<B>7.4</B> &nbsp; <A href="#118">Why don't you distribute normal diff or xdelta patches?</A><BR>
<B>7.5</B> &nbsp; <A href="#119">Will you add GNOME or KDE support?</A><BR>
<B>7.6</B> &nbsp; <A href="#120">How can I produce a backtrace when Window Maker keeps crashing?</A><BR>
</UL>
</FONT>
<BR><HR><BR>
<A name="115"></A>
<h2>7.1 &nbsp;Is there a pager for Window Maker?</h2>
Not at the moment because there is not a pressing need for a pager. The concept of
multiple desktops does exist and there are currently 3 ways to switch between them.
<P>
First, the Meta+Number combination will switch between desktops. The Workspaces
menu will also let you switch workspaces. Lastly, the clip will also scroll one through
workspaces.  For those that would like to send an application to a specific workspace, either drag
it to the edge of the desktop onto the next workspace, or right click on its
title bar, select 'Move To', and click the workspace you want it to be
moved to.
<P>
However, Window Maker does support KDE and GNOME protocols, including their
workspace management, so you may use their pager in conjunction with Window Maker 
in these.  Note that in order for this to work, you must enable support when
you configure Window Maker (using the --enable-kde and
--enable-gnome configure options).
<P>
Note also that the Blackbox pager application will work with
Window Maker.
<A name="116"></A>
<h2>7.2 &nbsp;How do I use getstyle and setstyle?</h2>
To capture the current Window Maker style, use the command
<P>
<PRE>
	getstyle > current.style
</PRE>
To replace the current style, use the command
<P>
<PRE>
	setstyle filename.style
</PRE>
<A name="117"></A>
<h2>7.3 &nbsp;Why was libPropList removed from the distribution?</h2>
Alfredo Kojima writes:
<P>
libPropList was removed from Window Maker because other programs also
use it, such as GNOME.  If libPropList is distributed with wmaker, it
would cause problems with whatever version of libPropList you already 
had installed.
<P>
Now, there is no more GNOME libproplist and Window Maker libproplist. There is 
only libPropList which is worked on as a single community effort.
<A name="118"></A>
<h2>7.4 &nbsp;Why don't you distribute normal diff or xdelta patches?</h2>
Whenever possible, plain diff patches are distributed. If the new version has
new binary files, normal diff won't be able to handle them, so a patch
package is distributed instead. We don't use xdelta because a) most
systems do not have xdelta installed and b) xdelta is picky and requires
the files to be patched to be exactly the same as the one used to make the patch. 
The patch package scheme used is much more flexible.
<P>
We do not distribute a simple diff with the binary files separately (and
variations, like uuencoding the binary files) because a) it is more
complicated and error prone to require the user to manually move the files
to the correct places  b) the current patch package scheme <I>does</I> distribute
the binary files and diff files separately. If the user wants to install
everything by hand, nobody will object to that and c) sooner or later someone will
certainly ask for a script to automate the file moving stuff.
<P>
So we hacked a script (mkpatch) that automatically creates a patch
package with the normal text diff file, a list of removed files and the binary
files that have changed or been added, plus a script that does the patching
automatically. If you don't like the script, you can apply the patch and move
the files manually. Or download the whole distribution.
<A name="119"></A>
<h2>7.5 &nbsp;Will you add GNOME or KDE support?</h2>
Support for GNOME and KDE hints has been included since 0.50.0.
<P>
Note that you must enable this support at compile time with the proper
arguments to configure (--enable-kde and --enable-gnome).
<A name="120"></A>
<h2>7.6 &nbsp;How can I produce a backtrace when Window Maker keeps crashing?</h2>
Thanks to Paul Seelig for this answer:
<P>
You can use the GNU debugger "gdb" to get exact information about how
and where wmaker crashed.  Sending this information to the developers
is the most convenient way to help in debugging.
<P>
The wmaker binary needs to be compiled with debugging turned on
("./configure --with-debug etc.") for this to work.
<P>
Exit wmaker and start a failsafe X session with an open xterm.
<P>
First type the command "script" to log the following session into a
file commonly called "~/typescript".  Then enter "gdb wmaker" at the
shellprompt:
<P>
<PRE>
	[shell prompt]~ > script
	Script started, output file is typescript
	[shell prompt]~ > gdb wmaker
	GNU gdb 4.17.m68k.objc.threads.hwwp.fpu.gnat
	Copyright 1998 Free Software Foundation, Inc.
	GDB is free software, covered by the GNU General Public License, and you are
	welcome to change it and/or distribute copies of it under certain conditions.
	Type "show copying" to see the conditions.
	There is absolutely no warranty for GDB.  Type "show warranty" for details.
	This GDB was configured as "i486-pc-linux-gnu"...
	(gdb)
</PRE>
At the gdb prompt simply type "run" to start the WMaker session:
<P>
<PRE>
	(gdb) run
	Starting program: /usr/bin/X11/wmaker
</PRE>
Try to reproduce the error which has provoked the crash before and if
you succeed the session will simply freeze and you will see something
similiar to following prompt:
<P>
<PRE>
	Program received signal SIGSEGV, Segmentation fault.
	0x809ea0c in WMGetFirstInBag (bag=0x0, item=0x811e170) at bag.c:84
	84	    for (i = 0; i &lt; bag-&gt;count; i++) {
	(gdb)
</PRE>
Now you just type "bt" for "backtrace" and gdb will show you where the
crash happened:
<P>
<PRE>
	(gdb) bt
	#0  0x809ea0c in WMGetFirstInBag (bag=0x0, item=0x811e170) at bag.c:84
	#1  0x807c542 in wSessionSaveState (scr=0x80c28e8) at session.c:299
	#2  0x807bd88 in wScreenSaveState (scr=0x80c28e8) at screen.c:1089
	#3  0x807cf54 in Shutdown (mode=WSExitMode) at shutdown.c:111
	#4  0x8078101 in exitCommand (menu=0x80f7230, entry=0x80fdb38)
	    at rootmenu.c:193
	#5  0x8078403 in wRootMenuPerformShortcut (event=0xbffff360) at rootmenu.c:401
	#6  0x80630f7 in handleKeyPress (event=0xbffff360) at event.c:1492
	#7  0x8061c86 in DispatchEvent (event=0xbffff360) at event.c:232
	#8  0x8093092 in WMHandleEvent (event=0xbffff360) at wevent.c:585
	#9  0x8061dae in EventLoop () at event.c:322
	#10 0x806b238 in main (argc=1, argv=0xbffff404) at main.c:594
	(gdb)
</PRE>
To quit the debugger just type "quit" and say "y":
<P>
<PRE>
	(gdb) quit
	The program is running.  Exit anyway? (y or n) y
	[shell prompt]~ >
</PRE>
To quit the script session type "exit" again:
<P>
<PRE>
	[shell prompt]~ > exit
	exit
	Script done, output file is typescript
	[shell prompt]~ >
</PRE>
Send the resulting "~/typescript" together with a concise explanation
about how to reproduce the bug (please use the included BUGFORM for
instruction) to the <A HREF="http://www.windowmaker.org/feedback.php?mailto=developers">developers</A>.<BR>
</BLOCKQUOTE>
<?
}
if ($chapter==8) {
?>
<h1>Chapter 8: Troubleshooting Tips</h1>
No questions are currently available for this chapter.</FONT>

<?
}
if ($chapter==9) {
?>
<h1>Chapter 9: Programming for Window Maker</h1>
<UL type="disc">
<B>9.1</B> &nbsp; <A href="#111">How do I get a normal X application to produce an appicon?</A><BR>
<B>9.2</B> &nbsp; <A href="#112">How do I get my tcl/tk application to produce an appicon?</A><BR>
<B>9.3</B> &nbsp; <A href="#113">What is WINGs?</A><BR>
<B>9.4</B> &nbsp; <A href="#114">Where can I get more information about WINGs?</A><BR>
</UL>
</FONT>
<BR><HR><BR>
<A name="111"></A>
<h2>9.1 &nbsp;How do I get a normal X application to produce an appicon?</h2>
Another insightful answer from who else but Dan Pascu.
<P>
"You must define the WM_CLASS (XSetClassHint()) and the CLIENT_LEADER or
XWMHints.window_group properties, which are automatically set by most
applications that use Xt (Motif, Athena ...), but if you use plain Xlib you must set them
by hand.
<P>
Also you must make a call to XSetCommand(dpy, leader, argv, argc);
<P>
Take a look at WindowMaker-0.12.3/test/test.c that is an example for writing such an
app (which also have an app menu).
<A name="112"></A>
<h2>9.2 &nbsp;How do I get my tcl/tk application to produce an appicon?</h2>
Oliver Graf writes:
<P>
The main window (normally this is called '.' [dot] in tk) should use the following lines:
<P>
<PRE>
	wm command . [concat $argv0 $argv]
	wm group . .
</PRE>
All child windows attached to the same app-icon should use:
<P>
<PRE>
	toplevel .child
	wm group .child .
</PRE>
where .child should be replaced by the actual window path.
<P>
Replace '.' with the actual main-window path and 'wm group .child .' should be
added for each 'toplevel .child' call.
<A name="113"></A>
<h2>9.3 &nbsp;What is WINGs?</h2>
WINGs Is Not GNUstep. ;)
<P>
It is the widget library written for the widgets in Window Maker.<P>
It is currently under heavy development but several people have started
writing applications in it.  Its goal is to emulate the NeXT(tm)-style widgets.
<P>
<A HREF="http://www.ozemail.com.au/~crn/wm/wings.html">http://www.ozemail.com.au/~crn/wm/wings.html</A> is the closest thing to an
information center about WINGs.  You can find out more information in our
<A href="http://www.windowmaker.org/development.php?show=wings">WINGs development</A> section.
<A name="114"></A>
<h2>9.4 &nbsp;Where can I get more information about WINGs?</h2>
Nic Berstein has created a WINGs development list.
<P>
The purpose of this list is to provide a forum for support, ideas,
suggestions, bug reports etc. for the WINGs widget set library.
<P>
To subscribe to this list, send a message with the word ``subscribe'' in
the _BODY_ of the message to: <A HREF="mailto:wings-request@postilion.org">&lt;wings-request@postilion.org&gt;</A>.<BR>
</BLOCKQUOTE>

<?
}
if ($chapter > 0) {
?>
<HR size="1" noshade width="100%" align="center">
<DIV align="center">
<A href="FAQ.php">Top level of FAQ</A>
</DIV>
<?
}
?>




        </td>
      </tr>
    </table>



  <?php include("footer.php"); ?>

</body>
</html>
