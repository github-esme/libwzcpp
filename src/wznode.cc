#include "wznode.h"

#include <iostream>
namespace wz {

auto WZNode::ExpandDirectory() -> bool {
    if (_node_type != WZNodeType::kDirectory) return false;
    boost::container::vector<WZNode> nodes;
    {
        _reader->SetPosition(_offset);
        boost::unique_lock<boost::mutex> locked(_reader->GetLock());
        uint32_t nodes_count = _reader->ReadCompressedInt();
        for (auto i = 0u; i < nodes_count; i++) {
            std::string identity = "";
            auto node_type = static_cast<WZNodeType>(_reader->Read<int8_t>());
            auto block_size = 0;
            auto block_checksum = 0;
            auto block_offset = 0;
            switch (node_type) {
                case WZNodeType::kDirectoryWithOffset:
                case WZNodeType::kImageWithOffset:
                    _reader->SetPosition(_reader->GetHeader().size +
                                         _reader->Read<uint32_t>());
                    node_type =
                        static_cast<WZNodeType>(_reader->Read<int8_t>());
                    assert(node_type == WZNodeType::kDirectory ||
                           node_type == WZNodeType::kImage);
                    identity = _reader->ReadStringXoredWithFactor();
                    break;
                case WZNodeType::kDirectory:
                case WZNodeType::kImage:
                    identity = _reader->ReadStringXoredWithFactor();
                    break;
                default:
                    return false;
            }
            block_size = _reader->ReadCompressedInt();
            block_checksum = _reader->ReadCompressedInt();
            block_offset = _reader->ReadNodeOffset();
            nodes.emplace_back(WZNode(node_type, identity, block_size,
                                      block_checksum, block_offset, _reader));
        }
    }
    for (auto& node : nodes) {
        std::cout << node.GetIdentity()
                  << " type: " << static_cast<uint32_t>(node.GetNodeType())
                  << std::endl;
        auto iterator = _nodes.emplace(node.GetIdentity(), node);
        if (node.GetNodeType() == WZNodeType::kDirectory)
            iterator.first->second.ExpandDirectory();
        else
            iterator.first->second.ExpandNodes();
    }
    return true;
}

auto WZNode::ExpandNodes() -> bool {
    _reader->SetPosition(_offset);
    auto propname = _reader->TransitString(_offset);
    auto type = _reader->GetNodeTypeByString(propname);
    std::cout << "propname = " << propname << std::endl;
    // switch (type) {
    //     case WZNodeType::kProperty:
    //         break;
    //     default:
    //         break;
    // }
    return true;
}

}  // namespace wz