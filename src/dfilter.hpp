#ifndef _DFILTER_GUARD
#define _DFILTER_GUARD

#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <stack>
#include "assert.h"
extern "C" {
  #include "pavl.h"
}

namespace DFI {

  int compare_dnodes(const void *pavl_a, const void *pavl_b, void *pavl_param);

  class TNode;
  class DFilter;
  class DNode;
  class SLink;
  class DResult;

  class DNode {
  public:
    unsigned int mod_num;
    unsigned int type_mod;
    unsigned int base_index;
    unsigned int type_base_index;
    int rhs_offset;
    int type_rhs_offset;
    TNode *tnode;
    struct pavl_node *pnode;
    struct pavl_node *type_pnode;
    DFilter *dfilter;
    unsigned int smap_id;
    SLink *slink;

    unsigned int dfi();
    unsigned int type_dfi();
    DNode *avl_parent();
    DNode *type_avl_parent();
    DNode *type_avl_rhs();
    DNode *type_avl_lhs();
    int type_avl_hcol();
    DNode *postorder_successor();
    int postorder_dfi();
    bool pnode_is_rhs();
    bool type_pnode_is_rhs();
    bool pnode_has_children();
    bool type_pnode_has_children();
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

  class SLink {
  public:
    int smap_id;
    int incoming_links;
  };

  class DFilter {
  public:
    TNode *troot;
    int latest_mod;
    int imaginary_smap_id;
    int last_smap_id;
    unsigned int size;
    struct pavl_table *tbl;
    std::unordered_map<int, struct pavl_table*> type_tables;
    std::unordered_map<int, int> latest_type_mods;
    std::unordered_map<int, int> type_imaginary_smap_ids;
    std::unordered_map<int, DNode*> successor_map;
    //TODO: destructor (can call delete_tree) but must delete type_tables, etc

    DFilter();
    DFilter(TNode *root);
    DNode *avl_root();
    DNode *type_avl_root(int type);
    void generate_index(TNode *root);
    void assign_dnode(TNode *tnode, unsigned int base_index, unsigned int type_base_index, int rhs_offset, int type_rhs_offset);
    struct pavl_table *acquire_type_table(int type);
    int num_nodes_of_type(int type);
    int latest_type_mod(int type);
    int increment_type_mod(int type);
    DNode *avl_insert_between(struct pavl_node *parent, TNode *tnode, struct pavl_node *child);
  };

  class DResult {
  private:
    int type;
    unsigned int mod_num;
    unsigned int type_mod;
    bool first_run = true;
    DFilter *dfilter;
    DNode *first;
    DNode *last;
    DNode *node;
  public:
    DResult(DNode *first, DNode *last, int type); // TODO: make into iterator
    TNode *next();
    bool has_next();
    unsigned int size();
  };

  DNode *pavl_dnode(struct pavl_node *node);
}

#endif
