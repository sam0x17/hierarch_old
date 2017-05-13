
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

  Context *AvlNode::context() {
    assert(ctx != NULL);
    return ctx;
  }

  TypeContext *TypeNode::context() {
    assert(ctx != NULL);
    return &ctx->types[this->type_id];
  }

  index_t AvlNode::index() {
    basic_op();
    if(this->mod >= this->context()->mod)
      return this->base_index;
    assert(this->avl_parent() != NULL);
    this->avl_parent()->index(); // recursive call
    assert(this->avl_parent()->mod == this->context()->mod); // avl parent should be up-to-date
    assert(this->avl_parent()->offset == 0);
    if(this->avl_left() != NULL)
      this->avl_left()->offset += this->offset;
    if(this->avl_right() != NULL)
      this->avl_right()->offset += this->offset;
    this->base_index += this->offset;
    this->offset = 0;
    this->mod = this->avl_parent()->mod;
    assert(this->avl_parent()->base_index < this->base_index);
    return this->base_index;
  }

  void AvlNode::displace_helper(index_t delta, index_t shift_start, index_t mod) {
    if(this->avl_parent() != NULL)
      this->avl_parent()->displace_helper(delta, shift_start, mod);
    this->offset = 0;
    if(this->base_index > shift_start)
      this->base_index += delta;
    if(this->avl_left() != NULL && this->avl_left()->base_index > shift_start)
      this->avl_left()->offset += delta;
    if(this->avl_right() != NULL && this->avl_right()->base_index > shift_start)
      this->avl_right()->offset += delta;
    this->mod = mod;
  }

  void AvlNode::displace(index_t delta) {
    this->displace_helper(delta, this->index(), ++this->context()->mod);
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
