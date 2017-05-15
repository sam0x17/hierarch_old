#ifndef HIERARCH_CONTEXT
#define HIERARCH_CONTEXT

#include <hierarch/hierarch.h>

namespace Hierarch {
  class ContextBase {
  public:
    struct avl_tree atree;
    index_t min_index = 0;
    index_t max_index = 0;
    index_t mod = 0;
    std::vector<AvlNode*> imaginary_predecessors;
    ContextBase();
  };

  class TypeContext : public ContextBase {
  public:
    type_id_t type_id = 0;
    count_t num_nodes = 0;
  };

  class Context : public ContextBase {
  public:
    context_id_t context_id;
    Node *root = NULL;
    spp::sparse_hash_map<node_id_t, Node*> nodes;
    spp::sparse_hash_map<type_id_t, TypeContext> types;

    ~Context();
  };
}
#endif
