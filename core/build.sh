#!/bin/bash
mkdir -p include || exit 1
rm include/* -rf || exit 1
mkdir -p bin || exit 1
rm -rf bin/* || exit 1
cd bin || exit 1
git clone git@github.com:greensky00/avltree.git || exit 1
git clone git@github.com:greg7mdp/sparsepp.git || exit 1
mkdir -p ../include/sparsepp || exit 1
mkdir -p ../include/hierarch || exit 1
mkdir -p ../include/avltree || exit 1
cp sparsepp/sparsepp/*.h ../include/sparsepp/ || exit 1
cp avltree/avltree/* ../include/avltree || exit 1
cd .. || exit 1
rm bin/* -rf || exit 1
cp src/*.h include/hierarch/ || exit 1
g++ src/*.cpp include/avltree/avltree.c -std=c++14 -static -Wall -O3 -I include -o bin/hierarch || exit 1
