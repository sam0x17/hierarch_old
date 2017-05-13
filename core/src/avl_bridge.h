#ifndef HIERARCH_AVL_BRIDGE
#define HIERARCH_AVL_BRIDGE

hierarch_index_t avl_node_value(struct avl_node *node) {
  Hierarch::AvlNode *info = _get_entry(node, Hierarch::AvlNode, avl);
  return info->index();
}

extern "C" {
  int cmp_func(struct avl_node *a, struct avl_node *b, void *aux) {
    hierarch_index_t cmp = avl_node_value(a) - avl_node_value(b);
    if(cmp < 0) return -1;
    if(cmp > 0) return 1;
    return 0;
  }
}

#endif
