#!/usr/bin/env bash

# run from the Stardew directory after building to create an appimage called farming.AppImage in the appimage folder

appdir="./appimage/appdir/usr"

linuxdeploy="./appimage/linuxdeploy-x86_64.AppImage"

appimagetool="./appimage/appimagetool-x86_64.AppImage"


rm -rf "$appdir"
mkdir "$appdir"

cmake --install build --prefix $appdir


# get tools if they don't exist
if [ ! -f $linuxdeploy ]; then
    pushd ./appimage
    wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
    chmod +x linuxdeploy-x86_64.AppImage
    popd

fi

if [ ! -f $appimagetool ]; then
    pushd ./appimage
    wget https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage
    chmod +x appimagetool-x86_64.AppImage
    popd
fi


mkdir ./appimage/appdir/usr/
cp ./appimage/AppRun ./appimage/appdir/usr/AppRun
chmod +x ./appimage/appdir/usr/AppRun

mkdir -p ./appimage/appdir/usr/share/applications

cp ./appimage/WarFarmer.desktop ./appimage/appdir/usr/share/applications/WarFarmer.desktop

export LD_LIBRARY_PATH="./build/engine/engine/src:$LD_LIBRARY_PATH"

$linuxdeploy --appimage-extract-and-run --appdir appimage/appdir -e appimage/appdir/usr/bin/WarFarmer -i appimage/aphex.png

rm -f appimage/appdir/AppRun
cp appimage/appdir/usr/AppRun appimage/appdir/AppRun
chmod +x appimage/appdir/AppRun

$appimagetool --appimage-extract-and-run appimage/appdir appimage/farming.AppImage

chmod +x appimage/farming.AppImage

rm -rf ./appimage/appdir
