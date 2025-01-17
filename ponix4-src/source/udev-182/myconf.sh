#! /bin/sh

./configure --exec-prefix=/usr --prefix=/usr --disable-gtk-doc-html --disable-introspection \
--with-selinux=no  LIBUSB_LIBS=/usr/lib --disable-manpages --libexecdir=/bin \
--disable-gudev --disable-keymap --disable-mtd_probe  --enable-debug --enable-logging


#  --enable-gtk-doc        use gtk-doc to build documentation [[default=no]]
#  --enable-gtk-doc-html   build documentation in html format [[default=yes]]
#  --enable-gtk-doc-pdf    build documentation in pdf format [[default=no]]
#  --enable-debug          enable debug messages
#  --disable-logging       disable system logging
#  --disable-extras        disable extras with external dependencies
#  --disable-introspection disable GObject introspection
#  --with-html-dir=PATH    path to installed docs
#  --with-rootlibdir=DIR   rootfs directory to install shared libraries
#  --with-selinux          enable SELinux support
#  --with-firmware-path=DIR[:DIR[...]]
#                          Firmware search path
#                          (default=/lib/firmware/updates:/lib/firmware)
#  --with-systemdsystemunitdir=DIR
#                          Directory for systemd service files
#  --with-pci-ids-path=DIR Path to pci.ids file