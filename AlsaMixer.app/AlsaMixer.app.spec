%define prefix /usr/local

Summary: dockable mixer for ALSA driver and Window Maker
Name: AlsaMixer.app
Version: 0.1
Release: 1
Source0: %{name}-%{version}.tar.gz
License: GPL
Group: X11/Utilities
BuildRoot: %{_tmppath}/%{name}-root
Packager: Petr Hlavka <xhlavk00@stud.fit.vutbr.cz>
Requires: XFree86-devel, libXpm-devel
URL: http://www.stud.fit.vutbr.cz/~xhlavk00/AlsaMixer.app/

%description
AlsaMixer.app is simple dockable mixer application for WindowMaker. It can
control up to three volume sources, load/store mixer settings, run external
application on middle click (i.e. more sophisticated mixer)

%prep
%setup -q

%build
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p -m 0755 $RPM_BUILD_ROOT%{prefix}/bin
cp -f AlsaMixer.app $RPM_BUILD_ROOT%{prefix}/bin


%clean
if [ -d $RPM_BUILD_ROOT ]; then rm -rf $RPM_BUILD_ROOT; fi
if [ -d $RPM_BUILD_DIR/%{name}-%{version} ]; then rm -rf $RPM_BUILD_DIR/%{name}-%{version}; fi

%files
%defattr(-,root,root)
%attr(0755,root,root) %{prefix}/bin/AlsaMixer.app

%changelog
* Tue Sep 30 2004 Petr Hlavka
- initial release
