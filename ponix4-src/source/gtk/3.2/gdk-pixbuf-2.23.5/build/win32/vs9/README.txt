Note that all this is rather experimental.

This VS9 solution and the projects it includes are intented to be used
in a GDK-Pixbuf source tree unpacked from a tarball. In a git checkout you
first need to use some Unix-like environment or manual work to expand
the .in files needed, mainly config.h.win32.in into config.h.win32.
You will also need to expand gdk-pixbuf.vcprojin here into
gdk-pixbuf.vcproj.

Two variants of the library are supported here for compilation-one
that makes use of native GDI+ APIs (The Debug/Release configurations)
and another that does not use GDI+ (those that use external third
party image manipulation libraries, the Debug_NoGDIP/Release_NoGDIP
configurations).  Note that both variants require LibPNG and ZLib, and
both variants have the image loaders built into the main GDK-Pixbuf library.

The dependencies for this package are gettext-runtime (libintl), GLib*
ZLib and LibPNG at the minimum.  Note that if you intend to build GDK-Pixbuf
that does not use GDI+, you will also need the IJG JPEG libraries, libTIFF and
libjasper (for JPEG-2000 operations).

For the Dependencies, you may either:

a) look for all of the dependencies (except GLib*, libjasper) under

   http://ftp.gnome.org/pub/GNOME/binaries/win32/dependencies/ (32-bit) -OR-
   http://ftp.gnome.org/pub/GNOME/binaries/win64/dependencies/ (64-bit)

   Please use the latest versions of these libraries that are available there,
   these are packaged by Tor Lillqvist, which are built with MinGW/GCC.
   Please see b) below regarding the build of libjasper and GLib*

   Note for LibPNG, version 1.5.x is needed.

-OR-

b) Build them yourself with VS9 (but you may most probably wish to get
   gettext-runtime from the URL(s) mentioned in a)).  Use the latest
   stable versions for them (you may need to get the latest unstable version of
   GLib if you are using an unstable version of GDK-Pixbuf):

   GLib*:   Grab the latest sources from http://www.gtk.org under "Download"
            (stable only-please make a search for the latest unstable versions)
   IJG JPEG: http://www.ijg.org/
   LibPNG: http://www.libpng.org/pub/png/libpng.html (1.5.x is needed here)
   LibTIFF: http://www.remotesensing.org/libtiff/
   LibJasper: http://www.ece.uvic.ca/~mdadams/jasper/
   ZLib:   http://www.zlib.net

   The above 6 packages all have supported mechanisms (Makefiles and/or Project
   Files) for building under VS9 (upgrade the Project Files from earlier VS
   versions will do for these, when applicable).  It is recommended that ZLib
   is built using the win32/Makefile.msc makefile with VS9 with the ASM routines
   to avoid linking problems (copy zdll.lib to zlib1.lib[Release] or to zlib1d.lib
   [Debug] after completion of compilation)-see win32/Makefile.msc in ZLib for
   more details.

* This GLib refers to a build that is built by VS9

Set up the source tree as follows under some arbitrary top
folder <root>:

<root>\gdk-pixbuf\<this-gdk-pixbuf-source-tree>
<root>\vs9\<PlatformName>

*this* file you are now reading is thus located at
<root>\gdk-pixbuf\<this-gdk-pixbuf-source-tree>\build\win32\vs9\README.

<PlatformName> is either Win32 or x64, as in VS9 project files.

You should unpack the <dependent-package>-dev and <dependent-packge> (runtime)
into <root>\vs9\<PlatformName>, if you download any of the packages from

http://ftp.gnome.org/pub/GNOME/binaries/win32/dependencies/ (32-bit) -OR-
http://ftp.gnome.org/pub/GNOME/binaries/win64/dependencies/ (64-bit)

so that for instance libintl.h end up at 
<root>\vs9\<PlatformName>\include\libintl.h.

If you build any of the dependencies yourselves, copy the: 
-DLLs and EXEs into <root>\vs9\<PlatformName>\bin
-headers into <root>\vs9\<PlatformName>\include\
-LIBs into <root>\vs9\<PlatformName>\lib

If you have not built GLib with VS9 and placed the LIBs and headers in a
place where VS9 can find them automatically, you should also uncompress
your GLib sources in <root>\ and build it from there, following the
instructions in <root>\glib<-version>\build\win32\vs9, so that the required
headers, EXEs, DLLs and LIBs will end up in
<root>\vs9\<PlatformName>\include\glib-2.0 (headers)
<root>\vs9\<PlatformName>\lib (LIBs, also glib-2.0/include/glibconfig.h)
<root>\vs9\<PlatformName>\bin (EXEs/DLLs)
respectively.

After the build of GDK-Pixbuf, the "install" project will copy build results
and headers into their appropriate location under <root>\vs9\<PlatformName>.
For instance, built DLLs go into <root>\vs9\<PlatformName>\bin, built LIBs into
<root>\vs9\<PlatformName>\lib and GDK-Pixbuf headers into
<root>\vs9\<PlatformName>\include\GDKPixbuf-2.0. This is then from where
project files higher in the stack are supposed to look for them, not
from a specific GDK-Pixbuf source tree.

--Chun-wei Fan <fanc999@yahoo.com.tw>
--(adapted from the GLib VS9 README.txt file originally written by Tor Lillqvist)
