Buildroot: /tmp/var-wmradio-root
Copyright: GPL
ExclusiveOS: Linux
Group: Applications/Multimedia
Name: wmradio
Version: 0.9
Packager: tomas.cermak(at)email.cz
Source: wmradio-%{version}.tgz
Summary: Dockable FM radio
URL: http://gogo.aquasoft.cz/~cermak/wmradio.html
Release: 1

%description
wmradio is FM radio card applet for WindowMaker

%prep
%setup

%build
./configure --disable-libxosd --disable-gnome --prefix=/usr
make

%install
make install
#make ginstall
make install-skins
make install-skins

%clean
rm -rf $RPM_BUILD_ROOT

%files
/usr/bin/wmradio
/usr/bin/xwmradio
/usr/bin/wmradio-remote
/usr/bin/wmradio-config.py
%doc /usr/man/man1/wmradio.1.gz
%dir /usr/lib/wmradio
/usr/lib/wmradio/*
/usr/share/applications/wmradio-config.desktop
/usr/share/applications/wmradio.desktop
/usr/share/pixmaps/wmradio.png

