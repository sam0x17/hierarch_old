#include <hierarch/tests.h>

namespace HierarchTests {
  void pass() { std::cout << "OK" << std::endl; }

  void test_node_addressing_issues() {
    switch_context(create_context());
    AvlNode node;
    assert((void *)(&node) == (void *)(&node.avl));
    assert(node.avl_parent() == NULL);
    assert(node.avl_left() == NULL);
    assert(node.avl_right() == NULL);
    avl_insert(&node.context()->atree, &node.avl, cmp_func); // make avl insertion
    assert(node.avl_parent() == NULL);
    assert(node.avl_left() == NULL);
    assert(node.avl_right() == NULL);
    delete_context();
    pass();
  }

  void test_context_manipulation() {
    std::cout << "test_context_manipulation... " << std::flush;
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
    std::cout << "test_context_id_stability... " << std::flush;
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
    std::cout << "test_type_id_stability... " << std::flush;
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

  void test_basic_node_insertion() {
    std::cout << "test_basic_node_insertion... " << std::flush;
    AvlNode *node1 = NULL;
    AvlNode *node2 = NULL;
    AvlNode *node3 = NULL;
    AvlNode *node4 = NULL;
    AvlNode *node5 = NULL;
    context_id_t context_id = create_context();
    switch_context(context_id);
    select_node(add_leaf());
    node1 = node_cursor;
    assert(node1->offset == 0);
    assert(node1->base_index == 0);
    assert(node1->avl_parent() == NULL);
    std::cout << "added node 1 id=" << node1->node()->id << std::endl;
    select_node(add_leaf());
    node2 = node_cursor;
    assert(node1 != node2);
    assert(node2->base_index == 1);
    assert(node2->offset == 0);
    assert(node2->avl_parent() == node1);
    assert(node1->avl_left() == node2);
    select_node(node1->node()->id);
    assert(node_cursor == node1);
    std::cout << "added node 2 id=" << node2->node()->id << std::endl;
    select_node(add_leaf());
    node3 = node_cursor;
    assert(node3 != node2);
    assert(node3->base_index == 2);
    assert(node3->offset == 0);
    select_node(node1->node()->id);
    std::cout << "added node 3" << std::endl;
    select_node(add_leaf());
    node4 = node_cursor;
    assert(node4 != node3);
    assert(node4->base_index == 3);
    assert(node4->offset == 0);
    select_node(node1->node()->id);
    std::cout << "added node 4" << std::endl;
    select_node(node3->node()->id);
    assert(node_cursor == node3);
    select_node(add_leaf());
    node5 = node_cursor;
    assert(node5 != node4);
    assert(node5->index() == 4);
    std::cout << "added node 5" << std::endl;
    delete_context();
    pass();
    std::cout << "moving on to next tests" << std::endl;
  }

  void run() {
    test_basic_node_insertion();
    test_node_addressing_issues();
    /*test_context_manipulation();
    test_context_id_stability();
    test_type_id_stability();*/
    std::cout << std::endl;
  }
}
