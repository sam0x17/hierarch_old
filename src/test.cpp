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

int main() {
  std::cout << "test suite started" << std::endl;
  std::cout << "generating random tree" << std::endl;
  std::string type_names[] = {"A", "B", "C", "D", "E", "F", "G"};
  int branch_dist[] = {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 4, 5};
  TNode *rootA = generate_random_tree(200, branch_dist, ASIZE(branch_dist), ASIZE(type_names));
  tnode_to_dot(rootA, "bin/output.dot", type_names, ASIZE(type_names));
  std::cout << "done" << std::endl;
  TNode::delete_tree(rootA);
}
