./configure --prefix=/usr --x-includes=/usr/xorg/include --x-libraries=/usr/xorg/lib --enable-xft \
--disable-png --disable-jpeg --disable-confirm --disable-shape --disable-debug \
CFLAGS="${CFLAGS} -I/source/wm/jwm-581/include" \
LDFLAGS="${LDFLAGS} -L/source/wm/jwm-581/lib" \
PKG_CONFIG_PATH=/source/wm/jwm-581/lib/pkgconfig:${PKG_CONFIG_PATH}
cp config.h.my config.h
cp Makefile.my src/Makefile