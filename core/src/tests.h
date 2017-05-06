#ifndef HIERARCH_TESTS
#define HIERARCH_TESTS

#include <hierarch/hierarch.h>
#include <assert.h>

namespace HierarchTests {
  using namespace Hierarch;

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

  void run() {
    test_node_index();
    test_type_node_index();
    std::cout << std::endl;
  }
}

#endif
