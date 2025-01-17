#%define prefix %{_prefix}
%define prefix /usr

%define ver 2.6.5
%define rel 1
%define c_p --without-gnome --without-rplay-library --sysconfdir=/etc
%define m_p CFLAGS="-O2"

# Different distributions expect sources to be in different places;
# the following solves this problem, but makes it harder to reuse .src.rpm
%define _sourcedir /tmp

Summary:   F(?) Virtual Window Manager
Name:      fvwm
Version:   %{ver}
Release:   %{rel}
License:   GPL
Group:     User Interface/Desktops
Source:    %{name}-%{version}.tar.gz
URL:       http://www.fvwm.org/
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Packager:  Fvwm Workers <fvwm-workers@fvwm.org>
Autoreq:   1
Requires:  perl >= 5.004

# RedHat should stop using name "fvwm" to refer to fvwm1 package.
Provides:  fvwm

Docdir:    %{prefix}/share/doc

%description
Fvwm is a powerful ICCCM-compliant multiple virtual desktop window manager
for the X Window System.

This 2.5 version includes new features like full support of the EWMH
(Enhanced Window Manager Hints) specification, internationalization,
improved window decoration code (no flickering anymore), bi-directional
asian text support, FreeType font support (antialiasing), image rendering,
Perl based module library, support for PNG images, side titles and much more.

%description -l fr
Fvwm est un gestionnaire de fenêtres puissant et extrêmement configurable
pour le système X Window.

La version 2.5 contient un grand nombre de nouvelles fonctionnalités.
Voici quelques exemples:
full support of the EWMH
(Enhanced Window Manager Hints) specification, internationalization,
improved window decoration code (no flickering anymore), bi-directional
asian text support, FreeType font support (antialiasing), image rendering,
Perl based module library, support for PNG images, side titles and much more.

%description -l ru
Fvwm является мощным оконным менеджером для X Window System, соответствующим
стандартам ICCCM, с поддержкой множественных виртуальных десктопов.

Версия 2.5 включает в себя новые особенности, такие как полная поддержка
спецификации EWMH (Enhanced Window Manager Hints), интернационализация,
улучшение оконных декораций (полное отсутствие мигания), поддержка
дву-направленного азиатского текста, поддержка фонтов FreeType (сглаживание),
фильтрование изображений, библиотека для написания модулей на Perl,
поддержка изображений в формате PNG, боковые заголовки окон и многое другое.

%prep
%setup

%build
# gnome libs are only used in FvwmGtk, probably it is overhead to require them;
# compiling without -g saves about 7Mb
./configure --prefix=%{prefix} --mandir=/usr/share/man %{c_p}
make %{m_p}

%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT prefix=%{prefix} install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)

%doc AUTHORS COPYING INSTALL INSTALL.fvwm NEWS README
%doc docs/ANNOUNCE docs/BUGS docs/COMMANDS docs/DEVELOPERS docs/FAQ docs/TODO
%doc docs/error_codes docs/fvwm.lsm
%{prefix}/bin/*
%{prefix}/libexec/*
%{prefix}/share/doc/*
%{prefix}/share/fvwm
%{prefix}/share/man/*/*
%{prefix}/share/locale/*/*/*

%define date%(echo `LC_ALL="C" date +"%a %b %d %Y"`)
%changelog
* %{date} Fvwm Workers <fvwm-workers@fvwm.org>
  - Auto building %{version}-%{release}
* Sun May 12 2000 Mikhael Goikhman <migo@homemail.com>
  - First try at making the package
