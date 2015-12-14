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

TNode *rand_child(TNode *node) {
  if(node->children.size() == 0)
    return node;
  return node->children[rand_int(0, node->children.size() - 1)];
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

void tnode_to_dot_no_names_with_successor_links(TNode *root, std::string path) {
  std::ofstream file;
  file.open(path);
  file << "digraph {\n";
  std::queue<TNode *> q;
  q.push(root);
  while(!q.empty()) {
    TNode *cur = q.front();
    q.pop();
    std::string parent_name = "\"" + itos(cur->dnode->dfi()) + " (" + itos(cur->type) + ")\"";
    if(cur->dnode->postorder_successor() == NULL) {
      file << "  " << parent_name << " -> " << "imaginary" << " [constraint=false arrowhead=odiamond style=dashed];\n";
    } else {
      DNode *successor = cur->dnode->postorder_successor();
      std::string successor_name = "\"" + itos(successor->dfi()) + " (" + itos(successor->tnode->type) + ")\"";
      file << "  " << parent_name << " -> " << successor_name << " [constraint=false arrowhead=odiamond style=dashed];\n";
    }
    for(TNode *child : cur->children) {
      std::string child_name = "\"" + itos(child->dnode->dfi()) + " (" + itos(child->type) + ")\"";
      file << "  " << parent_name << " -> " << child_name << ";\n";
      q.push(child);
    }
  }
  file << "}\n";
  file.close();
}

void avl_to_dot_no_names(TNode *root, std::string path) {
  std::ofstream file;
  file.open(path);
  file << "digraph {\n";
  std::queue<TNode*> q;
  q.push(root->dnode->dfilter->avl_root()->tnode);
  while(!q.empty()) {
    TNode *cur = q.front();
    q.pop();
    std::string parent_name = "\"" + itos(cur->dnode->dfi()) + " (" + itos(cur->type) + ")\"";
    TNode *childA = NULL;
    TNode *childB = NULL;
    if(cur->dnode->pnode->pavl_link[0] != NULL)
      childA = pavl_dnode(cur->dnode->pnode->pavl_link[0])->tnode;
      if(cur->dnode->pnode->pavl_link[1] != NULL)
        childB = pavl_dnode(cur->dnode->pnode->pavl_link[1])->tnode;
    if(childA != NULL) {
      TNode *child = childA;
      std::string child_name = "\"" + itos(child->dnode->dfi()) + " (" + itos(child->type) + ")\"";
      file << "  " << parent_name << " -> " << child_name << ";\n";
      q.push(child);
    }
    if(childB != NULL) {
      TNode *child = childB;
      std::string child_name = "\"" + itos(child->dnode->dfi()) + " (" + itos(child->type) + ")\"";
      file << "  " << parent_name << " -> " << child_name << ";\n";
      q.push(child);
    }
  }
  file << "}\n";
  file.close();
}

void tnode_to_dot_no_names(TNode *root, std::string path) {
  std::ofstream file;
  file.open(path);
  file << "graph {\n";
  std::queue<TNode *> q;
  q.push(root);
  while(!q.empty()) {
    TNode *cur = q.front();
    q.pop();
    std::string parent_name = "\"" + itos(cur->dnode->dfi()) + " (" + itos(cur->type) + ")\"";
    for(TNode *child : cur->children) {
      std::string child_name = "\"" + itos(child->dnode->dfi()) + " (" + itos(child->type) + ")\"";
      file << "  " << parent_name << " -- " << child_name << ";\n";
      q.push(child);
    }
  }
  file << "}\n";
  file.close();
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
  std::cout << std::endl;
  std::cout << "testing random tree generation..." << std::endl;
  std::cout << std::endl;
  int branch_dist[] = {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 4, 5};
  int num_nodes = 1;
  for(int i = 0; i < 6; i++) {
    TNode *root = generate_random_tree(num_nodes, branch_dist, ASIZE(branch_dist), 50);
    std::cout << "  " << num_nodes << " nodes... " << std::flush;
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

TNode *generate_realistic_tree(int size, int num_types) {
  // "realistic" branching factor probability distribution
  int branch_dist[] = {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 3, 4, 5};
  return generate_random_tree(size, branch_dist, ASIZE(branch_dist), num_types);
}

void test_index_generation() {
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "testing index generation..." << std::endl;
  std::cout << std::endl;
  std::vector<int> sizes = {1, 2, 3, 10, 20, 40, 80, 200, 400, 800, 1000, 2000, 20000, 100000};
  for(int size : sizes) {
    TNode *root = generate_realistic_tree(size, 20);
    std::cout << "  " << size << " nodes... " << std::flush;
    DFilter filter = DFilter(root);

    // peform fresh index checks
    assert(filter.size == size);
    int type_sum = 0;
    for(auto kv : filter.type_tables) {
      struct pavl_table *type_table = kv.second;
      assert(type_table != NULL);
      type_sum += type_table->pavl_count;
    }
    assert(type_sum == filter.size);
    std::queue<TNode*> q;
    q.push(root);
    //if(size == 200)
    //  tnode_to_dot_no_names_with_successor_links(root, "bin/debug.dot");
    while(!q.empty()) {
      TNode *node = q.front();
      q.pop();
      assert(node->dnode != NULL);
      DNode *dnode = node->dnode;
      assert(dnode->base_index >= 0 && dnode->base_index < size);
      assert(dnode->type_base_index >= 0 && dnode->type_base_index < size);
      assert(dnode->dfi() == dnode->base_index);
      assert(dnode->type_dfi() == dnode->type_base_index);
      assert(dnode->type_dfi() <= dnode->dfi());
      assert(dnode->postorder_successor_dfi() <= filter.size);
      assert(dnode->postorder_successor_dfi() > dnode->dfi());
      DNode *successor = dnode->cached_successor;
      DNode *true_successor = get_successor_manual(dnode);
      assert(successor == true_successor);
      for(TNode *child : node->children)
        q.push(child);
    }
    // test get_node
    for(int dfi = 0; dfi < size; dfi++) {
      TNode *node = filter.get_node(dfi);
      assert(node != NULL);
      assert(node->dnode->dfilter == &filter);
      assert(node->dnode->dfi() == dfi);
    }
    TNode::delete_tree(root);
    std::cout << "[OK]" << std::endl;
  }
}

void test_result_iteration() {
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "testing result iteration..." << std::endl;
  std::cout << std::endl;
  std::vector<int> sizes = {1, 40, 80, 200, 400, 800, 1000, 2000, 20000, 100000};
  for(int size : sizes) {
    TNode *root = generate_realistic_tree(size, 5);
    std::cout << "  " << size << " nodes... " << std::flush;
    DFilter filter = DFilter(root);
    for(int i = 0; i < 100; i++) {
      int type = root->type;
      if(size > 1)
        type = rand_int(0, 4);
      DNode *first = NULL;
      DNode *last = NULL;
      if(filter.num_nodes_of_type(type) == 0)
        continue;
      int first_index = rand_int(0, filter.num_nodes_of_type(type) - 1);
      int last_index = rand_int(first_index, filter.num_nodes_of_type(type) - 1);
      std::queue<DNode*> q;
      assert(filter.type_avl_root(type) != NULL);
      q.push(filter.type_avl_root(type));
      while(!q.empty() && (first == NULL || last == NULL)) {
        DNode *node = q.front();
        q.pop();
        if(node->type_dfi() == first_index)
          first = node;
        if(node->type_dfi() == last_index)
          last = node;
        if(node->type_avl_rhs() != NULL)
          q.push(node->type_avl_rhs());
        if(node->type_avl_lhs() != NULL)
          q.push(node->type_avl_lhs());
      }
      DResult res = DResult(first, last, type);
      // test iterating over result
      assert(res.size() == last->type_dfi() - first->type_dfi() + 1);
      int observed_size = 0;
      while(res.has_next()) {
        TNode *node = res.next();
        assert(node != NULL);
        assert(node->dnode != NULL);
        assert(node->type == type);
        assert(node->dnode->dfi() >= first->dfi());
        assert(node->dnode->dfi() <= last->dfi());
        observed_size++;
      }
      assert(observed_size == res.size());
      DResult empty_res = DResult(NULL, NULL, type);
      assert(empty_res.size() == 0);
      assert(empty_res.has_next() == false);
      assert(empty_res.next() == NULL);
      assert(empty_res.has_next() == false);
      assert(empty_res.next() == NULL);
    }
    TNode::delete_tree(root);
    std::cout << "[OK]" << std::endl;
  }
}

int main() {
  std::cout << "test suite started" << std::endl;
  test_generate_random_tree();
  test_index_generation();
  test_result_iteration();
  //std::cout << "generating random tree" << std::endl;
  std::string type_names[] = {"A", "B", "C", "D", "E"};
  int branch_dist[] = {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 4, 5};
  //tnode_to_dot(rootA, "bin/output.dot", type_names, ASIZE(type_names));

  for(int i = 0; i < 30; i++) {
    TNode *root = generate_random_tree(30, branch_dist, ASIZE(branch_dist), ASIZE(type_names));
    DFilter filter = DFilter(root);
    TNode *node = filter.get_node(i);
    assert(node != NULL);
    std::cout << "selected parent node dfi: " << node->dnode->dfi() << std::endl;
    int insertion_index = rand_int(0, node->children.size());
    tnode_to_dot_no_names(root, "bin/before_insert.dot");
    avl_to_dot_no_names(root, "bin/avl_before_insert.dot");
    TNode *result = filter.insert(node, insertion_index, 1);
    avl_to_dot_no_names(root, "bin/avl_after_insert.dot");
    tnode_to_dot_no_names(root, "bin/after_insert.dot");
    if(result == NULL)
      exit(0);
    // verify no duplicate nodes
    std::queue<TNode*> q;
    std::unordered_set<int> used_dfis;
    q.push(root);
    while(!q.empty()) {
      TNode *node = q.front();
      q.pop();
      std::cout << "visiting " << node->dnode->dfi() << std::endl;
      if(used_dfis.find(node->dnode->dfi()) != used_dfis.end()) {
        std::cout << "VIOLATION: " << node->dnode->dfi() << std::endl;
        exit(0);
      }
      //assert(used_dfis.find(node->dnode->dfi()) == used_dfis.end());
      used_dfis.insert(node->dnode->dfi());
      for(TNode *child : node->children) {
        q.push(child);
      }
    }
    TNode *verify = filter.get_node(result->dnode->dfi());
    assert(verify == result);
    TNode::delete_tree(root);
  }
  std::cout << "done with first set" << std::endl;
  std::cout << std::endl;
  std::cout << std::endl;
  TNode *rootA = generate_random_tree(2000, branch_dist, ASIZE(branch_dist), ASIZE(type_names));
  std::cout << "generating index..." << std::endl;
  DFilter filter = DFilter(rootA);
  for(int i = 0; i < 200000; i++) {
    std::cout << "i: " << i << std::endl;
    int dfi = rand_int(0, filter.size);
    std::cout << "getting node with dfi: " << dfi << std::endl;
    TNode *node = filter.get_node(dfi);
    assert(node != NULL);
    std::cout << "selected parent node dfi: " << node->dnode->dfi() << std::endl;
    int insertion_index = rand_int(0, node->children.size());
    TNode *result = filter.insert(node, insertion_index, 1);
    // verify no duplicate nodes
    std::queue<TNode*> q;
    std::unordered_set<int> used_dfis;
    q.push(rootA);
    while(!q.empty()) {
      TNode *node = q.front();
      q.pop();
      if(used_dfis.find(node->dnode->dfi()) != used_dfis.end()) {
        std::cout << "VIOLATION: " << node->dnode->dfi() << std::endl;
      }
      //assert(used_dfis.find(node->dnode->dfi()) == used_dfis.end());
      used_dfis.insert(node->dnode->dfi());
      for(TNode *child : node->children) {
        q.push(child);
      }
    }
    TNode *verify = filter.get_node(result->dnode->dfi());
    assert(result != NULL);
    std::cout << "result node: " << result->dnode->dfi() << std::endl;
    assert(verify != NULL);
    std::cout << "verify node: " << verify->dnode->dfi() << std::endl;
    assert(verify == result);
    //TNode::delete_tree(rootA);
  }
  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "test suite finished." << std::endl;
  std::cout << std::endl;
}
