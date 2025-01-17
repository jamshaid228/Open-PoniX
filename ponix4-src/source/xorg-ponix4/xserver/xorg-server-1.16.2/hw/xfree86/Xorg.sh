#!/bin/sh
#
# Execute Xorg.wrap if it exists otherwise execute Xorg.bin directly.
# This allows distros to put the suid wrapper in a separate package.

basedir=
if [ -x "$basedir"/Xorg.wrap ]; then
	exec "$basedir"/Xorg.wrap "$@"
else
	exec "$basedir"/Xorg.bin "$@"
fi
