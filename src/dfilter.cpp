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
      delete cur;
    }
    root = NULL;
  }

  int compare_dnodes(const void *pa, const void *pb, void *param)
  {
    DNode *nodeA = (DNode *)pa;
    DNode *nodeB = (DNode *)pb;

    if(nodeA->base_index < nodeB->base_index)
      return -1;
    else if (nodeA->base_index > nodeB->base_index)
      return +1;
    else
      return 0;
  }

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

  DFilter::DFilter() {
    this->tbl = pavl_create(compare_dnodes, NULL, &pavl_allocator_default);
    this->troot = NULL;
  }

  DFilter::DFilter(TNode *root) {
    this->tbl = pavl_create(compare_dnodes, NULL, &pavl_allocator_default);
    this->troot = root;
    this->generate_index(root);
  }

  void DFilter::assign_dnode(TNode *tnode, unsigned int base_index, int rhs_offset, unsigned int tnode_depth) {
    DNode *d = tnode->dnode = new DNode();
    std::cout << "base_index: " << base_index << " tnode: " << tnode << std::endl;
    d->base_index = base_index;
    d->rhs_offset = rhs_offset;
    d->tnode_depth = tnode_depth;
    d->tnode = tnode;
    d->pnode = pavl_probe_node(this->tbl, d);
  }

  void DFilter::generate_index(TNode *root) {
    std::cout << "Generating index..." << std::endl;
    std::queue<TNode*> q;
    q.push(root); //TODO: fix this
    int base_index = 0;
    for(TNode *cur_tnode = root; cur_tnode == root || !q.empty(); cur_tnode = q.front(), q.pop()) {
      int tnode_depth = 0;
      if(cur_tnode->parent != NULL) {
        tnode_depth = cur_tnode->parent->dnode->tnode_depth + 1;
      }
      this->assign_dnode(cur_tnode, base_index, 0, tnode_depth);
      std::cout << cur_tnode->dnode->base_index << std::endl;
      for(TNode *child : cur_tnode->children) {
        q.push(child);
      }
      base_index++;
    }
    std::cout << "done generating index" << std::endl;
    std::cout << "size of AVL tree: " << this->tbl->pavl_count << std::endl;
  }

  void dfi_propagate_offset(DNode *node, int offset) {
    int orig_base_index = node->base_index;
    DNode *cur = node;
    for(cur = node; cur->parent() != NULL; cur = cur->parent()) {
      if(cur->base_index >= orig_base_index) {
        cur->base_index += offset;
        cur->rhs_offset += offset;
      }
    }
  }

}
