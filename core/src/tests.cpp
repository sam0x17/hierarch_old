#include <hierarch/tests.h>

namespace HierarchTests {
  void pass() { std::cout << "." << std::flush; }

  void test_node_index() {
    Node node;
    node.offset = -3;
    node.base_index = 7;
    assert(node.index() == 4);
    node.offset = 2;
    assert(node.index() == 9);
    pass();
  }

  void test_type_node_index() {
    TypeNode node;
    node.offset = -3;
    node.base_index = 7;
    assert(node.index() == 4);
    node.offset = 2;
    assert(node.index() == 9);
    pass();
  }

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

  void run() {
    test_node_index();
    test_type_node_index();
    test_context_manipulation();
    std::cout << std::endl;
  }
}
