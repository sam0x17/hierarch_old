#include <hierarch/hierarch.h>

namespace Hierarch {
  context_id_t create_context() {
    init();
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
}

int main() {
  std::cout << "running tests" << std::endl;
  Hierarch::create_context();
  HierarchTests::run();
}
