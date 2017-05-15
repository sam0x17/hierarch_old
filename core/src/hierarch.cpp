#include <hierarch/hierarch.h>

namespace Hierarch {
  void init() {
    if(ran_init) return;
    ran_init = true;
    rng.seed(std::random_device()());
  }

  context_id_t create_context() {
    for(;;) {
      context_id_t id = gen_context_id();
      if(!contexts.contains(id)) {
        Context context;
        context.context_id = id;
        contexts.insert({id, context});
        return id;
      }
      assert(contexts.size() <= MAX_CTX_ID);
    }
  }

  context_id_t gen_context_id() { return ctx_dist(rng); }
  type_id_t gen_type_id() { return type_dist(rng); }
  node_id_t gen_node_id() { return node_dist(rng); }

  Context *switch_context(context_id_t context_id) {
    auto got = contexts.find(context_id);
    assert(got != contexts.end());
    ctx = &(got->second);
    node_cursor = NULL;
    return ctx;
  }

  void delete_context() {
    assert(ctx != NULL);
    ctx->~Context();
    contexts.erase(ctx->context_id);
    ctx = NULL;
    node_cursor = NULL;
  }

  Context *current_context() { return ctx; }

  type_id_t create_type() {
    assert(ctx != NULL);
    for(;;) {
      type_id_t id = gen_type_id();
      if(!ctx->types.contains(id)) {
        TypeContext type_context;
        type_context.type_id = id;
        ctx->types.insert({id, type_context});
        return id;
      }
      assert(ctx->types.size() <= MAX_TYPE_ID);
    }
  }

  void select_node(node_id_t node_id) {
    assert(ctx != NULL);
    assert(ctx->nodes.contains(node_id));
    node_cursor = ctx->nodes[node_id];
    assert(node_cursor->id == node_id);
  }

  node_id_t add_leaf() {
    assert(ctx != NULL);
    // node displacement and insertion
    Node *node = new Node();
    node->id = gen_node_id();
    assert(!ctx->nodes.contains(node->id));
    ctx->nodes.insert({node->id, node});
    assert(ctx->nodes.contains(node->id));
    node->parent = node_cursor;
    Node *parent = node->parent;
    AvlNode *successor = NULL; // displaced node
    if(parent != NULL) { // if there is a parent
      parent->index();
      successor = parent->successor;
    }
    if(successor != NULL) { // if there is a successor
      node->base_index = successor->index();
      successor->displace(+1); // displace successor, since we have taken its index
      assert(parent->mod == ctx->mod);
    } else { // successor is imaginary
      node->base_index = ++ctx->max_index;
      if(parent == NULL) {
        assert(ctx->max_index == 1);
        ctx->max_index = 0;
        node->base_index = 0;
        node->offset = 0;
        assert(node->mod == ctx->mod);
      } else {
        parent->displace(0); // mark all nodes in path from parent to root as up-to-date
        assert(parent->mod == ctx->mod);
      }
    }

    // successor adjustment
    std::vector<AvlNode*> *predecessors;
    if(successor != NULL) {
      predecessors = &successor->predecessors;
      node->successor = successor;
    } else predecessors = &ctx->imaginary_predecessors;
    for(AvlNode *predecessor : *predecessors) // for each predecessor of displaced node
      if(predecessor->index() < node->base_index)
        predecessor->successor = node;
    node->mod = ctx->mod;
    avl_insert(&node->context()->atree, &node->avl, cmp_func); // make avl insertion
    node->index();
    return node->id;
  }

  /*node_id_t add_leaf() {
    Node node_tmp;
    node_tmp.id = gen_node_id();
    ctx->nodes.insert({node_tmp.id, node_tmp});
    Node *node = &ctx->nodes[node_tmp.id];
    node->parent = node_cursor;
    basic_op();
    if(node_cursor != NULL) { // if there is a selected node
      node->parent->apply_offset();
      if(node->parent->successor != NULL) { // if there is a parent successor
        node->parent->successor->applly_offset();
        node->base_index = node->parent->successor->base_index;
        node->successor = node->parent->successor;
        for(AvlNode *predecessor : node->successor->predecessors) { // for each predecessor of parent's successor
          basic_op();
          if(predecessor->index() > node->parent->base_index) { // if a descendant of parent
            predecessor->successor = node;
          }
        }
      } else { // parent successor is imaginary
        node->base_index = ctx->max_index;
      }
    } else {
      assert(ctx->root == NULL); // must make a selection if there is a root
    }
    ctx->max_index += offset;
    return node->id;
  }

  void propagate(AvlNode *changed_node, index_t offset) {
    index_t reference = start_node->index();
    for(AvlNode *cursor = start_node->avl_parent(); cursor != NULL; cursor = cursor->avl_parent()) {
      //if(cursor->index
      cursor->offset += offset;
    }

  }*/

  void apply_type(node_id_t node_id, type_id_t node_type);

  void start_benchmark() {
    std::cout << "starting benchmark" << std::endl;
    num_basic_ops = 0;
    benchmark = true;
  }

  void end_benchmark() {
    std::cout << "benchmark finished" << std::endl;
    std::cout << "\tbasic ops: " << num_basic_ops << std::endl;
    benchmark = false;
  }

  inline void basic_op() {
    if(!benchmark) return;
    num_basic_ops++;
  }
}

int main() {
  Hierarch::init();
  HierarchTests::run();
}
