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

The site is built using [Jekyll](http://jekyllrb.com).  Jekyll is a blogging
platform, and each dockapp is a "post".  Therefore, they are stored in the
`_posts` directory.  Note that each filename begins with "1970-01-01".   Date
prefixes are required for Jekyll posts (again, because it is a blogging
platform).  The particular date was an arbitrary choice (the date of the Unix
epoch), but each dockapp should use the same date so that they sort
alphabetically and not by date.

The files themselves consist of a [YAML](http://yaml.org/) front matter block
followed by a description using
[Markdown](https://daringfireball.net/projects/markdown/).

There are two types of dockapp:  hosted and non-hosted.

Hosted dockapps are maintained by the Window Maker Team.  Their source is under
version control in the
[Window Maker dockapps git repository](http://repo.or.cz/dockapps.git)
and release tarballs are hosted here at dockapps.net.

Non-hosted dockapps are maintained elsewhere.

Let's consider a hypothetical hosted dockapp "wmfoo".  We would have a file
`_posts/1970-01-01-wmfoo.md` containing:

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
    Description of wmfoo using Markdown.

The image files are added to the `img` directory and the tarballs to the
`download` directory.

Now consider a hypothetical non-hosted dockapp "wmbar".  The file
`_posts/1970-01-01-wmbar.md` contains:

    ---
    layout: dockapp
    title: wmbar
    permalink: wmbar
    hosted: 0
    website: http://wmbar.org
    images:
     - wmbar1.png
     - wmbar2.png
    categories: baz
    ---
    Description of wmbar using Markdown.

Note the two main differences.  Hosted dockapps have `hosted: 1` while
non-hosted dockapps have `hosted: 0`.  Also, non-hosted dockapps do not
have a `versions` sequence.

Note that some dockapps (e.g., [mixer.app](http://dockapps.net/mixerapp))
contain dots in their names.  In this case, we need to remove the dot from
the `permalink` field, e.g.,

    title: foo.app
    permalink: fooapp

Note that categories are also stored as Jekyll posts.  For example, the "baz"
category which wmfoo and wmbar both belong to corresponds to a file
`_posts/1970-01-01-baz.md` containing:

    ---
    layout: category
    title: baz
    permalink: category/baz
    ---

# What if I have a question or find a problem with the site?

Either [create an issue](https://github.com/d-torrance/dockapps/issues/new)
or [email the Window Maker developers](mailto:wmaker-dev@lists.windowmaker.org).

