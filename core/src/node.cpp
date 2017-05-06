
namespace Hierarch {
  index_t AvlNode::index() {
    return this->offset + this->base_index;
  }
}
