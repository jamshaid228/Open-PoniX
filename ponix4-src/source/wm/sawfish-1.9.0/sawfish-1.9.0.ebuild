# Copyright 1999-2008 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

# This is *not* an official ebuild of any distributions.
# After the configure script is run, required package versions are
# filled in. But other than that, there's no guarantee this is a
# correct ebuild.

inherit eutils

DESCRIPTION="Extensible window manager using a Lisp-based scripting language"
HOMEPAGE="http://sawfish.wikia.com/"
SRC_URI="mirror://sourceforge/sawmill/${P}.tar.bz2"

LICENSE="GPL-2"
SLOT="0"
KEYWORDS="alpha amd64 ia64 ppc ppc64 sparc x86"
IUSE="gnome nls pango imlib"

RDEPEND=">=x11-libs/rep-gtk-0.90.7
	>=dev-libs/librep-0.92.1
	>=x11-libs/gtk+-2.24.0
	imlib? <=x11-libs/imlib-1.9
	pango? >=x11-libs/pango-1.8.0
	nls? ( sys-devel/gettext )"

DEPEND="${RDEPEND}
	>=dev-util/pkgconfig-0.12.0"

src_compile() {

	if use gnome; then
		set -- "$@" \
			--with-gnome-prefix=/usr
	fi

	if ! use pango; then
		set -- "$@" \
			--without-pango
	fi

	if ! use nls; then
		set -- "$@" \
			--disable-nls

	fi

	if use imlib; then
		set -- "$@" \
			--without-gdk-pixbuf
	else	set -- "$@" \
			--with-gdk-pixbuf
	fi

	econf "$@" || die "configure failed"

	emake || die "make failed"

}

src_install() {

	emake DESTDIR="${D}" install || die "make install failed"
	dodoc AUTHORS ChangeLog FAQ NEWS README TODO OPTIONS README.IMPORTANT KEYBINDINGS USERDOC COPYING COPYING.SOUNDS

}
