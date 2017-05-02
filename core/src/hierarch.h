#ifndef _HIERARCH
#define _HIERARCH

#include <iostream>
#include <vector>

#include <sparsepp/spp.h>
#include <hierarch/node.h>
#include <hierarch/context.h>

namespace Hierarch {
  std::vector<Context> contexts;
  int init_context();
  void switch_context(int id);
}
#endif
