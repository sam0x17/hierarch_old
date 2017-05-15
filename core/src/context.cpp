
namespace Hierarch {
  ContextBase::ContextBase() {
    avl_init(&this->atree, NULL);
  }

  /*ContextBase::~ContextBase() {
    std::cout << "starting destructor" << std::endl;
    struct avl_node *cur = avl_first(&this->atree);
    AvlNode *node = NULL;
    while(cur) {
      node = _get_entry(cur, AvlNode, avl);
      cur = avl_next(cur);
      avl_remove(&this->atree, &node->avl);
    }
    std::cout << "finished destructor" << std::endl;
  }*/
}
