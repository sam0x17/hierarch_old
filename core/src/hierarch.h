#ifndef _HIERARCH
#define _HIERARCH

#include <iostream>
#include <vector>
#include <cstdlib>

#include <sparsepp/spp.h>
#include <avltree/avltree.h>

#include <hierarch/types.h>
#include <hierarch/node.h>
#include <hierarch/avl_bridge.h>
#include <hierarch/result.h>
#include <hierarch/context.h>

namespace Hierarch {
  std::vector<Context> contexts;
  Context *ctx = NULL;

  type_id_t create_type();
  void delete_type(type_id_t type_id);

  context_id_t create_context();
  void switch_context(context_id_t context_id);
  void delete_context();
  Context *current_context();

  void enable_benchmark();
  void disable_benchmark();
  type_id_t gen_type_id();
  node_id_t gen_node_id();

  bool benchmark = false;
  unsigned long num_basic_ops = 0;
  void basic_op();
}
#endif
