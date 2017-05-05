#ifndef _HIERARCH_NODE
#define _HIERARCH_NODE

#include <hierarch/hierarch.h>

namespace Hierarch {
  class Node;
  class Result;

  class NodeInfo {
  public:
    index_t index = 0;
    index_t offset = 0;
    struct avl_node anode;

    /*Node *avl_parent();
    Node *avl_left();
    Node *avl_right();
    bool avl_is_right_child();
    bool avl_is_left_child();*/
  };

  class TypeInfo : public NodeInfo {
  public:
    type_id_t type_id = 0;
  };

  class Node : public NodeInfo {
  public:
    node_id_t id = 0;
    Node *parent = NULL;
    std::vector<Node*> children;
    std::vector<TypeInfo> types;

    bool is_root();
    bool is_leaf();
    bool is_interior();
  };
}
#endif
