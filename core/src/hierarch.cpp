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
    return ctx;
  }

  void delete_context() {
    assert(ctx != NULL);
    //TODO: deallocate stuff
    contexts.erase(ctx->context_id);
    ctx = NULL;
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
