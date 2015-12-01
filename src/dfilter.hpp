#ifndef _DFILTER_GUARD
#define _DFILTER_GUARD

#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include "avl.hpp"

namespace DFI {

  class TNode;

  class DNode {
  public:
    int base_index;
    int rhs_offset;
    int depth;
    TNode *tnode;
    int tnode_depth;
    DNode *parent;
    DNode *left_child;
    DNode *right_child;
    DNode *preorder_successor;
  };

  class TNode {
  public:
    TNode *parent;
    DNode *dnode;
    std::vector<TNode*> children;
    void *data;
  };

  class DFilter {
    TNode *troot;
    int size;
  public:
    DFilter();
    DFilter(TNode *root);
    void generate_index(TNode *root);
  };

}

#endif
