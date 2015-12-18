#!/bin/bash
./build.sh
./bin/hierarch
cd bin
dot -Tpng tnode_tree.dot -o tree.png
dot -Tpng avl_tree.dot -o avl_tree.png
dot -Tpng type_avl_tree.dot -o type_avl_tree.png
