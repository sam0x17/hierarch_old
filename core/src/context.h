#ifndef _HIERARCH_CONTEXT
#define _HIERARCH_CONTEXT

#include <hierarch/hierarch.h>

namespace Hierarch {
  class ContextInfo {
  public:
    count_t num_nodes = 0;
    index_t min_index = 0;
    index_t max_index = 0;
    struct avl_tree atree;
  };

  class Context : public ContextInfo {
  public:
    Node *root = NULL;
    spp::sparse_hash_map<node_id_t, Node> nodes;
    spp::sparse_hash_map<type_id_t, ContextInfo> types;
  };
}
#endif
