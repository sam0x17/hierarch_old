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

  const unsigned int NODE_DELETED = 65182332;
  const unsigned int NODE_ALIVE = 93390193;

  int compare_dnodes(const void *pavl_a, const void *pavl_b, void *pavl_param);

  class TNode;
  class DFilter;
  class DNode;
  class DResult;

  class DNode {
  public:
    unsigned int mod_num = 0;
    unsigned int type_mod = 0;
    unsigned int base_index = 0;
    unsigned int type_base_index = 0;
    unsigned int status_code = NODE_ALIVE;
    int rhs_offset = 0;
    int type_rhs_offset = 0;
    TNode *tnode;
    struct pavl_node *pnode;
    struct pavl_node *type_pnode;
    DFilter *dfilter;
    DNode *cached_successor;
    int cached_successor_dfi;

    unsigned int dfi();
    unsigned int type_dfi();
    DNode *avl_parent();
    DNode *type_avl_parent();
    DNode *type_avl_rhs();
    DNode *type_avl_lhs();
    int type_avl_hcol();
    int postorder_successor_dfi();
    DNode *postorder_successor();
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

  class DFilter {
  public:
    TNode *troot;
    int latest_mod = 0;
    unsigned int size;
    struct pavl_table *tbl;
    std::unordered_map<int, struct pavl_table*> type_tables;
    std::unordered_map<int, int> latest_type_mods;
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
    TNode *get_node(int dfi);
    TNode *get_closest_node(int dfi, int type);

    // insert a new tree node at the specified location and update
    // index accordingly
    // complexity: O(log(n))
    // params:
    //   parent: the TNode that should be the parent of this new node
    //           if parent is NULL, TNode will be inserted as root
    //   position: the position of the new node in the parent's collection
    //             of child nodes. If a node already exists at that position,
    //             that node will be pushed to the right
    //   type: the type code for the new node
    void insert(TNode *parent, int position, int type);

    // remove the specified tree node (along with any descendants that node
    // may have), and update the index accordingly
    // complexity: O(log(n)*m) where m is the number of nodes removed
    // params:
    //   node: pointer to the node to remove
    void remove(TNode *node);

    // changes the type of the specified tree node to the specified type and
    // updates the index accordingly
    // complexity: O(log(n_t)) where n_t is the number of nodes of type t in
    //             the tree and t is the the type from (new type, old type) that
    //             has more instantiations in the tree
    // parems:
    //   node: pointer to the node to be replaced
    //   type: the type to change this node to
    void replace(TNode *node, int type);

    DNode *avl_insert_between(struct pavl_node *parent, TNode *tnode, struct pavl_node *child);
    void propogate_dfi_change(DNode *node, int modification);
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
  DNode *get_successor_manual(DNode *node);
  bool node_deleted(DNode *node); // hack
}

#endif
