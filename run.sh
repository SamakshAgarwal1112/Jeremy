#!/bin/bash

cd "$(dirname "$0")"

mkdir -p build
cd build

cmake ..
make -j$(nproc)

cd ..

gnome-terminal -- bash -c "./build/server; exec bash"

gnome-terminal -- bash -c "./build/client; exec bash"
