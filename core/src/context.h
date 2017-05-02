#ifndef _HIERARCH_CONTEXT
#define _HIERARCH_CONTEXT

#include <hierarch/hierarch.h>

namespace Hierarch {
  bool DEBUG_MODE = true;

  class Context {
  public:
    Node root;
    unsigned int latest_mod = 0;
    unsigned int num_nodes = 0;
    unsigned long num_basic_ops = 0;
    std::vector<int> types;
    // hashtable of node ids
  private:
    void record_basic_op();
  };
}
#endif
