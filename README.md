# Hierarch

Hierarch is a proof-of-concept implementation of a novel hierarchical indexing scheme and data
structure designed to yield rapid results to "get descendants by type" queries over dynamic,
typed, in-memory trees. That is, given some arbitrary tree node, this index will answer 
queries of the form "give me all the descendants of this node that are of type T" in
near-constant time. Since these types of queries are the basis for most containment-based
analysis, this can be quite powerful. What's special about this index is that it can be
updated on the fly with new tree changes without creating a crippling performance bottleneck.

## Setup

Hierarch was designed for a UNIX environment, and has only been tested on Ubuntu 14.04.

1. clone repository
2. ensure that g++ and graphviz are installed and that your system can compile C++11
3. ./build.sh
4. (packages are magically installed in the bin directory of the project without affecting your system)
5. ./run.sh

This will run the test suite followed by the benchmarks used in the paper.
