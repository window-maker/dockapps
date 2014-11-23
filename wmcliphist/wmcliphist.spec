Summary: dockable clipboard history applet for Window Maker
Name: wmcliphist
Version: 1.0
Release: 1
Source0: %{name}-%{version}.tar.gz
License: GPL
Group: Applications/File
BuildRoot: %{_builddir}/%{name}-root
Packager: Daniel Tschan <tschan+rpm@devzone.ch>
URL: http://linux.nawebu.cz/wmcliphist/

%description
In short, it is clipboard history dockable application for Window Maker
(and maybe AfterStep with some little modifications - not tested).
wmcliphist keeps history of clipboard operations and allows you to put
previously copied items back to clipboard for pasting to other
applications. I wrote wmcliphist because there was no such application
suitable for usage in Window Maker and I was confused to run number of
KDE daemons for Klipper (which was the inspiration).

%prep
%setup -q

%build
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/bin
cp -f wmcliphist $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/share/%{name}
cp -f .wmcliphistrc $RPM_BUILD_ROOT/usr/share/%{name}/wmcliphistrc.sample

%clean
if [ -d $RPM_BUILD_ROOT ]; then rm -rf $RPM_BUILD_ROOT; fi
if [ -d $RPM_BUILD_DIR/%{name}-%{version} ]; then rm -rf $RPM_BUILD_DIR/%{name}-%{version}; fi

%files
%defattr(-,root,root)
%attr(0755,root,root) /usr/bin/wmcliphist
%doc ChangeLog COPYING README AUTHORS NEWS INSTALL
/usr/share/%{name}/wmcliphistrc.sample

%changelog
* Sun Aug 24 2003 Michal Krause
- exec actions can be performed automatically or on demand by middle mouse
  button click on menu item or by hot key for last captured item (Michael
  Beattie)
- wmcliphist can automatically take up clipboard content allowing to paste it
  when application who copied it ended already
* Mon Jun 23 2003 Michal Krause
- brand new icon set made by Daniel Richard G. <skunk@iskunk.org>
- icon size (60, 40, 30 or 16 pixels) selectable on command line
* Sun Apr 06 2003 Michal Krause
- first version containing spec for building RPMs (Daniel Tschan
  <tschan+rpm@devzone.ch>)
- fixed locale and zombie bugs (Victor Cheburkin <vc@iptcom.net>)
- wmcliphist now starts without .wmcliphistrc (Sebastian Ley
  <sebastian.ley@mmweg.rwth-aachen.de>)
- permissions of .wmcliphist.data are more secure now (Kresimir Kukulj
  <madmax@iskon.hr>)
- fixed buffer overrun bug in reading history (Simon 'corecode' Schubert
  <corecode@corecode.ath.cx>)
- keyboard navigation in history menu is possible now
- icons with antialiasing against dark, mid and light background are now
  compiled into wmcliphist and it can be choosen on command line (-i)
- wmcliphist can optionaly ask you to confirm performing of exec action
  (suggested by Ivica Letunic <Ivica.Letunic@EMBL-Heidelberg.DE>)
- added hotkey which returns previously captured item back to clipboard. It can
  safe some keystrokes or mouse clicks in some situation, escpecially if you
  need clear destination before pasting to it and selecting of its content
  replaces your data in the clipboard
