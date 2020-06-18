#ifndef LIBWZ_WZNODE_H
#define LIBWZ_WZNODE_H

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <memory>

#include "wzreader.h"

namespace wz {

class WZReader;
class WZNode : boost::enable_shared_from_this<WZNode> {
 public:
  enum class WZNodeType : uint32_t {
    kDirectory,
    kImage,
    kProperty,
  };
  explicit WZNode(WZNodeType node_type, uint64_t offset)
      : WZNode(node_type, offset, nullptr) {}
  explicit WZNode(WZNodeType node_type, uint64_t offset,
                  const boost::shared_ptr<WZReader>& reader)
      : _node_type(node_type), _offset(offset), _reader(reader) {}
  virtual ~WZNode() { _reader.reset(nullptr); }

  WZNode(const WZNode&) = delete;
  WZNode(WZNode&&) = delete;
  WZNode& operator=(const WZNode&) = delete;
  WZNode& operator=(WZNode&&) = delete;

  virtual WZNodeType NodeType() { return _node_type; }

 private:
  WZNodeType _node_type;
  uint64_t _offset;
  boost::shared_ptr<WZReader> _reader;
};
}  // namespace wz

#endif