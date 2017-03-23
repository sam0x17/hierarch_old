
inline void pavl_propagate_ex(struct pavl_node *node) {
  DFI::DNode *dnode = (DFI::DNode *)node->pavl_data;
  if(dnode->ex_dfi_offset != 0) {
    dnode->base_index += dnode->ex_dfi_offset;
    if(node->pavl_link[0] != NULL) { // propagate LHS
      DFI::DNode *dnode_lhs = (DFI::DNode *)node->pavl_link[0]->pavl_data;
      dnode_lhs->ex_dfi_offset += dnode->ex_dfi_offset;
    }
    if(node->pavl_link[1] != NULL) { // propagate RHS
      DFI::DNode *dnode_rhs = (DFI::DNode *)node->pavl_link[1]->pavl_data;
      dnode_rhs->ex_dfi_offset += dnode->ex_dfi_offset;
    }
    dnode->ex_dfi_offset = 0;
  }
}

inline void pavl_propagate_nearby_ex(struct pavl_node *node) {
  if(node->pavl_parent != NULL)
    pavl_propagate_ex(node->pavl_parent);
  if(node->pavl_link[0] != NULL)
    pavl_propagate_ex(node->pavl_link[0]);
  if(node->pavl_link[1] != NULL)
    pavl_propagate_ex(node->pavl_link[1]);
}

/* Inserts |item| into |tree| and returns a pointer to |item|'s node's address.
   If a duplicate item is found in the tree,
   returns a pointer to the duplicate node without inserting |item|.
   Returns |NULL| in case of memory allocation failure. */
struct pavl_node *pavl_probe_node_ex(struct pavl_table *tree, void *item) {
  struct pavl_node *y;     // Top node to update balance factor, and parent.
  struct pavl_node *p, *q; // Iterator, and parent.
  struct pavl_node *n;     // Newly inserted node.
  struct pavl_node *w;     // New root of rebalanced subtree.
  int dir;                 // Direction to descend.

  assert(tree != NULL && item != NULL);

  // SEARCH PAVL TREE FOR INSERTION POINT
  // We search much as before. Despite use of the parent pointers, we preserve
  // the use of q as the parent of p because the termination condition is a value
  // of NULL for p, and NULL has no parent. (Thus, q is not, strictly speaking,
  // always p's parent, but rather the last node examined before p.).
  // Because of parent pointers, there is no need for variable z, used in earlier
  // implementations of AVL insertion to maintain y's parent.
  y = tree->pavl_root;
  for (q = NULL, p = tree->pavl_root; p != NULL; q = p, p = p->pavl_link[dir]) {
    int cmp = tree->pavl_compare(item, p->pavl_data, tree->pavl_param);
    if (cmp == 0)
      return p;
    dir = cmp > 0;
    if (p->pavl_balance != 0)
      y = p;
  }

  // allocate node to be inserted
  n = tree->pavl_alloc->libavl_malloc(tree->pavl_alloc, sizeof *p);
  if (n == NULL)
    return NULL; // allocation error

  // perform insertion
  tree->pavl_count++;
  n->pavl_link[0] = n->pavl_link[1] = NULL;
  n->pavl_parent = q;
  n->pavl_data = item;
  if (q != NULL)
    q->pavl_link[dir] = n;
  else
    tree->pavl_root = n;
  n->pavl_balance = 0;
  if (tree->pavl_root == n)
    return n; // tree was empty, so done now

  // UPDATE BALANCE FACTORS
  // At each step, we need to know the node to update and, for that node, on
  // which side of its parent it is a child. In the code below, q is the node
  // and dir is the side.
  for (p = n; p != y; p = q) {
    q = p->pavl_parent;
    dir = q->pavl_link[0] != p;
    if (dir == 0)
      q->pavl_balance--;
    else
      q->pavl_balance++;
  }

  //pavl_propagate_ex(y);
  pavl_propagate_nearby_ex(y);

  // rebalancing after PAVL insertion
  if (y->pavl_balance == -2) { // insertion was in LEFT subtree
    struct pavl_node *x = y->pavl_link[0];
    pavl_propagate_nearby_ex(x);
    if (x->pavl_balance == -1) { // rebalance for negative balance factor
      /* ROTATE RIGHT AT Y
               Y5
              / \            X2
             X2  B7   =>    / \
            / \            W1  Y5
           W1  A3              / \
                              A3  B7
      */
      // w = x
      // y.left = x.right (A)
      // x.right = y
      // x.parent = y.parent
      // y.parent = x
      w = x;
      y->pavl_link[0] = x->pavl_link[1];
      x->pavl_link[1] = y;
      x->pavl_balance = y->pavl_balance = 0;
      x->pavl_parent = y->pavl_parent;
      y->pavl_parent = x;
      if (y->pavl_link[0] != NULL)
        y->pavl_link[0]->pavl_parent = y;
    } else { // rebalance for positive balance factor
      assert(x->pavl_balance == +1);
      w = x->pavl_link[1];
      x->pavl_link[1] = w->pavl_link[0];
      w->pavl_link[0] = x;
      y->pavl_link[0] = w->pavl_link[1];
      w->pavl_link[1] = y;
      if (w->pavl_balance == -1)
        x->pavl_balance = 0, y->pavl_balance = +1;
      else if (w->pavl_balance == 0)
        x->pavl_balance = y->pavl_balance = 0;
      else /* |w->pavl_balance == +1| */
        x->pavl_balance = -1, y->pavl_balance = 0;
      w->pavl_balance = 0;
      w->pavl_parent = y->pavl_parent;
      x->pavl_parent = y->pavl_parent = w;
      if (x->pavl_link[1] != NULL)
        x->pavl_link[1]->pavl_parent = x;
      if (y->pavl_link[0] != NULL)
        y->pavl_link[0]->pavl_parent = y;
    }
  } else if (y->pavl_balance == +2) { // insertion was in RIGHT subtree
    struct pavl_node *x = y->pavl_link[1];
    pavl_propagate_nearby_ex(x);
    if (x->pavl_balance == +1) { // rebalance for positive balance factor
      // LEFT ROTATION
      w = x;
      y->pavl_link[1] = x->pavl_link[0];
      x->pavl_link[0] = y;
      x->pavl_balance = y->pavl_balance = 0;
      x->pavl_parent = y->pavl_parent;
      y->pavl_parent = x;
      if (y->pavl_link[1] != NULL)
        y->pavl_link[1]->pavl_parent = y;
    } else { // rebalance for negative balance factor
      assert (x->pavl_balance == -1);
      w = x->pavl_link[0];
      x->pavl_link[0] = w->pavl_link[1];
      w->pavl_link[1] = x;
      y->pavl_link[1] = w->pavl_link[0];
      w->pavl_link[0] = y;
      if (w->pavl_balance == +1)
        x->pavl_balance = 0, y->pavl_balance = -1;
      else if (w->pavl_balance == 0)
        x->pavl_balance = y->pavl_balance = 0;
      else /* |w->pavl_balance == -1| */
        x->pavl_balance = +1, y->pavl_balance = 0;
      w->pavl_balance = 0;
      w->pavl_parent = y->pavl_parent;
      x->pavl_parent = y->pavl_parent = w;
      if (x->pavl_link[0] != NULL)
        x->pavl_link[0]->pavl_parent = x;
      if (y->pavl_link[1] != NULL)
        y->pavl_link[1]->pavl_parent = y;
    }
  } else return n; // no rotations needed, done
  // a rotation has occurred

  if (w->pavl_parent != NULL)
    w->pavl_parent->pavl_link[y != w->pavl_parent->pavl_link[0]] = w;
  else
    tree->pavl_root = w;

  return n;
}
