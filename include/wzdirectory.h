#ifndef LIBWZ_WZ_DIRECTORY_H
#define LIBWZ_WZ_DIRECTORY_H

#include "wznode.h"

namespace wz {

class WZDirectory : public WZNode {
 public:
  virtual WZNodeType NodeType() { return WZNodeType::kDirectory; }
 private:
};

}  // namespace wz

#endif