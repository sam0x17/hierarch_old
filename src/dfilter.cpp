#include "dfilter.hpp"
namespace DFI {

  // user-facing methods (the whole point of this library):

  // returns an DResult iterator (has iterator + count) containing
  // all nodes of the specified type that are descendants of the
  // specified node
  // complexity: O(log(n_t))
  DResult DFilter::get_descendants_by_type(TNode *node, int type) {
    if(node->children.size() == 0)
      return DResult(NULL, NULL, type);
    int start_dfi = node->dnode->dfi();
    int end_dfi = node->dnode->postorder_successor_dfi();
    if(start_dfi >= last_dfi_of_type(type) || end_dfi <= first_dfi_of_type(type))
      return DResult(NULL, NULL, type);
    DNode *start_node = get_bound_node(start_dfi + 1, type, true);
    if(start_node == NULL)
      return DResult(NULL, NULL, type);
    if(start_node->dfi() == end_dfi - 1)
      return DResult(start_node, start_node, type);
    DNode *end_node = get_bound_node(end_dfi - 1, type, false);
    if(end_node == NULL || start_node->dfi() > end_node->dfi())
      return DResult(NULL, NULL, type);
    return DResult(start_node, end_node, type);
  }

  // insert a new tree node at the specified location and update
  // index accordingly
  // complexity: O(T)
  // T = number of nodes between this node and the root
  // params:
  //   parent: the TNode that should be the parent of this new node
  //           if parent is NULL, TNode will be inserted as root
  //   position: the position of the new node in the parent's collection
  //             of child nodes. If a node already exists at that position,
  //             that node will be pushed to the right
  //   type: the type code for the new node
  TNode *DFilter::insert(TNode *parent, int position, int type) {
    TNode *node = new TNode();
    node->type = type;
    node->parent = parent;
    parent->dnode->dfi();
    int displaced_dfi;
    TNode *displaced_node = NULL;
    if(parent == NULL) {
      // new node will be root
      displaced_dfi = 0;
      displaced_node = troot;
      assert(position == 0);
    } else {
      assert(position <= parent->children.size());
      if(parent->children.size() == 0) {
        // parent is a leaf
        displaced_dfi = parent->dnode->base_index + 1;
      } else { // parent has children
        if(position == 0) {
          // new node will be parent's 1st child
          displaced_dfi = parent->dnode->base_index + 1;
          displaced_node = parent->children[0];
          assert(displaced_dfi == parent->children[0]->dnode->dfi());
        } else if(position == parent->children.size()) {
          // new node will be parent's last child
          parent->dnode->cached_successor = NULL; // force successor refresh
          displaced_dfi = parent->dnode->postorder_successor_dfi();
        } else { // new node will be an interior child
          displaced_dfi = parent->children[position]->dnode->dfi();
          displaced_node = parent->children[position];
        }
      }
    }
    if(displaced_node == NULL) {
      displaced_node = get_node(displaced_dfi); // could still be null if last nodeh
    }
    if(displaced_dfi == size)
      assert(displaced_node == NULL);
    else if(displaced_node != NULL)
      displaced_dfi = displaced_node->dnode->dfi();

    // create DNode and update AVL trees
    DNode *d = node->dnode = new DNode();
    d->status = NODE_ALIVE;
    d->base_mod = latest_mod;
    d->type_mod = latest_type_mod(type);
    d->dfilter = this;
    d->base_index = displaced_dfi;
    d->tnode = node;
    if(displaced_node == NULL) {
      assert(displaced_dfi == size);
      // node will become the last node in tree
      d->type_base_index = num_nodes_of_type(type);
      d->cached_successor = NULL;
      d->cached_successor_dfi = size + 1;
      d->pnode = pavl_probe_node(tbl, d);
      d->type_pnode = pavl_probe_node(acquire_type_table(type), d);
    } else {
      // a node will be displaced so the new node can be inserted
      // find closest type node
      TNode *closest_type_node = get_closest_node(displaced_dfi, type);
      d->type_base_index = 0;
      if(closest_type_node != NULL)
        d->type_base_index = closest_type_node->dnode->type_dfi();

      // propagate dfi changes up the tree
      propagate_dfi_change(displaced_node->dnode, +1);
    }

    // modify TNode tree accordingly
    if(parent == NULL) { // new node will be root
      node->add_child(troot);
      troot = node;
    } else if(parent->children.size() > 0) { // parent has children
      if(position == parent->children.size()) {
        parent->children.push_back(node);
      } else {
        parent->children.reserve(parent->children.size() + 1);
        parent->children.insert(parent->children.begin() + position, node);
      }
    } else { // parent is a leaf
      parent->children.push_back(node);
    }
    size++;

    assert(node->parent == parent);
    assert(parent == NULL || parent->children[position] == node);
    assert(node->type == type);
    assert(displaced_dfi >= 0 && displaced_dfi <= size);

    if(d->parent() != NULL)
      d->cached_parent_dfi = d->parent()->dfi();
    DNode *type_parent = d->type_parent(type);
    if(type_parent != NULL)
      d->cached_type_parent_dfi = type_parent->type_dfi();

    d->pnode = pavl_probe_node(tbl, d);
    d->type_pnode = pavl_probe_node(acquire_type_table(type), d); // could be optimized

    if(displaced_node != NULL) {
      d->cached_successor = displaced_node->dnode;
      d->cached_successor_dfi = d->cached_successor->dfi();
    }

    assert(d->pnode != NULL);
    assert(d->type_pnode != NULL);
    assert(d->type_base_index >= 0 && d->type_base_index <= num_nodes_of_type(type));
    assert(d->cached_successor_dfi >= d->dfi());
    return node;
  }

  // internal methods

  // O(log(n_t))
  // modified AVL search that finds either the lowest node of type type
  // greater than dfi or the highest node of type type less than dfi depending
  // on whether gth (greater than) is true or false
  DNode *DFilter::get_bound_node(int dfi, int type, bool gth) {
    DNode *node = type_avl_root(type);
    if(node == NULL)
      return NULL;
    DNode *validA = NULL;
    DNode *validB = NULL;
    int node_dfi = -1;
    while(node != NULL) {
      node_dfi = node->dfi();
      if((gth && node_dfi >= dfi) || (!gth && node_dfi <= dfi)) {
        validB = validA;
        validA = node;
      }
      if(dfi < node_dfi)
        node = node->type_avl_lhs();
      else if(dfi > node_dfi)
        node = node->type_avl_rhs();
      else
        break;
    }
    if(validA != NULL) {
      return validA;
    }
    if(validB != NULL)
      return validB;
    return NULL;
  }

  // O(log(n_type))
  TNode *DFilter::get_closest_node(int dfi, int type) {
    DNode stub;
    stub.base_index = dfi;
    stub.dfilter = NULL;
    struct pavl_node *match = pavl_find_closest_node(acquire_type_table(type), &stub);
    assert(acquire_type_table(type)->pavl_count == 0 || match != NULL);
    if(match == NULL)
      return NULL;
    return pavl_dnode(match)->tnode;
  }

  // O(log(n))
  TNode *DFilter::get_node(int dfi) {
    DNode stub;
    stub.base_index = dfi;
    stub.dfilter = NULL;
    struct pavl_node *match = pavl_find_closest_node(tbl, &stub);
    if(match == NULL)
      return NULL;
    DNode *ret = pavl_dnode(match);
    if(ret->dfi() == dfi)
      return ret->tnode;
    return NULL;
  }

  void TNode::add_child(TNode *child) {
    child->parent = this;
    children.push_back(child);
  }

  void TNode::delete_tree(TNode *&root) {
    DFilter *dfilter = NULL;
    if(root->dnode != NULL)
      dfilter = root->dnode->dfilter;
    std::queue<TNode*> q;
    q.push(root);
    while(!q.empty()) {
      TNode *cur = q.front();
      q.pop();
      for(TNode *child : cur->children)
        q.push(child);
      if(cur->dnode != NULL) {
        assert(cur->dnode->status != NODE_DELETED);
        cur->dnode->status = NODE_DELETED;
        delete cur->dnode;
      }
      delete cur;
    }
    if(dfilter != NULL) {
      if(dfilter->tbl != NULL)
        pavl_destroy(dfilter->tbl, dummy_item_func);
      for(auto kv : dfilter->type_tables) {
        struct pavl_table *table = kv.second;
        pavl_destroy(table, dummy_item_func);
      }
      dfilter->type_tables.clear();
      dfilter->min_type_bounds.clear();
      dfilter->max_type_bounds.clear();
      dfilter->latest_type_mods.clear();
    }
    root = NULL;
  }

  void dummy_item_func(void *pavl_item, void *pavl_param) {}

  int compare_dnodes(const void *pa, const void *pb, void *param)
  {
    DNode *nodeA = (DNode *)pa;
    DNode *nodeB = (DNode *)pb;

    if(nodeA->dfi() < nodeB->dfi())
      return -1;
    else if (nodeA->dfi() > nodeB->dfi())
      return +1;
    else
      return 0;
  }

  // O(T) if base_mod is out of date
  // O(1) if base_mod is up to date
  // updates base_mod if not already up to date
  // T = number of nodes between this node and root
  // in practice, will touch nodes on the way to the root until
  // an up-to-date node is encountered
  int DNode::dfi() {
    assert(this != NULL);
    if(dfilter == NULL)
      return base_index; // used for stubbing
    assert(base_mod <= dfilter->latest_mod);
    if(base_mod < dfilter->latest_mod) {
      if(parent() != NULL) {
        // must update base_index
        base_index += parent()->dfi() - cached_parent_dfi; // "recursive" call
        base_mod = parent()->base_mod;
      } else {
        base_mod = dfilter->latest_mod;
      }
    }
    if(parent() != NULL)
      cached_parent_dfi = parent()->base_index;
    assert(base_mod == dfilter->latest_mod);
    return base_index;
  }

  // O(T) if type_mod is out of date
  // O(1) if base_mod is up to date
  // updates base_mod if not already up to date
  // T = number of nodes between this node and root
  int DNode::type_dfi() {
    assert(this != NULL);
    if(dfilter == NULL)
      return type_base_index; // used for stubbing
    assert(type_mod <= dfilter->latest_type_mod(tnode->type));
    DNode *type_parent = this->type_parent(tnode->type);
    if(type_mod < dfilter->latest_type_mod(tnode->type)) {
      if(type_parent != NULL) {
        // must update type_base_index
        type_base_index += type_parent->type_dfi() - cached_type_parent_dfi; // "recursive" call
        type_mod = type_parent->type_mod;
      } else {
        type_mod = dfilter->latest_type_mod(tnode->type);
      }
      // update type bound pointers
      if(type_base_index == dfilter->num_nodes_of_type(tnode->type) - 1)
        dfilter->max_type_bounds[tnode->type] = this;
      if(type_base_index == 0)
        dfilter->min_type_bounds[tnode->type] = this;
    }
    if(type_parent != NULL)
      cached_type_parent_dfi = type_parent->type_base_index;
    assert(type_mod == dfilter->latest_type_mod(tnode->type));
    return type_base_index;
  }

  void DFilter::propagate_dfi_change(DNode *node, int delta) {
    assert(node != NULL);
    int type = node->tnode->type;
    int orig_base_index = node->dfi();
    int orig_type_base_index = node->type_dfi();
    latest_mod++;
    increment_type_mod(type);
    DNode *prev = NULL;
    DNode *prev_type_node = NULL;
    while(node != NULL) {
      for(TNode *child : node->tnode->children) {
        DNode *child_node = child->dnode;
        if(child_node == prev) continue;
        latest_mod--;
        child_node->dfi(); // force child update on previous mod
        latest_mod++;
        if(child_node->base_index >= orig_base_index) {
          child_node->base_index += delta;
          child_node->base_mod = latest_mod;
        }
        decrement_type_mod(type);
        child_node->type_dfi(); // force child type update on previous mod
        increment_type_mod(type);
        if(child->type == type && child_node->type_base_index >= orig_type_base_index ) {
          child_node->type_base_index += delta;
          child_node->type_mod = latest_type_mod(type);
        }
      }
      if(node->base_index >= orig_base_index) {
        node->base_index += delta;
        if(prev != NULL)
          prev->cached_parent_dfi = node->base_index;
      }
      if(node->tnode->type == type) {
        if(node->type_base_index >= orig_type_base_index) {
          node->type_base_index += delta;
          if(prev_type_node != NULL)
            prev_type_node->cached_type_parent_dfi = node->type_base_index;
        }
        node->type_mod = latest_type_mod(type);
        prev_type_node = node;
      }
      node->base_mod = latest_mod;
      prev = node;
      node = node->parent();
    }
  }

  DNode *pavl_dnode(struct pavl_node *node) {
    assert(node != NULL);
    return (DNode *)node->pavl_data;
  }

  DNode *get_successor_manual(DNode *node) {
    assert(node != NULL);
    // must go up until we can go right
    TNode *cur = node->tnode;
    while(true) {
      if(cur == NULL)
        return NULL;
      TNode *parent = cur->parent;
      if(parent != NULL && cur != parent->children[parent->children.size() - 1]) {
        // if cur is not the rightmost child of parent
        int pos = -1;
        for(int i = 0; i < parent->children.size(); i++) {
          // find position of cur in parent's children list
          TNode *child = parent->children[i];
          if(cur == child) {
            pos = i;
            break;
          }
        }
        assert(pos != parent->children.size() - 1);
        assert(pos != -1);
        // inspect next child to the right
        TNode *next_child = parent->children[pos + 1];
        if(next_child->dnode->dfi() > node->dfi())
          return next_child->dnode;
      }
      cur = parent;
    }
  }

  DNode *DNode::postorder_predecessor() {
    if(parent() == NULL)
      return NULL;
    int prev_child_index;
    for(int i = 0; i < tnode->parent->children.size(); i++) {
      if(tnode->parent->children[i]->dnode == this) {
        prev_child_index = i - 1;
        break;
      }
    }
    if(prev_child_index == -1)
      return parent();
    return tnode->parent->children[prev_child_index]->dnode->postorder_successor();
  }

  DNode *DNode::postorder_successor() {
    postorder_successor_dfi();
    return cached_successor;
  }

  int DNode::postorder_successor_dfi() {
    if(cached_successor != NULL) {
      if(node_deleted(cached_successor)) {
        // if node was just deleted (acceptable hack)
        dfi();
        cached_successor = get_successor_manual(this); // full update required
        if(cached_successor == NULL)
          cached_successor_dfi = dfilter->size;
        else
          cached_successor_dfi = cached_successor->dfi();
        return cached_successor_dfi;
      }
      if(base_mod == dfilter->latest_mod && cached_successor->base_mod == dfilter->latest_mod)
        return cached_successor_dfi; // no update required
    }
    int successor_orig_dfi = cached_successor_dfi;
    int self_orig_dfi = base_index;
    int successor_new_dfi;
    int self_new_dfi;
    if(cached_successor == NULL) {
      if(cached_successor_dfi == dfilter->size)
        return cached_successor_dfi; // no update required
      successor_new_dfi = dfilter->size;
    } else successor_new_dfi = cached_successor->dfi();
    self_new_dfi = dfi();
    if(cached_successor != NULL) {
      if(self_orig_dfi - self_new_dfi == successor_orig_dfi - successor_new_dfi) {
        // if the space between the node and it's successor hasn't changed
        cached_successor_dfi = successor_new_dfi;
        return cached_successor_dfi; // no update required
      }
    }
    // there has been a change between the node and it's successor
    // so we must perform a full update
    dfi();
    cached_successor = get_successor_manual(this); // full update required
    if(cached_successor == NULL)
      cached_successor_dfi = dfilter->size;
    else
      cached_successor_dfi = cached_successor->dfi();
    return cached_successor_dfi;
  }

  DNode *DNode::parent() {
    assert(tnode != NULL);
    if(tnode->parent == NULL)
      return NULL;
    assert(tnode->parent->dnode != NULL);
    return tnode->parent->dnode;
  }

  DNode *DNode::type_parent(int type) {
    assert(tnode != NULL);
    TNode *ancestor = tnode->parent;
    while(ancestor != NULL && ancestor->type != type) {
      ancestor = ancestor->parent;
    }
    if(ancestor == NULL)
      return NULL;
    return ancestor->dnode;
  }

  DNode *DNode::avl_parent() {
    assert(pnode != NULL);
    if(pnode->pavl_parent == NULL)
      return NULL;
    assert(pnode->pavl_parent->pavl_data != NULL);
    return pavl_dnode(pnode->pavl_parent);
  }

  DNode *DNode::type_avl_parent() {
    assert(type_pnode != NULL);
    if(type_pnode->pavl_parent == NULL)
      return NULL;
    assert(type_pnode->pavl_parent->pavl_data != NULL);
    return pavl_dnode(type_pnode->pavl_parent);
  }

  DNode *DNode::type_avl_rhs() {
    assert(type_pnode != NULL);
    if(type_pnode->pavl_link[1] == NULL)
      return NULL;
    assert(type_pnode->pavl_link[1]->pavl_data != NULL);
    return pavl_dnode(type_pnode->pavl_link[1]);
  }

  DNode *DNode::type_avl_lhs() {
    assert(type_pnode != NULL);
    if(type_pnode->pavl_link[0] == NULL)
      return NULL;
    assert(type_pnode->pavl_link[0]->pavl_data != NULL);
    return pavl_dnode(type_pnode->pavl_link[0]);
  }

  DNode *DFilter::first_node_of_type(int type) {
    if(min_type_bounds.find(type) == min_type_bounds.end())
      return NULL;
    return min_type_bounds[type];
  }

  DNode *DFilter::last_node_of_type(int type) {
    if(max_type_bounds.find(type) == max_type_bounds.end())
      return NULL;
    return max_type_bounds[type];
  }

  int DFilter::first_dfi_of_type(int type) {
    DNode *node = first_node_of_type(type);
    if(node == NULL)
      return -1;
    return node->dfi();
  }

  int DFilter::last_dfi_of_type(int type) {
    DNode *node = last_node_of_type(type);
    if(node == NULL)
      return -1;
    return node->dfi();
  }

  bool DNode::pnode_is_rhs() {
    assert(pnode != NULL);
    assert(pnode->pavl_parent != NULL);
    return pnode->pavl_parent->pavl_link[1] == pnode;
  }

  bool DNode::type_pnode_is_rhs() {
    assert(type_pnode != NULL);
    assert(type_pnode->pavl_parent != NULL);
    return type_pnode->pavl_parent->pavl_link[1];
  }

  bool DNode::pnode_has_children() {
    assert(pnode != NULL);
    return pnode->pavl_link[0] != NULL || pnode->pavl_link[1] != NULL;
  }

  bool DNode::type_pnode_has_children() {
    assert(type_pnode != NULL);
    return type_pnode->pavl_link[0] != NULL || type_pnode->pavl_link[1] != NULL;
  }

  DFilter::DFilter() {
    latest_mod = -1;
    tbl = pavl_create(compare_dnodes, NULL, &pavl_allocator_default);
    troot = NULL;
    size = 0;
  }

  DFilter::DFilter(TNode *root) {
    latest_mod = -1;
    tbl = pavl_create(compare_dnodes, NULL, &pavl_allocator_default);
    troot = root;
    size = 0;
    generate_index(root);
  }

  DFilter::~DFilter() {
    if(troot != NULL)
      TNode::delete_tree(troot);
  }

  void DFilter::assign_dnode(TNode *tnode, int base_index, int type_base_index) {
    DNode *d = tnode->dnode = new DNode();
    d->base_mod = latest_mod;
    d->type_mod = latest_type_mod(tnode->type);
    d->dfilter = this;
    d->base_index = base_index;
    d->type_base_index = type_base_index;
    d->cached_parent_dfi = 0;
    d->tnode = tnode;
    tnode->dnode = d;
    size++;

    if(tnode->parent != NULL)
      d->cached_parent_dfi = tnode->parent->dnode->dfi();
    DNode *type_parent = d->type_parent(tnode->type);
    if(type_parent != NULL)
      d->cached_type_parent_dfi = type_parent->type_dfi();

    // set up pavl nodes
    d->pnode = pavl_probe_node(tbl, d);
    d->type_pnode = pavl_probe_node(acquire_type_table(tnode->type), d);
  }

  struct pavl_table *DFilter::acquire_type_table(int type) {
    auto got = type_tables.find(type);
    if(got == type_tables.end())
      return type_tables[type] = pavl_create(compare_dnodes, NULL, &pavl_allocator_default);
    else
      return type_tables[type];
  }

  DNode *DFilter::avl_root() {
    assert(tbl != NULL);
    assert(tbl->pavl_root != NULL);
    return (DNode *)tbl->pavl_root->pavl_data;
  }

  DNode *DFilter::type_avl_root(int type) {
    return pavl_dnode(acquire_type_table(type)->pavl_root);
  }

  int DFilter::latest_type_mod(int type) {
    if(latest_type_mods.find(type) != latest_type_mods.end()) {
      return latest_type_mods[type];
    } else {
      return latest_type_mods[type] = 0;
    }
  }

  int DFilter::increment_type_mod(int type) {
    if(latest_type_mods.find(type) != latest_type_mods.end()) {
      return ++latest_type_mods[type];
    } else {
      return latest_type_mods[type] = 0;
    }
  }

  int DFilter::decrement_type_mod(int type) {
    if(latest_type_mods.find(type) != latest_type_mods.end()) {
      return --latest_type_mods[type];
    } else {
      return latest_type_mods[type] = 0;
    }
  }

  int DFilter::num_nodes_of_type(int type) {
    return (int)(acquire_type_table(type)->pavl_count);
  }

  int generate_index_helper(DFilter *dfilter, TNode *node, int *current_index,
                            std::unordered_map<DNode*, int> *reverse_smap,
                            std::unordered_map<int, DNode*> *node_map) {
    (*current_index)++;
    int base_index = (*current_index);
    int type_base_index = dfilter->num_nodes_of_type(node->type);
    dfilter->assign_dnode(node, *current_index, type_base_index);
    if(node->dnode->type_base_index == dfilter->num_nodes_of_type(node->type) - 1)
      dfilter->max_type_bounds[node->type] = node->dnode;
    if(node->dnode->type_base_index == 0)
      dfilter->min_type_bounds[node->type] = node->dnode;
    int last_index = base_index;
    for(TNode *child : node->children)
      last_index = generate_index_helper(dfilter, child, current_index, reverse_smap, node_map);
    (*reverse_smap)[node->dnode] = last_index + 1;
    assert(base_index == node->dnode->base_index);
    (*node_map)[base_index] = node->dnode;
    return last_index;
  }

  // O(n)
  void DFilter::generate_index(TNode *root) {
    latest_mod = 0;
    size = 0;
    int current_index = -1;
    std::unordered_map<DNode*, int> reverse_smap;
    std::unordered_map<int, DNode*> node_map;
    generate_index_helper(this, root, &current_index, &reverse_smap, &node_map);
    for(auto kv : reverse_smap) {
      DNode *node = kv.first;
      int successor_index = kv.second;
      if(node_map.find(successor_index) != node_map.end()) {
        node->cached_successor = node_map[successor_index];
        assert(successor_index == node->cached_successor->base_index);
      } else {
        assert(successor_index == size);
        node->cached_successor = NULL;
      }
      node->cached_successor_dfi = successor_index;
    }
    assert(latest_mod == 0);
  }

  bool node_deleted(DNode *node) {
    return node == NULL || node->status == NODE_DELETED;
  }

  DResult::DResult() {}

  DResult::DResult(DNode *first, DNode *last, int type) {
    if(first != NULL) {
      if(last == NULL)
        last = first;
      assert(first->dfilter != NULL);
      assert(first->dfilter == last->dfilter);
      assert(type == -1 || first->dfilter->type_tables.find(type) != first->dfilter->type_tables.end());
      assert(first->tnode->type == type);
      assert(last->tnode->type == type);
      // force update first and last nodes
      first->dfi();
      first->type_dfi();
      last->dfi();
      last->type_dfi();
      //assert(first->dfi() <= last->dfi());
      // init
      this->first = first;
      this->last = last;
      this->type = type;
      dfilter = first->dfilter;
      base_mod = dfilter->latest_mod;
      type_mod = dfilter->latest_type_mod(type);
      node = first;
    } else {
      this->first = NULL;
      this->last = NULL;
      this->node = NULL;
    }
  }

  bool DResult::has_next() {
    return node != NULL && node->dfi() <= last->dfi();
  }

  // incrementing to the next element is O(1)
  TNode *DResult::next() {
    if(first == NULL)
      return NULL; // empty result
    assert(base_mod == dfilter->latest_mod);
    assert(type_mod == dfilter->latest_type_mod(type));
    if(last == first) {
      node = NULL;
      return first->tnode;
    }
    // return current iteration item
    DNode *res = node;
    if(node->type_avl_rhs() != NULL) {
      node = node->type_avl_rhs();
      while(node->type_avl_lhs() != NULL)
        node = node->type_avl_lhs();
      return res->tnode;
    }
    while(true) {
      if(node->type_avl_parent() == NULL) {
        node = NULL;
        return res->tnode;
      }
      if(node->type_avl_parent()->type_avl_lhs() == node) {
        node = node->type_avl_parent();
        return res->tnode;
      }
      node = node->type_avl_parent();
    }
  }

  int DResult::size() {
    if(first == NULL)
      return 0; // empty result
    assert(base_mod == dfilter->latest_mod);
    assert(type_mod == dfilter->latest_type_mod(type));
    if(first == last)
      return 1;
    return last->type_dfi() - first->type_dfi() + 1;
  }

}
