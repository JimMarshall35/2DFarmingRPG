#!/bin/bash

export LD_LIBRARY_PATH="$PWD/build/openal_backend:$LD_LIBRARY_PATH"

python3 ./python/zzfx_gui.py $@