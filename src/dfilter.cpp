#include "dfilter.hpp"
namespace DFI {

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

  DNode::DNode(DFilter *dfilter, TNode *tnode, unsigned int base_index, int rhs_offset, unsigned int tnode_depth) {
    this->base_index = base_index;
    this->rhs_offset = rhs_offset;
    this->tnode = tnode;
    tnode->dnode = this;
    this->pnode = (struct pavl_node *)pavl_probe(dfilter->tbl, this);
    std::cout << "index: " << this << " " << this->pnode << std::endl;
  }

  DNode *DNode::parent() {
    return (DNode *)(this->pnode->pavl_parent->pavl_data);
  }

  DNode *DNode::left_child() {
    return (DNode *)(this->pnode->pavl_link[0]->pavl_data);
  }

  DNode *DNode::right_child() {
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

  void DFilter::generate_index(TNode *root) {
    std::cout << "Generating index..." << std::endl;
    std::queue<TNode*> q;
    int base_index = 0;
    for(TNode *cur_tnode = root; cur_tnode == root || !q.empty(); cur_tnode = q.front(), q.pop()) {
      int tnode_depth = 0;
      if(cur_tnode->parent != NULL) {
        tnode_depth = cur_tnode->parent->dnode->tnode_depth + 1;
      }
      new DNode(this, cur_tnode, base_index, 0, tnode_depth);
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
