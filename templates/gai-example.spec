%define name gai-example
%define version 0.1
%define release 1

Summary: GAI Example Applet
Name: %{name}
Version: %{version}
Release: %{release}
Source0: %{name}-%{version}.tar.bz2
License: GPL
Group: Development/Example
BuildRoot: %{_tmppath}/%{name}-buildroot
Prefix: %{_prefix}
BuildRequires: gai-devel
Requires: gai

%description
  This is the gai applet example package.

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
