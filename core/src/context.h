#ifndef HIERARCH_CONTEXT
#define HIERARCH_CONTEXT

#include <hierarch/hierarch.h>

namespace Hierarch {
  class ContextBase {
  public:
    struct avl_tree atree;
  };

  class TypeContext : public ContextBase {
    TypeNode *root;
    count_t num_nodes = 0;
    index_t min_index = 0;
    index_t max_index = 0;
  };

  class Context : public ContextBase {
  public:
    context_id_t context_id;
    Node *root = NULL;
    spp::sparse_hash_map<node_id_t, Node> nodes;
    spp::sparse_hash_map<type_id_t, TypeContext> types;
  };
}
#endif
