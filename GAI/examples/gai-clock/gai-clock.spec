%define name gai-clock
%define version 0.4
%define release 1mdk

Summary: GAI Digital Clock
Name: %{name}
Version: %{version}
Release: %{release}
Source0: %{name}-%{version}.tar.bz2
License: GPL
Group: Development/Example
BuildRoot: %{_tmppath}/%{name}-buildroot
Prefix: %{_prefix}
BuildRequires: gai
Requires: gai

%description
  This is an example applet on how to make applets with GAI.
  It is a simple digital clock.
  
%prep
%setup -q
sh configure --prefix=%_prefix

%build
make

%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=%buildroot

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
 /*

%changelog

# end of file
