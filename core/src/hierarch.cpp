#include <hierarch/hierarch.h>

namespace Hierarch {
  context_id_t create_context() {
    for(;;) {
      context_id_t id = gen_context_id();
      if(!contexts.contains(id)) {
        Context context;
        context.context_id = id;
        contexts.insert({id, context});
        return id;
      }
      if(contexts.size() >= MAX_CTX_ID) {
        throw "max number of contexts exceeded!";
      }
    }
  }

  context_id_t gen_context_id() { return ctx_dist(rng); }
  type_id_t gen_type_id() { return type_dist(rng); }
  node_id_t gen_node_id() { return node_dist(rng); }

  Context *switch_context(context_id_t context_id) {
    auto got = contexts.find(context_id);
    if(got == contexts.end()) {
      throw "unknown context_id!";
    }
    ctx = &got->second;
    return ctx;
  }

  void delete_context() {
    if(ctx == NULL) throw "tried to erase blank context!";
    contexts.erase(ctx->context_id);
  }

  Context *current_context() { return ctx; }
}

int main() {
  Hierarch::init();
  std::cout << "running tests" << std::endl;
  Hierarch::context_id_t id = Hierarch::create_context();
  HierarchTests::run();
  Hierarch::switch_context(id);
}
