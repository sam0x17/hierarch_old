#ifndef HIERARCH_NODE
#define HIERARCH_NODE

#include <hierarch/hierarch.h>

namespace Hierarch {
  class Node;
  class TypeNode;

  class AvlNode {
  public:
    index_t base_index = 0;
    index_t offset = 0;
    struct avl_node avl;

    AvlNode *avl_parent();
    AvlNode *avl_left();
    AvlNode *avl_right();
    index_t index();
    bool avl_is_right_child();
    bool avl_is_left_child();
    Node *node();
    TypeNode *type_node();
  };

  class TypeNode : public AvlNode {
  public:
    type_id_t type_id = 0;
  };

  class Node : public AvlNode {
  public:
    node_id_t id = 0;
    Node *parent = NULL;
    std::vector<Node> children;
    std::vector<TypeNode> types;

    bool is_root();
    bool is_leaf();
    bool is_interior();
  };
}

#endif
