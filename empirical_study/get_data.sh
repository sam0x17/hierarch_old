#!/bin/bash
echo "Downloading 150k js dataset from Software Reliability Lab (srl.inf.ethz.ch)..."
rm data -rf || exit 1
mkdir data || exit 1
touch data/.keep || exit 1
mkdir data/javascript_150k || exit 1
mkdir data/tmp || exit 1
cd data/tmp || exit 1
wget http://files.srl.inf.ethz.ch/data/js_dataset.tar.gz || exit 1
echo "download complete."
echo "extracting main archive (this might take a long time)..."
tar -xzvf js_dataset.tar.gz || exit 1
echo "extracting inner archive..."
tar -xzvf data.tar.gz || exit 1
mv data/* ../javascript_150k/ || exit 1
cd .. || exit 1
rm tmp -rf || exit 1
echo "extraction complete. JS dataset is now located in ./data/javascript_150k"
echo "done"
