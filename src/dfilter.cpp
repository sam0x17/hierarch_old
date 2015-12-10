#include "dfilter.hpp"
namespace DFI {

  void TNode::add_child(TNode *child) {
    child->parent = this;
    this->children.push_back(child);
  }

  void TNode::delete_tree(TNode *&root) {
    std::queue<TNode*> q;
    q.push(root);
    while(!q.empty()) {
      TNode *cur = q.front();
      q.pop();
      for(TNode *child : cur->children)
        q.push(child);
      if(cur->dnode != NULL)
        delete cur->dnode;
      delete cur;
    }
    root = NULL;
  }

  int compare_dnodes(const void *pa, const void *pb, void *param)
  {
    DNode *nodeA = (DNode *)pa;
    DNode *nodeB = (DNode *)pb;

    if(nodeA->dfi() < nodeB->dfi())
      return -1;
    else if (nodeA->dfi() > nodeB->dfi())
      return +1;
    else
      return 0;
  }

  DNode *DNode::avl_parent() {
    assert(this->pnode != NULL);
    assert(this->pnode->pavl_parent != NULL);
    assert(this->pnode->pavl_parent->pavl_data != NULL);
    return (DNode *)this->pnode->pavl_parent->pavl_data;
  }

  bool DNode::pnode_is_rhs() {
    assert(this->pnode != NULL);
    assert(this->pnode->pavl_parent != NULL);
    return this->pnode->pavl_parent->pavl_link[1] == this->pnode;
  }

  bool DNode::pnode_has_children() {
    assert(this->pnode != NULL);
    return this->pnode->pavl_link[0] != NULL || this->pnode->pavl_link[1] != NULL;
  }

  // O(log(n)) if mod_num is out of date
  // O(1) if mod_num is up to date
  // updates mod_num if not already up to date
  unsigned int DNode::dfi() {
    if(this->mod_num < this->dfilter->latest_mod) {
      // must update base_index
      std::cout << "method call" << std::endl;
      assert(this != this->dfilter->avl_root());
      assert(this->pnode != NULL);
      assert(this->pnode->pavl_parent != NULL);
      DNode *avl_parent = this->avl_parent();
      if(this->pnode_is_rhs()) {
        std::cout << "RHS" << std::endl;
        // this is a RHS node (in AVL tree)
        avl_parent->dfi(); // update parent dfi
        this->base_index += avl_parent->rhs_offset;
        if(avl_parent->pnode_has_children()) {
          this->rhs_offset += avl_parent->rhs_offset;
          std::cout << "has children" << std::endl;
        }
        avl_parent->rhs_offset = 0;
      } else {
        // this is a LHS node (in AVL tree)
        std::cout << "LHS" << std::endl;
        int orig_parent_index = avl_parent->base_index;
        int diff = avl_parent->dfi() - orig_parent_index;
        this->base_index += diff;
        if(this->pnode_has_children()) {
          this->rhs_offset += diff;
          std::cout << "has children" << std::endl;
        }
      }
      this->mod_num = avl_parent->mod_num;
    }
    assert(this->mod_num == this->dfilter->latest_mod);
    return this->base_index;
  }

/*
  DNode *DNode::parent() {
    if(this->pnode == NULL || this->pnode->pavl_parent == NULL) return NULL;
    return (DNode *)(this->pnode->pavl_parent->pavl_data);
  }

  DNode *DNode::left_child() {
    if(this->pnode == NULL || this->pnode->pavl_link[0] == NULL) return NULL;
    return (DNode *)(this->pnode->pavl_link[0]->pavl_data);
  }

  DNode *DNode::right_child() {
    if(this->pnode == NULL || this->pnode->pavl_link[1] == NULL) return NULL;
    return (DNode *)(this->pnode->pavl_link[1]->pavl_data);
  }
*/

  DFilter::DFilter() {
    this->tbl = pavl_create(compare_dnodes, NULL, &pavl_allocator_default);
    this->troot = NULL;
  }

  DFilter::DFilter(TNode *root) {
    this->tbl = pavl_create(compare_dnodes, NULL, &pavl_allocator_default);
    this->troot = root;
    this->generate_index(root);
  }

  void DFilter::assign_dnode(TNode *tnode, unsigned int base_index, int rhs_offset) {
    DNode *d = tnode->dnode = new DNode();
    d->mod_num = this->latest_mod;
    d->dfilter = this;
    d->base_index = base_index;
    d->rhs_offset = rhs_offset;
    d->tnode = tnode;
    tnode->dnode = d;
    d->pnode = pavl_probe_node(this->tbl, d);
    d->type_pnode = pavl_probe_node(this->acquire_type_table(tnode->type), d);
  }

  struct pavl_table *DFilter::acquire_type_table(int type) {
    auto got = this->type_tables.find(type);
    if(got == type_tables.end())
      return this->type_tables[type] = pavl_create(compare_dnodes, NULL, &pavl_allocator_default);
    else
      return this->type_tables[type];
  }

  DNode *DFilter::avl_root() {
    assert(this->tbl != NULL);
    assert(this->tbl->pavl_root != NULL);
    return (DNode *)this->tbl->pavl_root->pavl_data;
  }

  void DFilter::generate_index(TNode *root) {
    this->latest_mod = 0;
    std::cout << "Generating index..." << std::endl;
    std::queue<TNode*> q;
    q.push(root);
    int base_index = 0;
    DNode *target_node = NULL;
    while(!q.empty()) {
      TNode *cur_tnode = q.front();
      q.pop();
      /*int tnode_depth = 0;
      if(cur_tnode->parent != NULL) {
        tnode_depth = cur_tnode->parent->dnode->tnode_depth + 1;
      }*/
      this->assign_dnode(cur_tnode, base_index, 0);
      std::cout << cur_tnode->dnode->base_index << std::endl;
      for(TNode *child : cur_tnode->children) {
        q.push(child);
      }
      base_index++;
      if(base_index == 150)
        target_node = cur_tnode->dnode;
    }
    std::cout << "done generating index" << std::endl;
    std::cout << "size of AVL tree: " << this->tbl->pavl_count << std::endl;
    std::cout << "number of avl type trees: " << this->type_tables.size() << std::endl;
    std::cout << "original index: " << target_node->dfi() << std::endl;
    this->avl_root()->mod_num = 1;
    this->avl_root()->base_index += 1;
    this->avl_root()->rhs_offset += 1;
    this->latest_mod = 1;
    std::cout << "result of changing index: " << target_node->dfi() << std::endl;
    std::cout << "result of changing index: " << target_node->dfi() << std::endl;
  }

  void dfi_propagate_offset(DNode *node, int offset) {
    int orig_base_index = node->base_index;
    DNode *cur = node;
    for(cur = node; cur->tnode->parent != NULL; cur = cur->tnode->parent->dnode) {
      if(cur->base_index >= orig_base_index) {
        cur->base_index += offset;
        cur->rhs_offset += offset;
      }
    }
  }

}
