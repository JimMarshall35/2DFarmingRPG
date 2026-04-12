#!/bin/bash

if [ ! -d "build" ]; then
  mkdir build
fi

echo $1
echo "bake item defs? $2"
cd build
cmake .. -DCMAKE_BUILD_TYPE=$1 -DBAKE_ITEM_DEFS=$2
make
cd game

if [ ! -d "WfAssets" ]; then
  mkdir WfAssets
fi

cd ..
cd ..

echo "Copying assets folder..."
cp -a WfAssets build/game

echo "Copying test data..."
cp -a enginetest/data build/enginetest