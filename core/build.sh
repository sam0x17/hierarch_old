#!/bin/bash
mkdir -p include || exit 1
rm include/* -rf || exit 1
mkdir -p bin || exit 1
mkdir -p opt || exit 1
rm -rf bin/* || exit 1
if [ ! -d "opt/sparsepp" ]; then
  cd opt || exit 1
  git clone git@github.com:greg7mdp/sparsepp.git || exit 1
  cd .. || exit 1
fi
if [ ! -d "opt/avltree" ]; then
  cd opt || exit 1
  git clone git@github.com:greensky00/avltree.git || exit 1
  cd .. || exit 1
fi
mkdir -p include/sparsepp || exit 1
mkdir -p include/hierarch || exit 1
mkdir -p include/avltree || exit 1
cp opt/sparsepp/sparsepp/*.h include/sparsepp/ || exit 1
cp opt/avltree/avltree/* include/avltree || exit 1
sed -i 's/avl_parent/_avl_parent/g' include/avltree/* || exit 1
cp src/*.h include/hierarch/ || exit 1
cat src/hierarch.cpp src/node.cpp > /tmp/hierarch.cpp || exit 1 # amalgamate cpp files for simpler include structure
g++ /tmp/hierarch.cpp include/avltree/avltree.c -std=c++14 -static -Wall -O3 -I include -o bin/hierarch || exit 1
./bin/hierarch
