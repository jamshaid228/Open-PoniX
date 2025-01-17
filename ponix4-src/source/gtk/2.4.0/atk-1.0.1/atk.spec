Name:		atk
Summary:	Accessability Toolkit
Version:	1.0.1
Release:	1
License:	LGPL
Group:		Development/Libraries
Source:		ftp://ftp.gimp.org/pub/gtk/v1.3/%{name}-%{version}.tar.gz
BuildRoot:	/var/tmp/%{name}-%{version}-root
URL:		http://www.gtk.org
Requires:	glib2 >= 2.0.0
BuildRequires:	glib2-devel >= 2.0.0

%description
Handy library of accessability functions. Development libs and headers
are in atk-devel.

%package devel
Summary:	Header, docs and development libraries for atk.
Group:		Development/Libraries
Requires:	%{name} = %{version}

%description devel
Header, docs and development libraries for atk.

%prep
%setup -q

%build
CFLAGS="$RPM_OPT_FLAGS"
./configure --prefix=%{_prefix} \
    --bindir=%{_bindir} --mandir=%{_mandir} \
    --localstatedir=%{_localstatedir} --libdir=%{_libdir} \
    --datadir=%{_datadir} --includedir=%{_includedir} \
    --sysconfdir=%{_sysconfdir} --disable-gtk-doc
make

%install
rm -rf $RPM_BUILD_ROOT

make prefix=$RPM_BUILD_ROOT%{_prefix} bindir=$RPM_BUILD_ROOT%{_bindir} \
    mandir=$RPM_BUILD_ROOT%{_mandir} libdir=$RPM_BUILD_ROOT%{_libdir} \
    localstatedir=$RPM_BUILD_ROOT%{_localstatedir} \
    datadir=$RPM_BUILD_ROOT%{_datadir} \
    includedir=$RPM_BUILD_ROOT%{_includedir} \
    sysconfdir=$RPM_BUILD_ROOT%{_sysconfdir} install

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-, root, root)

%doc AUTHORS COPYING ChangeLog NEWS README
%{_libdir}/lib*.so.*

%files devel
%defattr(-, root, root)

%{_libdir}/lib*.so
%{_libdir}/*a
%{_libdir}/pkgconfig/*.pc
%{_includedir}/atk-1.0
%{_datadir}/gtk-doc/html/atk

%changelog
* Mon Aug 27 2001 Jens Finke <jens@gnome.org>
- glib2 package now required
- updated source url

* Wed Aug 15 2001 Jens Finke <jens@gnome.org>
- created spec file
