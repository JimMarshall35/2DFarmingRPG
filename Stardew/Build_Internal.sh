#!/usr/bin/env bash

if [ ! -d "build" ]; then
  mkdir build
fi

echo $1

cd build
cmake .. -DCMAKE_BUILD_TYPE=$1
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
