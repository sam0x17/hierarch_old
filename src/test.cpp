#include <iostream>
#include "dfilter.hpp"
using namespace DFI;

void tnode_connect(TNode *parent, TNode *child) {
  parent->children.push_back(child);
  child->parent = parent;
}

int main() {
  std::cout << "test started" << std::endl;
  TNode root = TNode();
  TNode nodeA = TNode();
  TNode nodeB = TNode();
  TNode nodeC = TNode();
  TNode nodeD = TNode();
  TNode nodeE = TNode();
  TNode nodeF = TNode();
  TNode nodeG = TNode();
  TNode nodeH = TNode();
  TNode nodeI = TNode();
  TNode nodeJ = TNode();
  TNode nodeK = TNode();
  TNode nodeL = TNode();
  TNode nodeM = TNode();
  root.parent = NULL;
  tnode_connect(&root, &nodeA);
  tnode_connect(&root, &nodeB);
  tnode_connect(&root, &nodeC);
  tnode_connect(&nodeA, &nodeD);
  tnode_connect(&nodeB, &nodeE);
  tnode_connect(&nodeE, &nodeF);
  tnode_connect(&nodeE, &nodeG);
  tnode_connect(&nodeG, &nodeH);
  tnode_connect(&nodeH, &nodeI);
  tnode_connect(&nodeI, &nodeJ);
  tnode_connect(&nodeI, &nodeK);
  tnode_connect(&nodeI, &nodeL);
  tnode_connect(&nodeC, &nodeM);
  std::cout << "children: " << root.children.size() << std::endl;

  DFilter *filter;
  filter = new DFilter(&root);

  free(filter);
}
