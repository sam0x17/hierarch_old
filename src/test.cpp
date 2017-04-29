// contains test and benchmark suite
#include <iostream>
#include "dfilter.hpp"
#include "assert.h"
#include "math.h"
#include <stdlib.h>
#include <queue>
#include <math.h>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <random>
#include <algorithm>
#include <chrono>
std::random_device rd;

using namespace std::chrono;

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

void type_avl_to_dot_no_names(TNode *root, int type, std::string path) {
  std::ofstream file;
  file.open(path);
  file << "digraph {\n";
  std::queue<TNode*> q;
  assert(root->dnode->dfilter->type_avl_root(type) != NULL);
  q.push(root->dnode->dfilter->type_avl_root(type)->tnode);
  while(!q.empty()) {
    TNode *cur = q.front();
    q.pop();
    std::string parent_name = "\"" + itos(cur->dnode->type_dfi()) + " (" + itos(cur->dnode->dfi()) + ")\"";
    TNode *childA = NULL;
    TNode *childB = NULL;
    if(cur->dnode->type_pnode->pavl_link[0] != NULL)
      childA = pavl_dnode(cur->dnode->type_pnode->pavl_link[0])->tnode;
      if(cur->dnode->type_pnode->pavl_link[1] != NULL)
        childB = pavl_dnode(cur->dnode->type_pnode->pavl_link[1])->tnode;
    if(childA != NULL) {
      TNode *child = childA;
      std::string child_name = "\"" + itos(child->dnode->type_dfi()) + " (" + itos(child->dnode->dfi()) + ")\"";
      file << "  " << parent_name << " -> " << child_name << ";\n";
      q.push(child);
    }
    if(childB != NULL) {
      TNode *child = childB;
      std::string child_name = "\"" + itos(child->dnode->type_dfi()) + " (" + itos(child->dnode->dfi()) + ")\"";
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
  std::vector<int> sizes = {1, 2, 3, 10, 20, 40, 80, 200, 400, 800, 1000, 2000, 20000, 100000, 1000000};
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
    //TNode::delete_tree(root);
    std::cout << "[OK]" << std::endl;
  }
}

void test_insertion_stability() {
  std::cout << std::endl;
  std::cout << "testing insertion stability (slow)..." << std::flush;
  std::string type_names[] = {"A", "B", "C", "D", "E"};
  int branch_dist[] = {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 4, 5};
  int num_nodes = 1;
  TNode *rootA = generate_random_tree(num_nodes, branch_dist, ASIZE(branch_dist), ASIZE(type_names));
  DFilter filter = DFilter(rootA);
  for(int i = 0; i < 8000; i++) {
    int dfi = rand_int(0, filter.size - 1);
    TNode *node = filter.get_node(dfi);
    assert(node != NULL);
    int insertion_index = rand_int(0, node->children.size());
    TNode *result = filter.insert(node, insertion_index, i);
    assert(result != NULL);
    int verify_dfi = result->dnode->dfi();
    TNode *verify = filter.get_node(verify_dfi);
    assert(verify != NULL);
    assert(verify == result);
    // verify no duplicate nodes
    std::unordered_set<int> used_dfis;
    bool found_violation = false;
    int current_dfi = -1;
    std::stack<TNode*> s;
    s.push(rootA);
    while(!s.empty()) {
      current_dfi++;
      TNode *node = s.top();
      s.pop();
      for(int c = node->children.size() - 1; c >= 0; c--) {
        // have to move RTL to get correct results with stack-based dfs
        s.push(node->children[c]);
      }
      int dfi = node->dnode->dfi();
      if(used_dfis.find(dfi) != used_dfis.end()) {
        std::cout << "VIOLATION: " << dfi << std::endl;
        found_violation = true;
      }
      if(dfi != current_dfi) {
        std::cout << "INCORRECT OR MISSING DFI" << std::endl;
        exit(0);
      }
      assert(dfi == current_dfi);
      assert(used_dfis.find(node->dnode->dfi()) == used_dfis.end());
      used_dfis.insert(dfi);
    }
    if(found_violation)
      exit(0);
  }
  std::cout << " [OK]" << std::endl;
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
    //TNode::delete_tree(root);
    std::cout << "[OK]" << std::endl;
  }
}

struct naive_result {
  DResult res;
  int count;
};

struct naive_result get_descendants_by_type_naive(TNode *node, int type) {
  // traverse descendants of node taking note of all descendants of type
  int count = 0;
  std::stack<TNode*> s;
  s.push(node);
  DNode *first = NULL;
  DNode *last = NULL;
  while(!s.empty()) {
    TNode *cur = s.top();
    s.pop();
    if(cur->type == type && node != cur) {
      count++;
      if(first == NULL) {
        first = cur->dnode;
      } else {
        last = cur->dnode;
      }
    }
    for(int c = cur->children.size() - 1; c >= 0; c--) {
      TNode *child = cur->children[c];
      s.push(child);
    }
  }
  struct naive_result result;
  result.res = DResult(first, last, type);
  result.count = count;
  return result;
}

void test_get_descendants_by_type() {
  std::cout << "testing get descendants by type... " << std::flush;
  int branch_dist[] = {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 4, 5, 8};
  int num_types = 35;
  int num_nodes = 100000;
  TNode *root = generate_random_tree(num_nodes, branch_dist, ASIZE(branch_dist), num_types);
  DFilter filter = DFilter(root);
  for(int i = 0; i < filter.size; i++) {
    for(int type = 0; type < num_types; type++) {
      TNode *node = filter.get_node(i);
      struct naive_result naive_res = get_descendants_by_type_naive(node, type);
      DResult res = filter.get_descendants_by_type(node, type);
      assert(res.size() == naive_res.count);
      assert(res.first == naive_res.res.first);
      assert(res.last == naive_res.res.last);
    }
  }
  std::cout << "[OK]" << std::endl;
}

void print_tree(TNode *root, int type) { // used to generate the slide images
  tnode_to_dot_no_names_with_successor_links(root, "bin/tnode_tree.dot");
  avl_to_dot_no_names(root, "bin/avl_tree.dot");
  type_avl_to_dot_no_names(root, type, "bin/type_avl_tree.dot");
}

void benchmark_insertion() {
  std::cout << std::endl;
  std::cout << "benchmarking insertion..." << std::endl;
  std::cout << "num_insertions\ttime(ms)" << std::endl;
  int branch_dist[] = {1};
  int num_types = 35;
  for(int num_nodes = 1000; num_nodes <= 100000; num_nodes += 1000) {
    TNode *root = generate_random_tree(1, branch_dist, ASIZE(branch_dist), num_types);
    DFilter filter = DFilter(root);
    std::vector<TNode*> nodes;
    nodes.push_back(root);
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    for(int dfi = 0; dfi < num_nodes; dfi++) {
      TNode *node = nodes[dfi];
      int insertion_index = rand_int(0, node->children.size());
      TNode *result = filter.insert(node, insertion_index, rand_int(0, num_types - 1));
      nodes.push_back(result);
    }
    auto duration = duration_cast<milliseconds>(high_resolution_clock::now() - t1).count();
    std::cout << num_nodes << "\t" << duration << std::endl;
  }
  std::cout << std::endl;
}

void benchmark_get_descendants_by_type() {
  std::cout << std::endl;
  std::cout << "benchmarking get descendants by type..." << std::endl;
  std::cout << "DFilter\t\t\t\tNaive" << std::endl;
  std::cout << "nodes\tavg query time (microseconds)\t\tnodes\tavg query time (microseconds)" << std::endl;
  int branch_dist[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 6, 6, 7, 8};
  int num_types = 35;
  for(int num_nodes = 1000; num_nodes <= 100000; num_nodes += 1000) {
    TNode *root = generate_random_tree(num_nodes, branch_dist, ASIZE(branch_dist), num_types);
    DFilter filter = DFilter(root);
    std::vector<TNode*> nodes;
    for(int dfi = 0; dfi < num_nodes; dfi++) {
      TNode *node = filter.get_node(dfi);
      if(node->children.size() == 0 || node->dnode->postorder_successor_dfi() - node->dnode->dfi() > 20)
        nodes.push_back(filter.get_node(dfi));
    }
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    for(TNode *node : nodes) {
      for(int type = 0; type < num_types; type++)
        DResult result = filter.get_descendants_by_type(node, type);
    }
    auto duration = duration_cast<microseconds>(high_resolution_clock::now() - t1).count();
    int dfilter_duration = (int)duration;
    t1 = high_resolution_clock::now();
    for(TNode *node : nodes) {
      for(int type = 0; type < num_types; type++) {
        get_descendants_by_type_naive(node, type);
      }
    }
    duration = duration_cast<microseconds>(high_resolution_clock::now() - t1).count();
    int naive_duration = (int)duration;
    std::cout << num_nodes << "\t" << dfilter_duration/(double)num_types/(double)nodes.size() << "\t\t" << num_nodes << "\t" << naive_duration/(double)num_types/(double)nodes.size() << std::endl;
  }
  std::cout << std::endl;
}

void benchmark_get_descendants_by_type_root() {
  std::cout << std::endl;
  std::cout << "benchmarking get descendants by type (from root node)..." << std::endl;
  std::cout << "DFilter\t\t\t\tNaive" << std::endl;
  std::cout << "nodes\tavg query time(ms)\t\tnodes\tavg query time(microseconds)" << std::endl;
  int branch_dist[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 7, 8};
  int num_types = 35;
  for(int num_nodes = 1000; num_nodes <= 100000; num_nodes += 1000) {
    TNode *root = generate_random_tree(num_nodes, branch_dist, ASIZE(branch_dist), num_types);
    DFilter filter = DFilter(root);
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    for(int i = 0; i < 100; i++) {
      for(int type = 0; type < num_types; type++)
        DResult result = filter.get_descendants_by_type(root, type);
    }
    auto duration = duration_cast<microseconds>(high_resolution_clock::now() - t1).count();
    int dfilter_duration = (int)duration;
    t1 = high_resolution_clock::now();
    for(int i = 0; i < 100; i++) {
      for(int type = 0; type < num_types; type++)
        get_descendants_by_type_naive(root, type);
    }
    duration = duration_cast<microseconds>(high_resolution_clock::now() - t1).count();
    int naive_duration = (int)duration;
    std::cout << num_nodes << "\t" << dfilter_duration/(double)100.0/(double)num_types << "\t\t" << num_nodes << "\t" << naive_duration/(double)100.0/(double)num_types << std::endl;
  }
  std::cout << std::endl;
}

int main() {
  /*TNode *root = generate_realistic_tree(100, 3);
  DFilter filter = DFilter(root);
  print_tree(root, 2);
  exit(0);*/
  std::cout << "test suite started" << std::endl;
  test_generate_random_tree();
  test_index_generation();
  test_result_iteration();
  test_insertion_stability();
  test_get_descendants_by_type();
  std::cout << std::endl;
  std::cout << "test suite finished." << std::endl;
  std::cout << std::endl;
  std::cout << "benchmark suite started" << std::endl;
  benchmark_get_descendants_by_type();
  benchmark_get_descendants_by_type_root();
  benchmark_insertion();
  std::cout << "benchmark suite finished." << std::endl;
  std::cout << std::endl;
}
