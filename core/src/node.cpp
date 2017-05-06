
namespace Hierarch {
  index_t NodeInfo::index() {
    return this->offset + this->base_index;
  }
}
