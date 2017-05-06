#ifndef HIERARCH_TESTS
#define HIERARCH_TESTS

#include <hierarch/hierarch.h>
#include <assert.h>

namespace HierarchTests {
  using namespace Hierarch;

  void pass() { std::cout << "." << std::flush; }

  void test_node_info_index() {
    Node node;
    std::cout << "node address: " << (void *)(&node) << std::endl;
    std::cout << "thingaddress: " << (void *)(&node.avl) << std::endl;
    node.offset = -3;
    node.base_index = 7;
    assert(node.index() == 4);
    node.offset = 2;
    assert(node.index() == 9);
    pass();
  }

  void run() {
    test_node_info_index();
    std::cout << std::endl;
  }
}

#endif
