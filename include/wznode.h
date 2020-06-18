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
      : _parsed(false),
        _node_type(node_type),
        _offset(offset),
        _parent(nullptr),
        _reader(reader) {}
  virtual ~WZNode() { _reader.reset(); }

  WZNode(const WZNode&) = delete;
  WZNode(WZNode&&) = delete;
  WZNode& operator=(const WZNode&) = delete;
  WZNode& operator=(WZNode&&) = delete;

  auto NodeType() -> WZNodeType { return _node_type; }
  auto GetReader() -> boost::shared_ptr<WZReader>& { return _reader; }
  auto GetParent() -> boost::shared_ptr<WZNode>& { return _parent; }
  auto SetParent(boost::shared_ptr<WZNode>& parent) { _parent = parent; }
  auto IsParsed() -> bool { return _parsed; }

 protected:
  virtual void parse();

 private:
  bool _parsed;
  WZNodeType _node_type;
  uint64_t _offset;
  boost::shared_ptr<WZNode> _parent;
  boost::shared_ptr<WZReader> _reader;
};
}  // namespace wz

#endif