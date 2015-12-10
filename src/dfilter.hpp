#ifndef _DFILTER_GUARD
#define _DFILTER_GUARD

#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <list>
#include "assert.h"
extern "C" {
  #include "pavl.h"
}

namespace DFI {

  int compare_dnodes(const void *pavl_a, const void *pavl_b, void *pavl_param);

  class TNode;
  class DFilter;

  class DNode {
  public:
    unsigned int mod_num;
    unsigned int base_index;
    unsigned int type_base_index;
    int rhs_offset;
    int type_rhs_offset;
    TNode *tnode;
    struct pavl_node *pnode;
    struct pavl_node *type_pnode;
    TNode *postorder_successor;
    std::list<TNode*> incoming_successor_links;
    DFilter *dfilter;

    unsigned int dfi();
    unsigned int type_dfi();
    DNode *avl_parent();
    bool pnode_is_rhs();
    bool pnode_has_children();
    bool type_pnode_is_rhs();
  };

  class TNode {
  public:
    TNode *parent;
    DNode *dnode;
    std::vector<TNode*> children;
    void *data;
    int type = 0;
    void add_child(TNode *child);
    static void delete_tree(TNode *&root);
  };

  class DFilter {
  public:
    TNode *troot;
    unsigned int latest_mod;
    struct pavl_table *tbl;
    std::unordered_map<int, struct pavl_table*> type_tables;
    std::unordered_map<int, int> latest_type_mods;

    DFilter();
    DFilter(TNode *root);
    DNode *avl_root();
    DNode *type_avl_root();
    void generate_index(TNode *root);
    void assign_dnode(TNode *tnode, unsigned int base_index, int rhs_offset);
    struct pavl_table *acquire_type_table(int type);
  };

}

#endif
