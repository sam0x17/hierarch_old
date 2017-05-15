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
    std::cout << "== add sequence started" << std::endl;
    assert(ctx != NULL);
    ctx->max_index++;
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
      std::cout << "there is a parent" << std::endl;
      std::cout << "initial parent index: " << parent->index() << std::endl;
      parent->index();
      successor = parent->successor;
    }
    if(successor != NULL) { // if there is a successor
      std::cout << "there is a successor" << std::endl;
      assert(successor != parent);
      std::cout << "initial successor index: " << successor->index() << std::endl;
      node->base_index = successor->index();
      assert(successor->mod == ctx->mod);
      assert(successor->offset == 0);
      std::cout << "displacing successor index by +1" << std::endl;
      successor->displace(+1); // displace successor, since we have taken its index
      assert(successor->base_index == node->base_index + 1);
      assert(successor->offset == 0);
      assert(successor->mod == ctx->mod);
      std::cout << "successor new index: " << successor->index() << std::endl;
      parent->index();
      assert(parent->mod == ctx->mod);
    } else { // successor is imaginary
      if(parent == NULL) { // first node in empty tree
        assert(ctx->max_index == 1);
        node->base_index = 0;
        node->offset = 0;
        node->mod = ++ctx->mod;
      } else {
        parent->displace(0); // mark all nodes in path from parent to root as up-to-date
        node->base_index = ctx->max_index - 1;
        assert(node->base_index > parent->base_index);
        node->offset = 0;
        node->mod = ctx->mod;
        assert(parent->mod == ctx->mod);
      }
    }

    // perform avl insertion
    node->mod = ctx->mod;
    avl_insert(&node->context()->atree, &node->avl, cmp_func); // make avl insertion
    node->index();
    assert(node->offset == 0);
    if(parent != NULL && node->index() <= parent->index()) {
      std::cout << "node index: " << node->index() << std::endl;
      std::cout << "parent index: " << parent->index() << std::endl;
    }
    if(parent != NULL) assert(node->index() > parent->index());
    if(successor != NULL) assert(node->index() < successor->index());

    // successor adjustment
    std::vector<AvlNode*> *predecessors;
    if(successor != NULL) {
      predecessors = &successor->predecessors;
      node->successor = successor;
    } else predecessors = &ctx->imaginary_predecessors;
    std::vector<AvlNode*> modified_predecessors;
    modified_predecessors.push_back(node);
    index_t parent_index = node->node()->parent_index();
    index_t pred_index;
    for(AvlNode *predecessor : *predecessors) { // for each predecessor of displaced node
      pred_index = predecessor->index();
      if(pred_index > parent_index && pred_index < node->base_index) {
        predecessor->successor = node;
        node->predecessors.push_back(predecessor);
      } else {
        modified_predecessors.push_back(predecessor);
      }
    }
    if(node->successor == NULL) {
      ctx->imaginary_predecessors.clear();
      ctx->imaginary_predecessors = modified_predecessors;
    } else {
      successor->predecessors.clear();
      successor->predecessors = modified_predecessors;
    }
    if(parent != NULL) assert(parent != successor);
    return node->id;
  }

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
