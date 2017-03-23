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

  const int NODE_DELETED = 65182332;
  const int NODE_ALIVE = 93390193;

  int compare_dnodes(const void *pavl_a, const void *pavl_b, void *pavl_param);

  class TNode;
  class DFilter;
  class DNode;
  class DResult;

  class DNode {
  public:
    int base_index = 0;
    int type_base_index = 0;
    int base_mod = 0;
    int type_mod = 0;
    int ex_dfi_offset = 0;
    int cached_parent_dfi = 0;
    int cached_type_parent_dfi = 0;
    int cached_successor_dfi = 0;
    int status = NODE_ALIVE;
    TNode *tnode = NULL;
    struct pavl_node *pnode = NULL;
    struct pavl_node *type_pnode = NULL;
    DFilter *dfilter = NULL;
    DNode *cached_successor = NULL;

    int dfi();
    int type_dfi();
    DNode *parent();
    DNode *type_parent(int type);
    DNode *avl_parent();
    DNode *type_avl_parent();
    DNode *type_avl_rhs();
    DNode *type_avl_lhs();
    DNode *type_pnode_rhs();
    DNode *type_pnode_lhs();
    int postorder_successor_dfi();
    DNode *postorder_successor();
    DNode *postorder_predecessor();
    bool pnode_is_rhs();
    bool type_pnode_is_rhs();
    bool pnode_has_children();
    bool type_pnode_has_children();
  };

  class TNode {
  public:
    TNode *parent = NULL;
    DNode *dnode = NULL;
    std::vector<TNode*> children;
    void *data = NULL;
    int type = 0;
    void add_child(TNode *child);
    static void delete_tree(TNode *&root);
  };

  class DFilter {
  public:
    TNode *troot = NULL;
    int latest_mod = 0;
    int size;
    struct pavl_table *tbl = NULL;
    std::unordered_map<int, struct pavl_table*> type_tables;
    std::unordered_map<int, DNode*> max_type_bounds;
    std::unordered_map<int, DNode*> min_type_bounds;
    std::unordered_map<int, int> latest_type_mods;

    DFilter();
    DFilter(TNode *root);
    ~DFilter();
    DNode *avl_root();
    DNode *type_avl_root(int type);
    void generate_index(TNode *root);
    void assign_dnode(TNode *tnode, int base_index, int type_base_index);
    struct pavl_table *acquire_type_table(int type);
    int num_nodes_of_type(int type);
    int latest_type_mod(int type);
    int increment_type_mod(int type);
    int decrement_type_mod(int type);
    TNode *get_node(int dfi);
    TNode *get_closest_node(int dfi, int type);
    DNode *get_bound_node(int dfi, int type, bool gth);
    DNode *first_node_of_type(int type);
    DNode *last_node_of_type(int type);
    int first_dfi_of_type(int type);
    int last_dfi_of_type(int type);

    // returns a DResult iterator (has iterator + count) containing
    // all nodes of the specified type that are descendants of the
    // specified node
    // complexity: O(log(n_t))
    // where n_t is the number of nodes of type t in the tree
    DResult get_descendants_by_type(TNode *node, int type);

    // insert a new tree node at the specified location and update
    // index accordingly
    // complexity: O(T)
    // T = number of nodes between this node and the root
    // params:
    //   parent: the TNode that should be the parent of this new node
    //           if parent is NULL, TNode will be inserted as root
    //   position: the position of the new node in the parent's collection
    //             of child nodes. If a node already exists at that position,
    //             that node will be pushed to the right
    //   type: the type code for the new node
    TNode *insert(TNode *parent, int position, int type);

    void propagate_dfi_change(DNode *node, int delta);
  };

  class DResult {
    int type;
    int base_mod;
    int type_mod;
    bool first_run = true;
    DFilter *dfilter = NULL;
  public:
    DNode *first = NULL;
    DNode *last = NULL;
    DNode *node = NULL;
    DResult();
    DResult(DNode *first, DNode *last, int type); // TODO: make into C++11 iterator
    TNode *next();
    bool has_next();
    int size();
  };

  DNode *pavl_dnode(struct pavl_node *node);
  DNode *get_successor_manual(DNode *node);
  bool node_deleted(DNode *node); // hack
  void dummy_item_func(void *pavl_item, void *pavl_param);
}

#endif
