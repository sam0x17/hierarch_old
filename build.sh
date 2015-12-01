#!/bin/bash
echo ""
echo "Hierarch 1.0 build process started"
echo ""
echo "Copyright 2015 Sam Kelly"
echo ""
echo "processing prerequisites:"
printf "checking for wget... "
command -v wget >/dev/null 2>&1 || { echo "not installed, aborting..." >&2; exit 1; }
echo "OK"
printf "checking for tar... "
command -v tar >/dev/null 2>&1 || { echo "not installed, aborting..." >&2; exit 1; }
echo "OK"
printf "checking for git... "
command -v git >/dev/null 2>&1 || { echo "not installed, aborting..." >&2; exit 1; }
echo "OK"
printf "checking for GNU LibAVL... "
if [ ! -d "bin/usr/local/include/gnuavl" ]; then
  ./scripts/install_libavl.sh
else
  echo "OK"
fi
echo ""
echo "compiling test/demo to bin/hierarch..."
g++ -std=c++11 -O -I bin/usr/local/include src/avl.cpp src/dfilter.cpp src/test.cpp -o bin/hierarch
echo ""
echo "compilation complete."
