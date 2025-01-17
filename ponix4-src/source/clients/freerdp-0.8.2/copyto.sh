#!/bin/bash

rm -r mydist

mkdir mydist

mkdir mydist/bin
mkdir mydist/lib
mkdir mydist/share

cp -R /build/bin/xfreerdp	mydist/bin/
cp -R -P /build/lib/freerdp	mydist/lib/
cp -R -P /build/lib/libfreerdp*	mydist/lib/
cp -R /build/share/freerdp	mydist/share/

tar -czf /tftpboot/freerdp.tgz mydist

