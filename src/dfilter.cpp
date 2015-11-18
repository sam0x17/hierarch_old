#include <iostream>
#include <vector>
#include <queue>
#include <map>

namespace DFI {



  class DNode {
  public:
    int base_index;
    int rhs_offset;
    void *tnode;
    DNode *parent;
    DNode *left_child;
    DNode *right_child;
    DNode *preorder_successor;
  };

  class TNode {
  public:
    TNode *parent;
    DNode *dnode;
    std::vector<TNode*> children;
    int type;
    void *data;
  };

  class DFilter {
    TNode *troot;
    int size;
  public:
    DFilter();
    DFilter(TNode *root);
    void generate_index(TNode *root);
  };

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
    for(TNode *cur_tnode = root; !q.empty(); cur_tnode = q.front(), q.pop()) {
      DNode *cur_dnode = new DNode();
      cur_dnode->base_index = base_index;
      cur_dnode->rhs_offset = 0;
      cur_dnode->tnode = (void *)cur_tnode;
      // add cur_dnode to red-black tree here
      for(TNode *child : cur_tnode->children) {

      }
      this->size++;
      base_index++;
      last_dequeue = cur_dnode;
    }
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

int main() {
  DFI::DNode d;
  DFI::DFilter filter;
  filter.generate_index(NULL);
  //n.base_index = 10.4;
  //printf("hey %f\n", n.base_index);
}
