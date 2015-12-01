#include "dfilter.hpp"

namespace DFI {

  DFilter::DFilter() {
    this->troot = NULL;
    this->size = 0;
  }

  DFilter::DFilter(TNode *root) {
    this->troot = root;
    this->size = 0;
    this->generate_index(root);
  }

  void DFilter::generate_index(TNode *root) {
    std::cout << "Generating index..." << std::endl;
    DNode *last_dequeue = NULL;
    std::queue<TNode*> q;
    int base_index = 0;
    for(TNode *cur_tnode = root; cur_tnode == root || !q.empty(); cur_tnode = q.front(), q.pop()) {
      DNode *cur_dnode = new DNode();
      cur_dnode->base_index = base_index;
      cur_dnode->rhs_offset = 0;
      cur_dnode->tnode = cur_tnode;
      cur_tnode->dnode = cur_dnode;
      if(((TNode *)cur_dnode->tnode)->parent == NULL) {
        cur_dnode->tnode_depth = 0;
      } else {
        cur_dnode->tnode_depth = cur_tnode->parent->dnode->tnode_depth + 1;
      }
      // add cur_dnode to red-black tree here
      for(TNode *child : cur_tnode->children) {
        q.push(child);
      }
      this->size++;
      base_index++;
      last_dequeue = cur_dnode;
    }
    std::cout << "done generating index" << std::endl;
  }

  void dfi_propagate_offset(DNode *node, int offset) {
    int orig_base_index = node->base_index;
    DNode *cur = node;
    for(cur = node; cur->parent != NULL; cur = cur->parent) {
      if(cur->base_index >= orig_base_index) {
        cur->base_index += offset;
        cur->rhs_offset += offset;
      }
    }
  }

}
