
namespace Hierarch {
  ContextBase::ContextBase() {
    avl_init(&this->atree, NULL);
  }

  Context::~Context() {
    for(auto i : this->nodes) {
      Node *node = i.second;
      delete node;
    }
  }
}
