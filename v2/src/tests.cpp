#include <hierarch/tests.h>

namespace HierarchTests {
  void pass() { std::cout << "OK" << std::endl; }

  void test_node_addressing_issues() {
    std::cout << "test_node_addressing_issues... " << std::flush;
    switch_context(create_context());
    AvlNode *node = new Node();
    assert((void *)(node) == (void *)(&node->avl));
    assert(node->avl_parent() == NULL);
    assert(node->avl_left() == NULL);
    assert(node->avl_right() == NULL);
    avl_insert(&node->context()->atree, &node->avl, cmp_func); // make avl insertion
    assert(node->avl_parent() == NULL);
    assert(node->avl_left() == NULL);
    assert(node->avl_right() == NULL);
    delete node;
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
    assert(ctx->mod == 0);
    select_node(add_leaf());
    assert(ctx->mod == 1);
    node1 = node_cursor;
    assert(node1->offset == 0);
    assert(node1->base_index == 0);
    assert(node1->avl_parent() == NULL);
    assert(node1->successor == NULL);
    select_node(add_leaf());
    assert(ctx->mod == 2);
    node2 = node_cursor;
    assert(node1 != node2);
    assert(node2->base_index == 1);
    assert(node2->offset == 0);
    assert(node2->avl_parent() == node1);
    assert(node1->avl_left() == node2);
    assert(node2->successor == NULL);
    select_node(node1->node()->id);
    assert(node_cursor == node1);
    select_node(add_leaf());
    assert(ctx->mod == 3);
    assert(node1->successor == NULL);
    node3 = node_cursor;
    assert(node3 != node2);
    assert(node1->index() == 0);
    assert(node2->index() == 1);
    assert(node3->index() == 2);
    assert(node3->base_index == 2);
    assert(node3->offset == 0);
    assert(node2->successor == node3);
    select_node(node1->node()->id);
    select_node(add_leaf());
    assert(ctx->mod == 4);
    node4 = node_cursor;
    assert(node4 != node3);
    assert(node4->base_index == 3);
    assert(node4->offset == 0);
    assert(node3->successor == node4);
    assert(node2->successor == node3);
    assert(node1->successor == NULL);
    select_node(node3->node()->id);
    assert(node_cursor == node3);
    select_node(add_leaf());
    assert(ctx->mod == 5);
    node5 = node_cursor;
    assert(node5 != node4);
    assert(node5->node()->parent == node3->node());
    assert(node4->successor == NULL);
    assert(node1->index() == 0);
    assert(node2->index() == 1);
    assert(node3->index() == 2);
    assert(node4->predecessors.size() == 2);
    assert(node3->successor == node4);
    assert(node5->successor == node4);
    assert(node4->index() == 4);
    assert(node5->index() == 3);
    delete_context();
    pass();
  }

  void test_random_node_insertion() {
    std::cout << "test_random_node_insertion... " << std::endl;
    switch_context(create_context());
    std::vector<node_id_t> node_ids;
    for(int i = 0; i < 1000; i++) {
      if(node_ids.size() > 0) {
        select_node(node_ids[rng() % node_ids.size()]);
      }
      node_ids.push_back(add_leaf());
      //if(node_cursor != NULL) std::cout << node_cursor->index() << std::endl;
      //std::cout << node_ids[node_ids.size() - 1] << std::endl;
    }
    delete_context();
    pass();
  }

  void run() {
    test_basic_node_insertion();
    test_node_addressing_issues();
    test_context_manipulation();
    test_context_id_stability();
    test_type_id_stability();
    //test_random_node_insertion();
    std::cout << std::endl;
  }
}
