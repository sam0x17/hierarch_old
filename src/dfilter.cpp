#include "dfilter.hpp"
namespace DFI {

  // user-facing methods (the whole point of this library):

  void DFilter::insert(TNode *parent, int position, int type) {
    TNode *node = new TNode();
    node->type = type;
    node->parent = parent;
    int displaced_dfi;
    TNode *displaced_node = NULL;
    if(parent == NULL) {
      // new node will be root
      displaced_dfi = 0;
      displaced_node = troot;
      assert(position == 0);
      node->add_child(troot);
      troot = node;
    } else {
      assert(position <= parent->children.size());
      if(parent->children.size() == 0) {
        // parent is a leaf
        displaced_dfi = parent->dnode->dfi() + 1;
        parent->children.push_back(node);
      } else { // parent has children
        if(position == 0) {
          // new node will be parent's 1st child
          displaced_dfi = parent->dnode->dfi() + 1;
          assert(displaced_dfi == parent->children[0]->dnode->dfi());
          displaced_node = parent->children[0];
        } else if(position == parent->children.size()) {
          // new node will be parent's last child
          displaced_dfi = parent->dnode->postorder_successor_dfi();
          if(displaced_dfi < size)
            displaced_node = parent->dnode->postorder_successor()->tnode;
        } else { // new node will be an interior child
          displaced_dfi = parent->children[position]->dnode->dfi();
          displaced_node = parent->children[position];
        }
        // modify TNode tree accordingly
        if(position == parent->children.size()) {
          parent->children.push_back(node);
        } else {
          parent->children.reserve(parent->children.size() + 1);
          parent->children.insert(parent->children.begin() + position, node);
        }
      }
    }
    if(displaced_node == NULL)
      displaced_node = get_node(displaced_dfi); // could still be null if last node

    assert(node->parent == parent);
    assert(parent != NULL || parent->children[position] == node);
    assert(node->type == type);
    assert(displaced_dfi >= 0 && displaced_dfi <= size);
    assert(displaced_dfi >= parent->dnode->dfi());
    assert((displaced_node == NULL && displaced_dfi == size) ||
            displaced_node->dnode->dfi() == displaced_dfi);

    // create DNode and update AVL trees
    DNode *d = node->dnode = new DNode();
    d->mod_num = ++latest_mod;
    d->type_mod = increment_type_mod(type);
    d->dfilter = this;
    d->base_index = displaced_dfi;
    d->type_rhs_offset = 0;
    d->rhs_offset = 0;
    d->tnode = node;
    if(displaced_node == NULL) {
      // node will become the last node in tree
      // no propogation required
      d->type_base_index = num_nodes_of_type(type);
    }
    /*
    d->type_base_index = type_base_index;
    d->rhs_offset = rhs_offset;
    d->type_rhs_offset = type_rhs_offset;
    d->tnode = tnode;
    tnode->dnode = d;
    d->smap_id = ++last_smap_id;
    size++;

    // set up pavl nodes
    d->pnode = pavl_probe_node(tbl, d);
    d->type_pnode = pavl_probe_node(acquire_type_table(tnode->type), d);

    // set up incoming s-link
    successor_map[d->smap_id] = d;*/
    std::cout << "tnode inserted with displacement dfi: " << displaced_dfi << std::endl;
  }

  // O(log(n))
  TNode *DFilter::get_closest_node(int dfi, int type) {
    DNode stub;
    stub.base_index = dfi;
    stub.dfilter = NULL;
    struct pavl_node *match = pavl_find_node(acquire_type_table(type), &stub);
    assert(match != NULL);
    return pavl_dnode(match)->tnode;
  }

  // O(log(n))
  TNode *DFilter::get_node(int dfi) {
    DNode stub;
    stub.base_index = dfi;
    stub.dfilter = NULL;
    struct pavl_node *match = pavl_find_node(tbl, &stub);
    if(match == NULL)
      return NULL;
    return pavl_dnode(match)->tnode;
  }

  void TNode::add_child(TNode *child) {
    child->parent = this;
    children.push_back(child);
  }

  void TNode::delete_tree(TNode *&root) {
    std::queue<TNode*> q;
    q.push(root);
    while(!q.empty()) {
      TNode *cur = q.front();
      q.pop();
      for(TNode *child : cur->children)
        q.push(child);
      if(cur->dnode != NULL) {
        cur->dnode->status_code = NODE_DELETED;
        delete cur->dnode;
        assert(node_deleted(cur->dnode)); // hack
      }
      delete cur;
    }
    root = NULL;
  }

  // internal methods


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

  DNode *pavl_dnode(struct pavl_node *node) {
    assert(node != NULL);
    return (DNode *)node->pavl_data;
  }

  DNode *get_successor_manual(DNode *node) {
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
      if(mod_num == dfilter->latest_mod && cached_successor->mod_num == dfilter->latest_mod)
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
    if(self_orig_dfi - self_new_dfi == successor_orig_dfi - successor_new_dfi) {
      // if the space between the node and it's successor hasn't changed
      cached_successor_dfi = successor_new_dfi;
      return cached_successor_dfi; // no update required
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

  // O(log(n))
  int DNode::type_avl_hcol() {
    int col = 0;
    struct pavl_node *cur = type_pnode;
    while(cur->pavl_parent != NULL) {
      if(cur == cur->pavl_parent->pavl_link[0])
        col--;
      else
        col++;
      cur = cur->pavl_parent;
    }
    return col;
  }

  // O(log(n)) if mod_num is out of date
  // O(1) if mod_num is up to date
  // updates mod_num if not already up to date
  unsigned int DNode::dfi() {
    assert(this != NULL);
    if(dfilter == NULL)
      return base_index; // used for stubbing
    if(mod_num < dfilter->latest_mod) {
      std::cout << "non trivial" << std::endl;
      // must update base_index
      assert(this != dfilter->avl_root());
      assert(pnode != NULL);
      assert(pnode->pavl_parent != NULL);
      DNode *avl_parent = this->avl_parent();
      if(pnode_is_rhs()) {
        // this is a RHS node (in AVL tree)
        avl_parent->dfi(); // update parent dfi
        base_index += avl_parent->rhs_offset;
        if(avl_parent->pnode_has_children())
          rhs_offset += avl_parent->rhs_offset;
        avl_parent->rhs_offset = 0;
      } else {
        // this is a LHS node (in AVL tree)
        int orig_parent_index = avl_parent->base_index;
        int diff = avl_parent->dfi() - orig_parent_index;
        base_index += diff;
        if(pnode_has_children())
          rhs_offset += diff;
      }
      mod_num = avl_parent->mod_num;
    }
    assert(mod_num == dfilter->latest_mod);
    return base_index;
  }

  // O(log(n)) if type_mod is out of date
  // O(1) if type_mod is up to date
  // updates type_mod if not already up to date
  unsigned int DNode::type_dfi() {
    assert(this != NULL);
    if(dfilter == NULL)
      return type_base_index;
    int type = tnode->type;
    if(type_mod < dfilter->latest_type_mod(type)) {
      // must update type_base_index
      assert(type_pnode != NULL);
      DNode *type_avl_parent = this->type_avl_parent();
      assert(type_avl_parent != NULL);
      if(type_pnode_is_rhs()) {
        // this is a RHS node (in AVL tree)
        type_avl_parent->type_dfi(); // update parent dfi
        type_base_index += type_avl_parent->type_rhs_offset;
        if(type_avl_parent->type_pnode_has_children()) // always true though?
          type_rhs_offset += type_avl_parent->type_rhs_offset;
        type_avl_parent->type_rhs_offset = 0;
      } else {
        // this is a LHS node (in AVL tree)
        int orig_parent_index = type_avl_parent->type_base_index;
        int diff = type_avl_parent->type_dfi() - orig_parent_index;
        type_base_index += diff;
        if(type_pnode_has_children())
          type_rhs_offset += diff;
      }
      type_mod = type_avl_parent->type_mod;
    }
    assert(type_mod == dfilter->latest_type_mod(type));
    return type_base_index;
  }

  DFilter::DFilter() {
    tbl = pavl_create(compare_dnodes, NULL, &pavl_allocator_default);
    troot = NULL;
    size = 0;
  }

  DFilter::DFilter(TNode *root) {
    tbl = pavl_create(compare_dnodes, NULL, &pavl_allocator_default);
    troot = root;
    size = 0;
    generate_index(root);
  }

  void DFilter::assign_dnode(TNode *tnode, unsigned int base_index, unsigned int type_base_index, int rhs_offset, int type_rhs_offset) {
    DNode *d = tnode->dnode = new DNode();
    d->mod_num = latest_mod;
    d->type_mod = latest_type_mod(tnode->type);
    d->dfilter = this;
    d->base_index = base_index;
    d->type_base_index = type_base_index;
    d->rhs_offset = rhs_offset;
    d->type_rhs_offset = type_rhs_offset;
    d->tnode = tnode;
    tnode->dnode = d;
    size++;

    // set up pavl nodes
    d->pnode = pavl_probe_node(tbl, d);
    d->type_pnode = pavl_probe_node(acquire_type_table(tnode->type), d);
  }

  DNode *DFilter::avl_insert_between(struct pavl_node *parent, TNode *tnode, struct pavl_node *child) {
    DNode *d = tnode->dnode = new DNode();
    d->mod_num = ++latest_mod;
    d->dfilter = this;
    d->base_index = pavl_dnode(parent)->base_index;
    d->rhs_offset = 1; //TODO: still need to propogate offset up unless it has been propogated already

    //pavl_insert_in_place(tbl, d, , int dir, struct pavl_node *child);
    return NULL;
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
    if(latest_type_mods.find(type) == latest_type_mods.end()) {
      return latest_type_mods[type];
    } else {
      return latest_type_mods[type] = 0;
    }
  }

  int DFilter::increment_type_mod(int type) {
    if(latest_type_mods.find(type) == latest_type_mods.end()) {
      return ++latest_type_mods[type];
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
    dfilter->assign_dnode(node, *current_index, type_base_index, 0, 0);
    int last_index = base_index;
    for(TNode *child : node->children)
      last_index = generate_index_helper(dfilter, child, current_index, reverse_smap, node_map);
    (*reverse_smap)[node->dnode] = last_index + 1;
    assert(base_index == node->dnode->base_index);
    (*node_map)[base_index] = node->dnode;
    return base_index;
  }
  void DFilter::generate_index(TNode *root) {
    latest_mod = -1;
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
  }

  bool node_deleted(DNode *node) {
    return node == NULL || node->status_code == NODE_DELETED;
  }

  DResult::DResult(DNode *first, DNode *last, int type) {
    if(first != NULL) {
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
      assert(first->dfi() <= last->dfi());
      // init
      this->first = first;
      this->last = last;
      this->type = type;
      dfilter = first->dfilter;
      mod_num = dfilter->latest_mod;
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
    assert(mod_num == dfilter->latest_mod);
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

  unsigned int DResult::size() {
    if(first == NULL)
      return 0; // empty result
    assert(mod_num == dfilter->latest_mod);
    assert(type_mod == dfilter->latest_type_mod(type));
    if(first == last)
      return 1;
    return last->type_dfi() - first->type_dfi() + 1;
  }

}
