
namespace Hierarch {
  ContextBase::ContextBase() {
    avl_init(&this->atree, NULL);
  }

  Context::~Context() {
    this->imaginary_predecessors.clear();
    for(auto i : this->nodes) {
      Node *node = i.second;
      delete node;
    }
    this->nodes.clear();
  }

  ContextBase::~ContextBase() {
    this->imaginary_predecessors.clear();
  }
}
