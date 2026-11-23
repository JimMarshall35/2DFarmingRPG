#!/bin/bash

MAJOR_VERSION=$(cat ./core/include/ZzFX_version.h | grep -P -o "(?<=#define ZZFX_MAJOR_VERSION \")[0-9]+(?=\")")
MINOR_VERSION=$(cat ./core/include/ZzFX_version.h | grep -P -o "(?<=#define ZZFX_MINOR_VERSION \")[0-9]+(?=\")")
REVISION=$(cat ./core/include/ZzFX_version.h | grep -P -o "(?<=#define ZZFX_REVISION \")[0-9]+(?=\")")
PACKAGE_NAME="ZzFX-C"
DEB_PKG_NAME=$(printf "%s_%s.%s-%s" $PACKAGE_NAME $MAJOR_VERSION $MINOR_VERSION $REVISION)
VERSION_STRING=$(printf "%s.%s-%s" $MAJOR_VERSION $MINOR_VERSION $REVISION)

# setup directories
mkdir -p ./$DEB_PKG_NAME/DEBIAN
mkdir -p ./$DEB_PKG_NAME/usr/bin
mkdir -p ./$DEB_PKG_NAME/usr/lib
mkdir -p ./$DEB_PKG_NAME/usr/include/zzfx

# install python scripts
cp ./python/zzfx_gui_lib.py ./$DEB_PKG_NAME/usr/bin/zzfx_gui_lib.py
cp ./python/zzfx.py ./$DEB_PKG_NAME/usr/bin/zzfx.py
cp ./python/zzfx_cli.py ./$DEB_PKG_NAME/usr/bin/zzfx_cli.py
cp ./python/zzfx_gui.py ./$DEB_PKG_NAME/usr/bin/zzfx_gui.py

# install scripts to start gui and cli
StartGui=$(cat << 'EOF'
#!/bin/bash
python3 /usr/bin/zzfx_gui.py $@
EOF
)
echo "$StartGui" >> ./$DEB_PKG_NAME/usr/bin/zzfx_gui
chmod +x ./$DEB_PKG_NAME/usr/bin/zzfx_gui

StartCLI=$(cat << 'EOF'
#!/bin/bash
python3 /usr/bin/zzfx_cli.py $@
EOF
)

echo "$StartCLI" >> ./$DEB_PKG_NAME/usr/bin/zzfx_cli
chmod +x ./$DEB_PKG_NAME/usr/bin/zzfx_cli

# install libraries
cp ./build/core/libZzFX.a ./$DEB_PKG_NAME/usr/lib/libZzFX.a
cp ./build/openal_backend/libZzFX_OpenAL.so ./$DEB_PKG_NAME/usr/lib/libZzFX_OpenAL.so

# install headers
cp -r ./core/include/*.h ./$DEB_PKG_NAME/usr/include/zzfx


# install debian control file
DebCtrl=$(cat << 'EOF'
Package: ZzFX-C
Version: %s
Section: games
Priority: optional
Architecture: amd64
Depends: python3 (>=3.12.3-0ubuntu2.1), libopenal-dev (>=1.23.1), python3-matplotlib (>=3.6.3)
Maintainer: Jim Marshall <jimmarshall35@gmail.com>
Description: An unofficial C port of https://github.com/KilledByAPixel/ZzFX
EOF
)

fixed=$(printf "$DebCtrl" "$VERSION_STRING")

echo "$fixed"

echo "$fixed" > ./$DEB_PKG_NAME/DEBIAN/control

# output name of debian folder for later jobs to read
#echo "./$DEB_PKG_NAME" > foldername.txt

# build debian
dpkg-deb --build "./$DEB_PKG_NAME"

