#include "dfilter.hpp"
namespace DFI {

  void TNode::add_child(TNode *child) {
    child->parent = this;
    this->children.push_back(child);
  }

  void TNode::delete_tree(TNode *&root) {
    std::queue<TNode*> q;
    std::unordered_set<SLink*> slinks;
    q.push(root);
    while(!q.empty()) {
      TNode *cur = q.front();
      q.pop();
      for(TNode *child : cur->children)
        q.push(child);
      if(cur->dnode != NULL) {
        assert(cur->dnode->slink != NULL);
        slinks.insert(cur->dnode->slink);
        delete cur->dnode;
      }
      delete cur;
    }
    for(SLink *slink : slinks)
      delete slink;
    slinks.clear();
    root = NULL;
  }

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

  DNode *DNode::postorder_successor() {
    assert(this->slink != NULL);
    assert(this->dfilter != NULL);
    return this->dfilter->successor_map[this->slink->smap_id];
  }

  int DNode::postorder_dfi() {
    DNode *successor = postorder_successor();
    if(successor == NULL)
      return dfilter->size;
    return successor->dfi();
  }

  DNode *DNode::avl_parent() {
    assert(this->pnode != NULL);
    if(this->pnode->pavl_parent == NULL)
      return NULL;
    assert(this->pnode->pavl_parent->pavl_data != NULL);
    return pavl_dnode(this->pnode->pavl_parent);
  }

  DNode *DNode::type_avl_parent() {
    assert(this->type_pnode != NULL);
    if(this->type_pnode->pavl_parent == NULL)
      return NULL;
    assert(this->type_pnode->pavl_parent->pavl_data != NULL);
    return pavl_dnode(this->type_pnode->pavl_parent);
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
    assert(this->pnode != NULL);
    assert(this->pnode->pavl_parent != NULL);
    return this->pnode->pavl_parent->pavl_link[1] == this->pnode;
  }

  bool DNode::type_pnode_is_rhs() {
    assert(this->type_pnode != NULL);
    assert(this->type_pnode->pavl_parent != NULL);
    return this->type_pnode->pavl_parent->pavl_link[1];
  }

  bool DNode::pnode_has_children() {
    assert(this->pnode != NULL);
    return this->pnode->pavl_link[0] != NULL || this->pnode->pavl_link[1] != NULL;
  }

  bool DNode::type_pnode_has_children() {
    assert(this->type_pnode != NULL);
    return this->type_pnode->pavl_link[0] != NULL || this->type_pnode->pavl_link[1] != NULL;
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
    if(this->mod_num < this->dfilter->latest_mod) {
      // must update base_index
      assert(this != this->dfilter->avl_root());
      assert(this->pnode != NULL);
      assert(this->pnode->pavl_parent != NULL);
      DNode *avl_parent = this->avl_parent();
      if(this->pnode_is_rhs()) {
        // this is a RHS node (in AVL tree)
        avl_parent->dfi(); // update parent dfi
        this->base_index += avl_parent->rhs_offset;
        if(avl_parent->pnode_has_children())
          this->rhs_offset += avl_parent->rhs_offset;
        avl_parent->rhs_offset = 0;
      } else {
        // this is a LHS node (in AVL tree)
        int orig_parent_index = avl_parent->base_index;
        int diff = avl_parent->dfi() - orig_parent_index;
        this->base_index += diff;
        if(this->pnode_has_children())
          this->rhs_offset += diff;
      }
      this->mod_num = avl_parent->mod_num;
    }
    assert(this->mod_num == this->dfilter->latest_mod);
    return this->base_index;
  }

  // O(log(n)) if type_mod is out of date
  // O(1) if type_mod is up to date
  // updates type_mod if not already up to date
  unsigned int DNode::type_dfi() {
    assert(this != NULL);
    assert(tnode != NULL);
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
    this->imaginary_smap_id = -1;
    this->tbl = pavl_create(compare_dnodes, NULL, &pavl_allocator_default);
    this->troot = NULL;
    this->size = 0;
  }

  DFilter::DFilter(TNode *root) {
    this->imaginary_smap_id = -1;
    this->tbl = pavl_create(compare_dnodes, NULL, &pavl_allocator_default);
    this->troot = root;
    this->size = 0;
    this->generate_index(root);
  }

  void DFilter::assign_dnode(TNode *tnode, unsigned int base_index, unsigned int type_base_index, int rhs_offset, int type_rhs_offset) {
    DNode *d = tnode->dnode = new DNode();
    d->mod_num = this->latest_mod;
    d->type_mod = this->latest_type_mod(tnode->type);
    d->dfilter = this;
    d->base_index = base_index;
    d->type_base_index = type_base_index;
    d->rhs_offset = rhs_offset;
    d->type_rhs_offset = type_rhs_offset;
    d->tnode = tnode;
    tnode->dnode = d;
    d->smap_id = ++this->last_smap_id;
    this->size++;

    // set up pavl nodes
    d->pnode = pavl_probe_node(this->tbl, d);
    d->type_pnode = pavl_probe_node(this->acquire_type_table(tnode->type), d);

    // set up incoming s-link
    this->successor_map[d->smap_id] = d;
  }

  DNode *DFilter::avl_insert_between(struct pavl_node *parent, TNode *tnode, struct pavl_node *child) {
    DNode *d = tnode->dnode = new DNode();
    d->mod_num = ++this->latest_mod;
    d->dfilter = this;
    d->base_index = pavl_dnode(parent)->base_index;
    d->rhs_offset = 1; //TODO: still need to propogate offset up unless it has been propogated already

    //pavl_insert_in_place(struct pavl_table *tree, void *item, struct pavl_node *parent, int dir, struct pavl_node *child);
    return NULL;
  }

  struct pavl_table *DFilter::acquire_type_table(int type) {
    auto got = this->type_tables.find(type);
    if(got == this->type_tables.end())
      return this->type_tables[type] = pavl_create(compare_dnodes, NULL, &pavl_allocator_default);
    else
      return this->type_tables[type];
  }

  DNode *DFilter::avl_root() {
    assert(this->tbl != NULL);
    assert(this->tbl->pavl_root != NULL);
    return (DNode *)this->tbl->pavl_root->pavl_data;
  }

  DNode *DFilter::type_avl_root(int type) {
    return pavl_dnode(acquire_type_table(type)->pavl_root);
  }

  int DFilter::latest_type_mod(int type) {
    if(this->latest_type_mods.find(type) == this->latest_type_mods.end()) {
      return this->latest_type_mods[type];
    } else {
      return this->latest_type_mods[type] = 0;
    }
  }

  int DFilter::increment_type_mod(int type) {
    if(this->latest_type_mods.find(type) == this->latest_type_mods.end()) {
      return ++(this->latest_type_mods[type]);
    } else {
      return this->latest_type_mods[type] = 0;
    }
  }

  int DFilter::num_nodes_of_type(int type) {
    return (int)(this->acquire_type_table(type)->pavl_count);
  }

  int generate_index_helper(DFilter *dfilter, TNode *node, int *current_index, std::unordered_map<DNode*, int> *reverse_smap) {
    (*current_index)++;
    int base_index = (*current_index);
    int type_base_index = dfilter->num_nodes_of_type(node->type);
    dfilter->assign_dnode(node, *current_index, type_base_index, 0, 0);
    dfilter->successor_map[*current_index] = node->dnode; // default sm_ids to base_index
    int last_index = base_index;
    for(TNode *child : node->children)
      last_index = generate_index_helper(dfilter, child, current_index, reverse_smap);
    int postorder_successor_index = last_index + 1;
    (*reverse_smap)[node->dnode] = postorder_successor_index;
    assert(base_index == node->dnode->base_index);
    assert(base_index == node->dnode->smap_id);
    return base_index;
  }
  void DFilter::generate_index(TNode *root) {
    this->imaginary_smap_id = -1;
    this->latest_mod = -1;
    this->last_smap_id = -1;
    this->size = 0;
    int current_index = -1;
    std::unordered_map<DNode*, int> reverse_smap;
    generate_index_helper(this, root, &current_index, &reverse_smap);
    // set up SLinks
    std::unordered_map<int, SLink*> slink_map;
    for(auto kv : reverse_smap) {
      DNode *node = kv.first;
      int smap_id = kv.second;
      SLink *slink;
      // find an existing slink pointing to target node, or create new one
      if(slink_map.find(smap_id) == slink_map.end()) {
        slink = new SLink();
        slink->incoming_links = 0;
        slink->smap_id = smap_id;
        slink_map[smap_id] = slink;
      } else {
        slink = slink_map[smap_id];
        slink->incoming_links++;
      }
      node->slink = slink;
      if(this->successor_map[smap_id] == NULL) {
        assert(this->imaginary_smap_id == -1 || this->imaginary_smap_id == smap_id);
        this->imaginary_smap_id = smap_id;
      }
    }
    slink_map.clear();
    reverse_smap.clear();
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
