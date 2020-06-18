#ifndef LIBWZ_WZ_IMAGE_H
#define LIBWZ_WZ_IMAGE_H

#include "wznode.h"

namespace wz {

class WZImage : public WZNode {
 public:
  virtual WZNodeType NodeType() { return WZNodeType::kImage; }
};

}  // namespace wz

#endif