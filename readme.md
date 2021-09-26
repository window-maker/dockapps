# dockapps

Dockapps are small tiles that easily fit into a dock in your window manager, but each one could be considered a complete configuration application, amusement, monitoring tool, or a combination of features.

Each tile is generally larger than a tray icon but smaller than an application, so they offer a way to do detailed operations while staying out of the way.

The instructions below compliment the README file in each app's subdirectory.

Generally, you should consult the individual README before running the commands in this file.

## Support
(issue reporting, pull request instructions, official links, etc.)

The issue tracker is at <https://github.com/window-maker/dockapps/issues>.
> However, this repository is just used for the issue tracker and hosting dockapps.net.
> The main code is hosted at https://repo.or.cz/dockapps, and patches should be submitted to wmaker-dev@googlegroups.com using git send-email.

-[@d-torrance](https://github.com/d-torrance)


## Compile

1. If there is no "configure" script but there are autotools input files such as "configure.ac", first run:
```bash
    autoreconf -fi
```
If there is no "configure" script but there is an autogen.sh or autogen file, run that instead:
```bash
    ./autogen.sh
    # or ./autogen
```


2. @OldCoder says that compiling old C programs requires a special configure command before make:
```bash
    CFLAGS="-fcommon" bash ./configure
```

@Poikilos says:
The special configure line above resolves multiple definition errors
such as:
```
/usr/bin/ld: wmgeneral.o:/home/x/git/dockapps/wmmp3/wmgeneral.h:44: multiple definition of `display'; main.o:/home/x/git/dockapps/wmmp3/wmgeneral.h:44: first defined here
```
where /home/x/git/dockapps is the location from which you are compiling.
- If you already tried `make` and it failed, you still must run
  `make clean` before trying `make` again so that the new configure
  line's fixes will take effect.

3.
```bash
    make
```

## Install

4. Generally, at this point (after doing the Compile steps; or after extracting a release source archive if it contains binaries) you can run:
```bash
    sudo make install
```
unless `make` had errors, or the app's readme has special install instructions.

5. If you are using Joe's Window Manager, you can make several
   dockapps part of a tray. You can make a tray tag directly inside of
   the JWM tag of ~/.jwmrc, such as via:
   ```XML
   <Tray layout="vertical" x="0" y="356" halign="left" width="69">
        <Swallow width="60" height="62" name="fishmon">wmfishtime</Swallow>
        <Swallow width="60" height="62" name="wmmp3">wmmp3</Swallow>
    </Tray>
```
   (where wmfishtime or wmmp3 is the executable in your system's PATH)
   - If you aren't sure how to configure Joe's Window Manager
     but you have jwm installed, you can use the
     default rc file as a template:
     ```bash
    if [ ! -f ~/.jwmrc ]; then
        cp /etc/jwm/system.jwmrc ~/.jwmrc
    else
        echo "$HOME/.jwmrc already exists."
    fi
```
