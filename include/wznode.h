#ifndef LIBWZ_WZNODE_H
#define LIBWZ_WZNODE_H

#include <boost/container/map.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <memory>

#include "wzreader.h"
#include "wztypes.h"

namespace wz {

class WZReader;
class WZNode : public boost::enable_shared_from_this<WZNode> {
   public:
    explicit WZNode(WZNodeType type, std::string identity, uint32_t offset,
                    const boost::shared_ptr<WZReader>& reader)
        : WZNode(type, identity, 0, 0, offset, reader) {}
    explicit WZNode(WZNodeType type, std::string identity, uint32_t size,
                    uint32_t checksum, uint64_t offset,
                    const boost::shared_ptr<WZReader>& reader)
        : _parsed(false),
          _node_type(type),
          _data_type(WZDataType::kNone),
          _identity(identity),
          _offset(offset),
          _parent(nullptr),
          _reader(reader) {}
    virtual ~WZNode() { _reader.reset(); }

    auto ExpandDirectory() -> bool;
    auto ExpandNodes() -> bool;
    auto GetIdentity() -> std::string { return _identity; }
    auto GetDataType() -> WZDataType { return _data_type; }
    auto GetNodeType() -> WZNodeType { return _node_type; }
    auto GetSize() -> uint32_t { return _size; }
    auto GetOffset() -> uint32_t { return _offset; }
    auto GetChecksum() -> uint32_t { return _checksum; }
    auto GetReader() -> boost::shared_ptr<WZReader>& { return _reader; }
    auto GetParent() -> WZNode* { return _parent; }
    auto SetOffset(uint32_t offset) -> void { _offset = offset; }
    auto SetChecksum(uint32_t checksum) -> void { _checksum = checksum; }
    auto SetIdentity(std::string identity) -> void { _identity = identity; }
    auto SetParent(WZNode* parent) -> void { _parent = parent; }
    auto SetSize(uint32_t size) -> void { _size = size; }
    auto IsParsed() -> bool { return _parsed; }

   protected:
   private:
    bool _parsed;
    bool _data_node;
    WZNodeType _node_type;
    WZDataType _data_type;
    std::string _identity;
    uint32_t _offset;
    uint32_t _checksum;
    uint32_t _size;
    WZNodes _nodes;
    WZData _data;
    WZNode* _parent;
    boost::shared_ptr<WZReader> _reader;

    auto ExpandDirectory(uint32_t offset) -> bool;
    auto ExpandNodes(uint32_t offset) -> bool;
    auto ExpandProperty(uint32_t offset) -> bool;
    auto ExpandCanvas(uint32_t offset) -> bool;
    auto ExpandShape2dVector2d(uint32_t offset) -> bool;
    auto ExpandShape2dConvex2d(uint32_t offset) -> bool;
    auto ExpandUol(uint32_t offset) -> bool;
    auto ExpandSound(uint32_t offset) -> bool;
    auto GetNodeTypeByString(const std::string& str) -> WZNodeType;
};

}  // namespace wz

#endif