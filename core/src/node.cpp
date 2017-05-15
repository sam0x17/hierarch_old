
namespace Hierarch {
  AvlNode::AvlNode() {
    this->avl.left = NULL;
    this->avl.right = NULL;
  }

  AvlNode *AvlNode::avl_parent() {
    assert(this != NULL);
    return (AvlNode *)(_avl_parent(&this->avl));
  }

  AvlNode *AvlNode::avl_left() {
    assert(this != NULL);
    return (AvlNode *)(this->avl.left);
  }

  AvlNode *AvlNode::avl_right() {
    assert(this != NULL);
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

  index_t Node::parent_index() {
    if(this->parent == NULL) return 0;
    return this->parent->index();
  }

  index_t AvlNode::successor_index() {
    assert(this != NULL);
    assert(this->context() != NULL);
    if(this->successor != NULL) return this->successor->index();
    return this->context()->max_index;
  }

  index_t AvlNode::index() {
    assert(this != NULL);
    assert(this->context() != NULL);
    basic_op();
    if(this->mod >= this->context()->mod) {
      assert(this->offset == 0);
      return this->base_index;
    }
    assert(this->avl_parent() != NULL);
    this->avl_parent()->index(); // recursive call
    assert(this->avl_parent()->offset == 0);
    if(this->avl_left() != NULL)
      this->avl_left()->offset += this->offset;
    if(this->avl_right() != NULL)
      this->avl_right()->offset += this->offset;
    this->base_index += this->offset;
    this->offset = 0;
    this->mod = this->avl_parent()->mod;
    assert(this->mod == this->context()->mod);
    return this->base_index;
  }

  void AvlNode::displace_helper(index_t delta, index_t shift_start, index_t mod) {
    assert(this != NULL);
    if(this->avl_parent() != NULL)
      this->avl_parent()->displace_helper(delta, shift_start, mod);
    this->offset = 0;
    if(this->base_index >= shift_start)
      this->base_index += delta;
    if(this->avl_left() != NULL && this->avl_left()->base_index > shift_start)
      this->avl_left()->offset += delta;
    if(this->avl_right() != NULL && this->avl_right()->base_index > shift_start)
      this->avl_right()->offset += delta;
    this->mod = mod;
  }

  void AvlNode::displace(index_t delta) {
    assert(this != NULL);
    index_t index = this->index();
    index_t mod = ++this->context()->mod;
    this->displace_helper(delta, index, mod);
    assert(mod == this->context()->mod);
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
