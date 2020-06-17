#ifndef LIBWZ_WZNODE_H
#define LIBWZ_WZNODE_H

#include <memory>

namespace wz {
class WZFile;
class WZNode : std::enable_shared_from_this<WZNode> {
   public:
    enum class NodeType : uint32_t {

    };

   private:
};
}  // namespace wz

#endif