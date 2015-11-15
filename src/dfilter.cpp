#include <iostream>
#include <vector>

namespace DFI {

  typedef struct DNode {
    int base_index;
    int rhs_offset;
    void *tnode;
    DNode *parent;
    DNode *left_child;
    DNode *right_child;
    DNode *preorder_successor;
  } DNode;

  typedef struct TNode {
    TNode *parent;
    DNode *dnode;
    std::vector<TNode*> children;
    int type;
    void *data;
  } TNode;

  template <class T> class DFilter {
    TNode *troot;
    DNode *droot;
    int size;
  public:
    DFilter();
    DFilter(TNode);
  };

  template <class T> DFilter<T>::DFilter() {
    this->troot = NULL;
    this->droot = NULL;
    this->size = 0;
  }

  template <class T> DFilter<T>::DFilter(TNode root) {
    this->troot = root;
    this->droot = NULL;
    this->size = 0;
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
  //n.base_index = 10.4;
  //printf("hey %f\n", n.base_index);
}
