#!/bin/sh

#
# Build release binaries for patcher9x
#
# Written by Mark Cave-Ayland <mark.cave-ayland@ilande.co.uk>
#
# Builds all of Linux amd64, win32, win64 and DOS boot floppy
#

# Extract version from the first line of CHANGELOG
export VERSION=`head -n1 CHANGELOG | sed -nz 's/\([0-9\.]\{1,\}\).*/\1/p'`
echo "Building release $VERSION"

# Archive vanilla source
SRCDIR=`pwd`
tar zcf /tmp/patcher9x.tar.gz .

# Extract archive to build-djgpp and build DOS executable
rm -rf /tmp/build-djgpp && mkdir -p /tmp/build-djgpp && \
cd /tmp/build-djgpp && tar xvf /tmp/patcher9x.tar.gz && \
make RELEASE=1 PROFILE=djgpp && \
make RELEASE=1 PROFILE=djgpp strip && \
cd $SRCDIR

# Extract archive to build-win32 and build Win32 executable
rm -rf /tmp/build-win32 && mkdir -p /tmp/build-win32 && \
cd /tmp/build-win32 && tar xvf /tmp/patcher9x.tar.gz && \
make PROFILE=nocrt GUEST_CC=i686-w64-mingw32-gcc RELEASE=1 && \
make PROFILE=nocrt SUFIX=.exe strip && \
cd $SRCDIR

# Extract archive to build-win64 and build Win64 executable
rm -rf /tmp/build-win64 && mkdir -p /tmp/build-win64 && \
cd /tmp/build-win64 && tar xvf /tmp/patcher9x.tar.gz && \
make PROFILE=nocrt64 GUEST_CC=x86_64-w64-mingw32-gcc RELEASE=1 && \
make PROFILE=nocrt64 SUFIX=.exe strip && \
cd $SRCDIR

# Extract archive to build-amd64 and build Linux executable
rm -rf /tmp/build-linux-amd64 && mkdir -p /tmp/build-linux-amd64 && \
cd /tmp/build-linux-amd64 && tar xvf /tmp/patcher9x.tar.gz && \
make RELEASE=1 && \
make RELEASE=1 strip && \
cd $SRCDIR

# Build README.txt from patcher9x help output
cd /tmp/build-linux-amd64 && PATH=.:$PATH patcher9x -h > /tmp/README.txt && cd $SRCDIR

# Build win32 archive
mkdir -p /tmp/archive-win32 && \
cp /tmp/build-win32/patcher9x.exe /tmp/archive-win32 && \
cp CHANGELOG /tmp/archive-win32/CHANGELOG.txt && \
cp LICENSE /tmp/archive-win32/LICENCE.txt && \
cat /tmp/README.txt | sed 's/patcher9x/patcher9x.exe/g' > /tmp/archive-win32/README.txt && \
cd /tmp/archive-win32 && zip /tmp/patcher9x-$VERSION-win32.zip * && cd $SRCDIR

# Build win64 archive
mkdir -p /tmp/archive-win64 && \
cp /tmp/build-win64/patcher9x.exe /tmp/archive-win64 && \
cp CHANGELOG /tmp/archive-win64/CHANGELOG.txt && \
cp LICENSE /tmp/archive-win64/LICENCE.txt && \
cat /tmp/README.txt | sed 's/patcher9x/patcher9x.exe/g' > /tmp/archive-win64/README.txt && \
cd /tmp/archive-win64 && zip /tmp/patcher9x-$VERSION-win64.zip * && cd $SRCDIR

# Build Linux amd64 archive
mkdir -p /tmp/archive-linux-amd64 && \
cp /tmp/build-linux-amd64/patcher9x /tmp/archive-linux-amd64 && \
cp CHANGELOG /tmp/archive-linux-amd64/CHANGELOG && \
cp LICENSE /tmp/archive-linux-amd64/LICENCE && \
cp /tmp/README.txt /tmp/archive-linux-amd64/README && \
cd /tmp/archive-linux-amd64 && tar zcvf /tmp/patcher9x-$VERSION-linux-amd64.tar.gz * && cd $SRCDIR

# Build boot floppy: extract partition image from LiteUSB edition for extras
unzip -d /tmp /opt/freedos/freedos-liteusb.zip FD13LITE.img && \
SS=`file /tmp/FD13LITE.img | sed 's/.*startsector \([0-9]\{1,\}\).*/\1/g'` && \
mkdir -p /var/lib/dosemu && dd if=/tmp/FD13LITE.img of=/var/lib/dosemu/fdimage skip=$SS

# Copy off boot disk files to /tmp/dosfiles
mkdir -p /tmp/dosfiles && \
mmount -V n: && \
mcopy -pm n:/freedos/bin/XCOPY.EXE /tmp/dosfiles && \
mcopy -pm n:/freedos/bin/COMMAND.COM /tmp/dosfiles && \
mcopy -pm n:/freedos/bin/DELTREE.COM /tmp/dosfiles && \
mcopy -pm n:/freedos/bin/FDISK.EXE /tmp/dosfiles && \
mcopy -pm n:/freedos/bin/FDISK.INI /tmp/dosfiles && \
mcopy -pm n:/freedos/bin/FORMAT.EXE /tmp/dosfiles && \
mcopy -pm n:/freedos/bin/FDISKPT.INI /tmp/dosfiles && \
mcopy -pm n:/freedos/bin/HIMEMX.EXE /tmp/dosfiles && \
mcopy -pm n:/freedos/bin/JEMM386.EXE /tmp/dosfiles && \
mcopy -pm n:/freedos/bin/SHSUCDX.COM /tmp/dosfiles && \
mcopy -pm n:/freedos/bin/UDVD2.SYS /tmp/dosfiles && \
mcopy -pm n:/freedos/bin/DEVLOAD.COM /tmp/dosfiles && \
mcopy -pm n:/freedos/bin/EDIT.EXE /tmp/dosfiles

# Copy CWSDPMI files to /tmp/dosfiles
mkdir -p /tmp/cwsdpmi && unzip -d /tmp/cwsdpmi /opt/cwsdpmi/cwsdpmi.zip bin/CWSDPMI.EXE bin/cwsdpmi.doc && \
cp -p /tmp/cwsdpmi/bin/CWSDPMI.EXE /tmp/dosfiles && \
cp -p /tmp/cwsdpmi/bin/cwsdpmi.doc /tmp/dosfiles/CWSDPMI.TXT

# Copy PATCH9X.EXE and repo boot files to /tmp/dosfiles
cp -p /tmp/build-djgpp/patch9x.exe /tmp/dosfiles/PATCH9X.EXE && \
cp -p boot/autoexec.bat /tmp/dosfiles/AUTOEXEC.BAT && \
cp -p boot/cdrom.bat /tmp/dosfiles/CDROM.BAT && \
cp -p boot/fdconfig.sys /tmp/dosfiles/FDCONFIG.SYS && \
cp -p boot/info.bat /tmp/dosfiles/INFO.BAT && \
cp -p boot/info.txt /tmp/dosfiles/INFO.TXT && \
cat boot/readme.txt.template | sed "s/%VERSION%/$VERSION/g" > /tmp/dosfiles/README.TXT

# Finally take the boot floppy, delete the unused files and then copy over ours
mkdir -p /tmp/floppy && unzip -d /tmp/floppy /opt/freedos/freedos-floppy.zip 144m/x86BOOT.img && \
cp /tmp/floppy/144m/x86BOOT.img /var/lib/dosemu/fdimage

mdel n:/FDAUTO.BAT && \
mdel n:/FDCONFIG.SYS && \
mdel n:/SETUP.BAT && \
mdeltree n:/FREEDOS && \
mcopy -pm /tmp/dosfiles/XCOPY.EXE n: && \
mcopy -pm /tmp/dosfiles/COMMAND.COM n: && \
mcopy -pm /tmp/dosfiles/DELTREE.COM n: && \
mcopy -pm /tmp/dosfiles/FDISK.EXE n: && \
mcopy -pm /tmp/dosfiles/FDISK.INI n: && \
mcopy -pm /tmp/dosfiles/FORMAT.EXE n: && \
mcopy -pm /tmp/dosfiles/FDISKPT.INI n: && \
mcopy -pm /tmp/dosfiles/HIMEMX.EXE n: && \
mcopy -pm /tmp/dosfiles/JEMM386.EXE n: && \
mcopy -pm /tmp/dosfiles/SHSUCDX.COM n: && \
mcopy -pm /tmp/dosfiles/UDVD2.SYS n: && \
mcopy -pm /tmp/dosfiles/DEVLOAD.COM n: && \
mcopy -pm /tmp/dosfiles/EDIT.EXE n: && \
mcopy -pm /tmp/dosfiles/CWSDPMI.EXE n: && \
mcopy -pm /tmp/dosfiles/CWSDPMI.TXT n: && \
mcopy -pm /tmp/dosfiles/PATCH9X.EXE n: && \
mcopy -pm /tmp/dosfiles/AUTOEXEC.BAT n: && \
mcopy -pm /tmp/dosfiles/CDROM.BAT n: && \
mcopy -pm /tmp/dosfiles/FDCONFIG.SYS n: && \
mcopy -pm /tmp/dosfiles/INFO.BAT n: && \
mcopy -pm /tmp/dosfiles/INFO.TXT n: && \
mcopy -pm /tmp/dosfiles/README.TXT n:

# Copy the ima file to /tmp
cp /var/lib/dosemu/fdimage /tmp/patcher9x-$VERSION-boot.ima
