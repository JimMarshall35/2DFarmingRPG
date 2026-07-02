#!/usr/bin/env bash

game_startup_script=$(cat <<EOF
#!/usr/bin/env bash
set -x
data_dir="\$HOME/.local/share"
game_saves="\$data_dir/openfarmer/saves"
game_config="\$data_dir/openfarmer/config"
mkdir -p \$game_saves
mkdir -p \$game_config
cp -a -r  --update=none /usr/share/WfAssets/Saves/. \$game_saves 
cp --update=none /usr/share/WfAssets/Keymap.json \$game_config
WarFarmer \$@ --assetsDir /usr/share/WfAssets --configDir \$game_config --savesDir \$game_saves 
EOF
)

# a post install script that makes the debian package suitable for an ongoing development testing build.
# it won't preserve saves between installs of the package but will ensure the game doesn't load corrupt data when the game save format changes.
# When a finished game is released the version numbering system will kick in when the binary format changes and this will be removed
postinst_script=$(cat <<EOF
#!/bin/sh
set -x

#!/bin/sh
set -e

case "$1" in
    configure)
        # runs once, after files are unpacked and this is a fresh install
        # or an upgrade from a previous version
        data_dir="\$HOME/.local/share"
        game_saves="\$data_dir/openfarmer/saves"
        rm -rf \$game_saves
        ;;
esac

exit 0
EOF
)



# parse versions from game header
MAJOR_VERSION=$(cat ./game/include/WfVersion.h | grep -P -o "(?<=#define WF_MAJOR_VERSION \")[0-9]+(?=\")")
MINOR_VERSION=$(cat ./game/include/WfVersion.h | grep -P -o "(?<=#define WF_MINOR_VERSION \")[0-9]+(?=\")")
REVISION=$(cat ./game/include/WfVersion.h | grep -P -o "(?<=#define WF_REVISION \")[0-9]+(?=\")")
PACKAGE_NAME="OpenFarmer"
DEB_PKG_NAME=$(printf "%s_%s.%s-%s" $PACKAGE_NAME $MAJOR_VERSION $MINOR_VERSION $REVISION)
VERSION_STRING=$(printf "%s.%s-%s" $MAJOR_VERSION $MINOR_VERSION $REVISION)
EXE_NAME="WarFarmer"

# create folder structure
mkdir -p "./$DEB_PKG_NAME/usr/bin"
mkdir -p "./$DEB_PKG_NAME/usr/lib"
mkdir -p "./$DEB_PKG_NAME/usr/share"
mkdir -p "./$DEB_PKG_NAME/usr/share/WfAssets"
mkdir -p "./$DEB_PKG_NAME/DEBIAN"

cmake --install build --prefix "./$DEB_PKG_NAME/usr"

# install debian control file
cp "./debian_control.txt" "./$DEB_PKG_NAME/DEBIAN"
mv "./$DEB_PKG_NAME/DEBIAN/debian_control.txt" "./$DEB_PKG_NAME/DEBIAN/control"
echo "$postinst_script" > "./$DEB_PKG_NAME/DEBIAN/postinst"
chmod 0755 "./$DEB_PKG_NAME/DEBIAN/postinst"
sed -i "s/<<VERSION>>/$VERSION_STRING/g" "./$DEB_PKG_NAME/DEBIAN/control"

# install script to start the game
echo "$game_startup_script" > "./$DEB_PKG_NAME/usr/bin/openfarmer"
chmod +x "./$DEB_PKG_NAME/usr/bin/openfarmer"

# build debian package
dpkg-deb --build "./$DEB_PKG_NAME"

# delete staging folder
rm -rf ./$DEB_PKG_NAME

