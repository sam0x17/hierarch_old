#include <iostream>
#include "dfilter.hpp"
#include "assert.h"
#include <stdlib.h>
#include <queue>
#include <math.h>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <random>
#include <algorithm>
std::random_device rd;

#define ASIZE(a) (sizeof(a) / sizeof((a)[0]))

using namespace DFI;

std::string itos(int n) {
  std::ostringstream ostream;
  ostream << n;
  return ostream.str();
}

int rand_int(int a, int b) {
  std::default_random_engine el(rd());
  std::uniform_int_distribution<int> uniform_dist(a, b);
  uniform_dist(el);
}

bool maybe() {
  return (bool)rand_int(0, 1);
}

TNode *random_node(int num_types) {
  TNode *node = new TNode();
  node->type = rand_int(0, num_types - 1);
  return node;
}

TNode *generate_random_tree(int n, int branch_dist[], int dist_size, int num_types) {
  TNode *root = random_node(num_types);
  if(n == 1) return root;
  std::queue<TNode*> q;
  q.push(root);
  int total_nodes = 1;
  while(!q.empty()) {
    TNode *cur = q.front();
    q.pop();
    int sn = branch_dist[rand_int(0, dist_size - 1)];
    if(sn == 0)
      if(total_nodes == 1 || q.empty())
        sn = 1;
    for(int i = 0; i < sn; i++) {
      total_nodes++;
      TNode *node = random_node(num_types);
      cur->add_child(node);
      q.push(node);
      if(total_nodes >= n) {
        assert(total_nodes == n);
        return root;
      }
    }
  }
  assert(total_nodes == n);
  return root;
}

void tnode_to_dot(TNode *root, std::string path, std::string type_names[], int num_types) {
  std::ofstream file;
  file.open(path);
  file << "graph {\n";
  std::queue<TNode*> q;
  q.push(root);
  std::unordered_map<TNode*, std::string> node_names;
  std::unordered_map<std::string, int> type_counts;
  node_names[root] = type_names[root->type] + "1";
  type_counts[type_names[root->type]] = 1;
  while(!q.empty()) {
    TNode *cur = q.front();
    q.pop();
    std::string parent_name = node_names[cur];
    for(TNode *child : cur->children) {
      std::string child_name = type_names[child->type];
      auto got = type_counts.find(child_name);
      int name_mod;
      if(got == type_counts.end()) {
        type_counts[child_name] = name_mod = 1;
      } else {
        type_counts[child_name] = name_mod = type_counts[child_name] + 1;
      }
      child_name = child_name + itos(name_mod);
      node_names[child] = child_name;
      file << "  " << parent_name << " -- " << child_name << ";\n";
      q.push(child);
    }
  }
  file << "}\n";
  file.close();
}

void test_generate_random_tree() {
  std::cout << std::endl;
  std::cout << "testing random tree generation..." << std::endl;
  int branch_dist[] = {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 4, 5};
  int num_nodes = 1;
  for(int i = 0; i < 7; i++) {
    TNode *root = generate_random_tree(num_nodes, branch_dist, ASIZE(branch_dist), 50);
    std::cout << num_nodes << " nodes... " << std::flush;
    std::queue<TNode*> q;
    assert(root->parent == NULL);
    q.push(root);
    while(!q.empty()) {
      TNode *node = q.front();
      q.pop();
      if(node->parent != NULL) {
        // node's parent contains node
        assert(std::find(node->parent->children.begin(),
                         node->parent->children.end(),
                         node) != node->parent->children.end());
      }
      for(TNode *child : node->children) {
        assert(node != child);
        assert(child != root);
        assert(child->parent == node);
        q.push(child);
      }
    }
    TNode::delete_tree(root);
    assert(root == NULL);
    std::cout << "[OK]" << std::endl;
    num_nodes *= 10;
  }
}

int main() {
  std::cout << "test suite started" << std::endl;
  std::cout << "generating random tree" << std::endl;
  std::string type_names[] = {"A", "B", "C", "D", "E", "F", "G"};
  int branch_dist[] = {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 4, 5};
  TNode *rootA = generate_random_tree(200, branch_dist, ASIZE(branch_dist), ASIZE(type_names));
  tnode_to_dot(rootA, "bin/output.dot", type_names, ASIZE(type_names));
  std::cout << "generating index..." << std::endl;
  DFilter filter = DFilter(rootA);

  TNode::delete_tree(rootA);
  test_generate_random_tree();
}
