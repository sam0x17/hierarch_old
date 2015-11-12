# include <stdio.h>
# include <memory.h>

struct dfi_node {
  int base_index;
  int rhs_offset;
  void *value;
  void *parent;
  void *left_child;
  void *right_child;
  void *preorder_successor;
} dfi_node;

void dfi_propagate_offset(struct dfi_node *node, int offset) {
  int orig_base_index = node->base_index;
  struct dfi_node *cur = node;
  for(cur = node; cur->parent != NULL; cur = (struct dfi_node *)cur->parent) {
    if(cur->base_index >= orig_base_index) {
      cur->base_index += offset;
      cur->rhs_offset += offset;
    }
  }
}

int main() {
  struct dfi_node n;
  //n.base_index = 10.4;
  //printf("hey %f\n", n.base_index);
}
