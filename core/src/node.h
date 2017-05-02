#ifndef _HIERARCH_NODE
#define _HIERARCH_NODE

#include <hierarch/hierarch.h>

namespace Hierarch {
  class Node {
  public:
    unsigned int index = 0;
    unsigned int mod = 0;
    int offset = 0;
    spp::sparse_hash_set<int> types;
    unsigned int id(); // calculate from memory address within vector!
  };
}
#endif
