#ifndef _DFILTER_GUARD
#define _DFILTER_GUARD

#include <iostream>
#include <vector>
#include <queue>
#include <map>
extern "C" {
  #include "pavl.h"
}

namespace DFI {

  int compare_dnodes(const void *pavl_a, const void *pavl_b, void *pavl_param);

  class TNode;
  class DFilter;

  class DNode {
  public:
    unsigned int base_index;
    int rhs_offset;
    TNode *tnode;
    unsigned int tnode_depth;
    struct pavl_node *pnode;
    DNode *preorder_successor;

    DNode *parent();
    DNode *left_child();
    DNode *right_child();

  };

  class TNode {
  public:
    TNode *parent;
    DNode *dnode;
    std::vector<TNode*> children;
    void *data;
  };

  class DFilter {
  public:
    TNode *troot;
    struct pavl_table *tbl;
    DFilter();
    DFilter(TNode *root);
    void generate_index(TNode *root);
    void assign_dnode(TNode *tnode, unsigned int base_index, int rhs_offset, unsigned int tnode_depth);
  };

}

#endif
