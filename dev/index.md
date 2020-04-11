---
layout: default
title: Development
---

Development
===========

Here are some pieces of information regarding development in Window Maker.

Source code versioning system
-----------------------------

The source code for Window Maker is contained in a [git](http://git-scm.com/)
repository located [here](http://repo.or.cz/w/wmaker-crm.git).  To obtain a
full-fledged copy of the repository do this:

    git clone git://repo.or.cz/wmaker-crm.git

There are two main branches in the repository, called 'master' and 'next'. The
purpose of the 'next' branch is to add and extra layer of testing before the
patches hit the 'master' branch. It is rebased when needed. The 'master' branch
should ideally never be rebased -- if it is, run to the nearest
anti-nuclear bunker.

Submitting patches
------------------

The Window Maker source code follows the
[coding style of the linux kernel](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/Documentation/process/coding-style.rst).
Respect it when submitting patches.

If you are not familiar with git, take a look at the
[git homepage](http://git-scm.com/) -- it contains the kind of documentation you
need to get started. You should also read the file contained in the Window Maker
repository [The perfect Window Maker
patch](http://repo.or.cz/w/wmaker-crm.git/blob/HEAD:/The-perfect-Window-Maker-patch.txt)
which gives you further details about patches to Window Maker.

Patches not submitted according to the above guidelines will not be accepted.

Last but not least, patches doing code cleanups are **STRONGLY** encouraged.

Git repository for dockapps
---------------------------

There is also a [git repository](http://repo.or.cz/w/dockapps.git) containing a
few dockapps which apparently have no maintainers anymore. Patches for those
dockapps (or to include more apps) can also be sent to
<wmaker-dev@googlegroups.com>.

Some sources of information
---------------------------

* [The Window Maker WINGs library]({{ site.baseurl }}/docs/wings.html).
* [The Xlib Manual](http://tronche.com/gui/x/xlib/)
