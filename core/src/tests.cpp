#include <hierarch/tests.h>

namespace HierarchTests {
  void pass() { std::cout << "." << std::flush; }

  void test_context_manipulation() {
    assert(current_context() == NULL);
    context_id_t id = create_context();
    assert(id > 0);
    assert(current_context() == NULL);
    switch_context(id);
    assert(current_context()->context_id == id);
    switch_context(id);
    assert(current_context() != NULL);
    assert(current_context()->context_id == id);
    context_id_t id2 = create_context();
    assert(id2 != id);
    switch_context(id2);
    assert(current_context() != NULL);
    assert(current_context()->context_id == id2);
    delete_context();
    assert(current_context() == NULL);
    switch_context(id);
    assert(current_context()->context_id == id);
    delete_context();
    assert(current_context() == NULL);
    pass();
  }

  void test_context_id_stability() {
    unsigned int count = 0;
    for(context_id_t i = MIN_CTX_ID; i < MAX_CTX_ID; i++) {
      create_context();
      count++;
    }
    assert(contexts.size() == MAX_CTX_ID);
    assert(contexts.size() == count);
    std::vector<context_id_t> ids;
    for(auto kv : contexts)
      ids.push_back(kv.first);
    for(context_id_t id : ids) {
      switch_context(id);
      assert(ctx != NULL);
      assert(ctx->context_id == id);
      delete_context();
    }
    assert(contexts.size() == 0);
    assert(ctx == NULL);
    pass();
  }

  void test_type_id_stability() {
    unsigned int count = 0;
    context_id_t context_id = create_context();
    switch_context(context_id);
    for(type_id_t i = MIN_TYPE_ID; i < MAX_TYPE_ID; i++) {
      create_type();
      count++;
    }
    assert(ctx->types.size() == MAX_TYPE_ID);
    assert(ctx->types.size() == count);
    std::vector<context_id_t> ids;
    ctx->types.clear();
    delete_context();
    pass();
  }

  void run() {
    test_context_manipulation();
    test_context_id_stability();
    test_type_id_stability();
    std::cout << std::endl;
  }
}
