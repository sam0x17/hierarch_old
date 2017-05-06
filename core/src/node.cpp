
namespace Hierarch {
  AvlNode *AvlNode::avl_parent() {
    return (AvlNode *)(this->avl.parent);
  }

  AvlNode *AvlNode::avl_left() {
    return (AvlNode *)(this->avl.left);
  }
  AvlNode *AvlNode::avl_right() {
    return (AvlNode *)(this->avl.right);
  }

  index_t AvlNode::index() {
    return this->offset + this->base_index;
  }

  bool AvlNode::avl_is_left() {
    AvlNode *parent = this->avl_parent();
    if(parent == NULL)
      return false;
    return this == parent->avl_left();
  }

  bool AvlNode::avl_is_right() {
    AvlNode *parent = this->avl_parent();
    if(parent == NULL)
      return false;
    return this == parent->avl_right();
  }

  Node *AvlNode::node() {
    return (Node *)(this);
  }

  bool Node::is_root() {
    return this->parent == NULL;
  }

  bool Node::is_leaf() {
    return this->children.size() == 0;
  }

  bool Node::is_interior() {
    return !this->is_leaf() && !this->is_root();
  }
}
