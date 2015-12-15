#!/bin/bash
./build.sh
./bin/hierarch
cd bin
dot -Tpng before_insert.dot -o 03before_insert.png
dot -Tpng after_insert.dot -o 04after_insert.png
dot -Tpng avl_before_insert.dot -o 01avl_before_insert.png
dot -Tpng avl_after_insert.dot -o 02avl_after_insert.png
