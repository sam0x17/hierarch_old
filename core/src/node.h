#ifndef HIERARCH_NODE
#define HIERARCH_NODE

#include <hierarch/hierarch.h>

namespace Hierarch {
  class Node;
  class TypeNode;
  class AvlNode;
  class ContextBase;
  class Context;
  class TypeContext;

  class AvlNode {
  public:
    struct avl_node avl; // MUST be first variable
    index_t base_index = 0;
    index_t offset = 0;
    index_t mod = 0;
    AvlNode *successor = NULL;
    std::vector<AvlNode*> predecessors;

    // when we add a (leaf) node, go to its parent's successor, for each predecessor (except for parent),
    //   if predecessor index is greater than the parent's index, change predecessor to point to new node
    //                                          ^ if a descendant of parent

    // when we remove a (leaf) node, set all of its predecessors to point to its successor

    // an up-to-date node has 0 offsets all the way up the avl tree to the avl root

    AvlNode *avl_parent();
    AvlNode *avl_left();
    AvlNode *avl_right();
    Context *context();
    index_t index();
    bool avl_is_left();
    bool avl_is_right();
    Node *node();
    TypeNode *type_node();
  };

  class TypeNode : public AvlNode {
  public:
    type_id_t type_id = 0;
    TypeContext *context();
  };

  class Node : public AvlNode {
  public:
    node_id_t id = 0;
    Node *parent = NULL;
    std::vector<Node> children;
    spp::sparse_hash_map<type_id_t, TypeNode> types;

    bool is_root();
    bool is_leaf();
    bool is_interior();


  };
}

#endif
