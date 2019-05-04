Window Maker webpage source
===========================

This is a web page source files. All of the files should be proper markdown
files accepted by [Jekyll](https://jekyllrb.com) static site generator.

Build/serve
-----------

In order to build the site, you'll need Jekyll framework installed and
[jekyll-rst](https://github.com/gryf/jekyll-rst) plugin. Easiest way
to achieve it, is to install it from system repositories.

If your distribution doesn't contain it (even in external ones, like PPA for
Ubuntu, AUR for Arch or some portage overlay from Gentoo), that it might be
installed locally using [Bundler](https://github.com/bundler/bundler), which
typical usage would be as follows:

```
$ cd window-maker.github.io && bundler init
$ bundler add jekyll
$ mkdir _plugins
$ git clone https://github.com/gryf/jekyll-rst _plugins/jekyll-rst
$ pip install docutils pygments
$ gem install RbST nokogiri
$ bundler exec jekyll serve
```

Consult [jekyll-rst](https://github.com/gryf/jekyll-rst) plugin documentation
for requirements. Other options for installing dependencies are also possible -
they might be installed from distribution repositories.

Last line will initialize gemfile, add jekyll to it, and then perform `jekyll
serve` which underneath will build the site and than run simple http server on
`http://localhost:4000` in development mode. More about jekyll you can find [on
it's page](https://jekyllrb.com/docs)
