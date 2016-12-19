%define name	wmswallow
%define ver	0.6
%define rel	1
%define copy	GPL
%define ich friedel <friedel@burse.uni-hamburg.de>
%define group	X11/WindowMaker Applets
%define realname %{name}
Summary:	wmswallow is a WindowMaker dock applet to make any application dockable in the WindowMaker dock
Name:		%{name}
Version:	%{ver}
Release:	%{rel}
Copyright:	%{copy}
Packager: %{ich}
URL: http://burse.uni-hamburg.de/~friedel/software/wmswallow.html
Group:	%{group}
Source: http://burse.uni-hamburg.de/~friedel/software/wmswallow/wmswallow.tar.Z
BuildRoot: /var/tmp/%{name}-root
#Patch: %{name}-%{ver}.patch
%description
-    wmswallow was mainly created to swallow coolmail in the WindowMaker dock, but it can
     swallow about any X-window you conceive. 
-    The window may receive mouseclicks, 
-    for windows that do not get mouseclicks (like xload or xeyes), you can specify a
     shell-command to execute on a click. 
-    The geometry for the swallowed app can be specified. Only HEIGHTxWIDTH are used,
     however.  
-    Applications can be started on wmswallows commandline. An already running window
     can be swallowed, either by name, class or directly by window-id 
-    The window may receive keyboard-focus, if you specify the "-focus" switch. 
-    Even windows (like xteddy or wine) that hide from the windowmanager can be
     swallowed with the -unmanaged switch 
%changelog

%prep

%setup -n %{realname}

#%patch -p1

%build
make xfree

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/local/bin
cp wmswallow $RPM_BUILD_ROOT/usr/local/bin

%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root)
%doc CHANGELOG README todo INSTALL README.solaris LICENCE
/usr/local/bin/wmswallow

