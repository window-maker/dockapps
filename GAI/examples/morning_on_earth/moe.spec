%define name moe
%define version 0.3
%define release 1mdk

Summary: Morning on earth
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
  It displays a rotating earth and can start a program if you
  click at it.
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
