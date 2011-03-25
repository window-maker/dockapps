%define	name			wmpower
%define	version			0.3.1
%define	release			1


Summary:	wmpower is a dockapp to see the power management of a laptop


Name:		%{name}
Version:	%{version}
Release:	%{release}
Copyright:	GPL
Group:		Applications/System
Vendor:		Francisco Rodrigo Escobedo Robles (frer@pepix.net)
Url:		http://wmpower.sourceforge.net/
Source:		%{name}-%{version}.tar.bz2
Provides:	wmpower
BuildRoot:	/var/tmp/%{name}-%{version}-%{release}


%description
wmpower is a Window Maker dock application allowing the user to
graphically see (and set) the power management status of his laptop.
  

%prep
%setup -q


%build
./configure --exec-prefix=/usr
make


%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/bin
make DESTDIR=$RPM_BUILD_ROOT install
strip $RPM_BUILD_ROOT/usr/bin/wmpower


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root)
%doc AUTHORS BUGS ChangeLog COPYING INSTALL NEWS README README.compal THANKS TODO
%attr(7755,root,root)/usr/bin/wmpower


%changelog
