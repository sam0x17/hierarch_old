#!/bin/bash
./build.sh
./bin/hierarch
cd bin
dot -Tpng before_insert.dot -o before_insert.png
dot -Tpng after_insert.dot -o after_insert.png
dot -Tpng avl_before_insert.dot -o avl_before_insert.png
dot -Tpng avl_after_insert.dot -o avl_after_insert.png
