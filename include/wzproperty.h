#ifndef LIBWZ_WZ_PROPERTY_H
#define LIBWZ_WZ_PROPERTY_H

#include "wznode.h"

namespace wz {

class WZProperty : public WZNode {
 public:
  virtual WZNodeType NodeType() { return WZNodeType::kProperty; }
};

}  // namespace wz

#endif