Summary: The Enlightenment window manager, DR16.
Name: e16
Version: 1.0.11
Release: 1%{?_rpm_pkg_ext:%{_rpm_pkg_ext}}%{?_vendorsuffix:.%{_vendorsuffix}}
License: BSD
Group: User Interface/Desktops
URL: http://www.enlightenment.org/
Source: http://prdownloads.sourceforge.net/enlightenment/%{name}-%{version}.tar.gz
Packager: %{?_packager:%{_packager}}%{!?_packager:Michael Jennings <mej@eterm.org>}
Vendor: %{?_vendorinfo:%{_vendorinfo}}%{!?_vendorinfo:The Enlightenment Project (http://www.enlightenment.org/)}
Distribution: %{?_distribution:%{_distribution}}%{!?_distribution:%{_vendor}}
Prefix: %{_prefix}
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: imlib2-devel freetype-devel
#BuildSuggests: esound-devel
Obsoletes: enlightenment < 0.16.8
Requires: imlib2 >= 1.2.0
Provides: e16-edox = 1.0.11

%description
Enlightenment is a window manager for the X Window System that
is designed to be powerful, extensible, configurable and
pretty darned good looking! It is one of the more graphically
intense window managers.

Enlightenment goes beyond managing windows by providing a useful
and appealing graphical shell from which to work. It is open
in design and instead of dictating a policy, allows the user to 
define their own policy, down to every last detail.

This package will install the Enlightenment window manager.

%prep
%setup -q

%build
%{configure} %{?acflags}
%{__make} %{?_smp_mflags} %{?mflags}

%install
rm -rf $RPM_BUILD_ROOT
%{__make} install DESTDIR=$RPM_BUILD_ROOT %{?mflags_install}
rm -rf $RPM_BUILD_ROOT/usr/share/doc
mkdir -p $RPM_BUILD_ROOT/etc/X11/xinit/Xclients.d
cp $RPM_BUILD_ROOT/usr/share/e16/misc/Xclients.* $RPM_BUILD_ROOT/etc/X11/xinit/Xclients.d/

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)
%doc AUTHORS COPYING ChangeLog docs/e16.html
%{_bindir}/*
%{_libdir}/%{name}
%{_datadir}/%{name}
%{_datadir}/locale/*/*/*
%{_datadir}/applications/*
%{_datadir}/xsessions/*
%{_datadir}/gnome-session/sessions/*
%{_sysconfdir}/X11/xinit/Xclients.d/*
%{_mandir}/*/*

%changelog
