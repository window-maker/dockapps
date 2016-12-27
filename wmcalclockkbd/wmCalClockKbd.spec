%define prefix /usr/local

Summary: dockable time, calendar and keyboard applet for  Window Maker
Name: wmCalClockKbd
Version: 0.1a
Release: 1
Source0: %{name}-%{version}.tar.gz
License: GPL
Group: X11/Utilities
BuildRoot: %{_tmppath}/%{name}-root
Packager: Petr Hlavka <xhlavk00@stud.fit.vutbr.cz>
Requires: XFree86 >= 4.3
URL: http://www.stud.fit.vutbr.cz/~xhlavk00/wmCalClockKbd/

%description
wmCalClockKbd has all features of wmCalClock and add very simple actual
keyboard group identificator (for XFree86 >= 4.3.0).
In another way, it's dockable application for WindowMaker (but you can also
use it in AfterStep, BlackBox, ...) window manager. wmCalClockKbd can show
actual time, date and keyboard group.

%prep
%setup -q

%build
echo "%{prefix}" > PREFIX
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p -m 0755 $RPM_BUILD_ROOT%{prefix}/bin
cp -f src/wmCalClockKbd $RPM_BUILD_ROOT%{prefix}/bin
mkdir -p -m 0755 $RPM_BUILD_ROOT%{prefix}/share/man/man1
cp -f src/wmCalClockKbd.1 $RPM_BUILD_ROOT%{prefix}/share/man/man1/
mkdir -p -m 0755 $RPM_BUILD_ROOT%{prefix}/share/%{name}
cp -f pixmaps/*.xpm $RPM_BUILD_ROOT%{prefix}/share/%{name}/
cp -f pixmaps/COPYING $RPM_BUILD_ROOT%{prefix}/share/%{name}/

%clean
if [ -d $RPM_BUILD_ROOT ]; then rm -rf $RPM_BUILD_ROOT; fi
if [ -d $RPM_BUILD_DIR/%{name}-%{version} ]; then rm -rf $RPM_BUILD_DIR/%{name}-%{version}; fi

%files
%defattr(-,root,root)
%attr(0755,root,root) %{prefix}/bin/wmCalClockKbd
%doc ChangeLog COPYING README AUTHORS NEWS INSTALL BUGS TODO
%{prefix}/share/%{name}
%{prefix}/share/%{name}/*.xpm
%{prefix}/share/%{name}/COPYING
%{prefix}/share/man/man1/wmCalClockKbd.1

%changelog
* Thu Nov 04 2004 Petr Hlavka
- added Russian and French flags

* Fri Sep 12 2003 Petr Hlavka
- initial release
