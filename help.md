---
layout: page
title: help
permalink: help
---

# What is a dockapp?

Dockapps are small applications that run inside a 64x64 icon, often
in [Window Maker](http://windowmaker.org)'s dock or
[AfterStep](http://afterstep.org)'s wharf.  They are useful for viewing the
time, obtaining weather reports, receiving email alerts, monitoring the system,
and other basic tasks.

# What is dockapps.net?

Our goal is twofold: to serve as a comprehensive directory of all available
dockapps and also to host dockapps which are maintained by the Window Maker
Team.

# How can I help?

If you would like to add or update a dockapp, fork
[https://github.com/d-torrance/dockapps](https://github.com/d-torrance/dockapps)
and submit a pull request to the `gh-pages` branch.

The site is built using [Jekyll](http://jekyllrb.com).  Each dockapp is
maintained in an individual file.  There are two main types of dockapp: hosted
and non-hosted.

Consider a hosted dockapp "wmfoo".  The file `_posts/1970-01-01-wmfoo.md` might
like the the following.

    ---
    layout: dockapp
    title: wmfoo
    permalink: wmfoo
    hosted: 1
    website: http://wmfoo.org
    images:
     - wmfoo1.png
     - wmfoo2.png
    versions:
     -
      number: 0.1
      download: wmfoo-0.1.tar.gz
     -
      number: 0.2
      download: wmfoo-0.2.tar.gz
    categories: baz
    ---
    Description of wmfoo using Markdown syntax.

The image files are added to the `img` directory and the tarballs to the
`download` directory.


## My dockapp will be hosted on dockapps.net

## My dockapp will be hosted elsewhere

# How do I add a category?
