#!/bin/bash

# make bin and bin/opt directories
echo Making directories...
maindir=$(pwd)
mkdir -p bin
cd bin
bindir=$(pwd)
cd ..
mkdir -p bin/opt
mkdir -p bin/opt/download
mkdir -p bin/opt/libavl

# wipe out existing libavl installation
rm bin/opt/libavl/* -r -f
rm bin/opt/download/libavl* -r -f
rm bin/opt/gnu* -r -f
rm bin/opt/download/gnu* -r -f

# downoad, build, and install libavl
echo "downloading libavl..."
cd bin/opt/download
git clone git@github.com:gsauthof/gnulibavl-cmake.git
rm gnulibavl-cmake/.git -r -f
git clone git@github.com:samkelly/libavl_fork.git
rm libavl_fork/.git -r -f
mv gnulibavl-cmake/* libavl_fork/
mv libavl_fork/* ../libavl/
rm libavl_fork -r -f
cd ../libavl
mkdir build
cd build
echo "buliding libavl..."
DESTDIR=$bindir cmake -C ../init.cmake -D LIB_SUB_DIR=lib ..
echo "installing libavl..."
DESTDIR=$bindir make install
echo "GNU LibAVL built and installed in $bindir"
