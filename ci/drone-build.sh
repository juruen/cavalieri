#!/bin/bash

set -e

sudo apt-get install debhelper cmake subversion protobuf-compiler \
  libprotobuf-dev libev-dev libgflags-dev libpython-dev \
  libcurl4-openssl-dev libssl-dev libtbb-dev libjsoncpp-dev lcov \
  flex bison

mkdir build
cd build
cmake ..
make -j8
