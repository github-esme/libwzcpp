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
            auto type = static_cast<WZNodeType>(_reader->Read<int8_t>());
            switch (type) {
                case WZNodeType::kDirectoryWithOffset:
                case WZNodeType::kImageWithOffset:
                    identity = _reader->ReadDecryptStringAt(
                        _reader->GetHeader().size + 1 +
                        _reader->Read<int32_t>());
                    break;
                case WZNodeType::kImage:
                case WZNodeType::kDirectory:
                    identity = _reader->ReadDecryptString();
                    break;
                default:
                    return false;
            }
            auto size = _reader->ReadCompressedInt();
            auto sum32 = _reader->ReadCompressedInt();
            auto offset = _reader->ReadNodeOffset();
            nodes.emplace_back(
                WZNode(type, identity, size, sum32, offset, _reader));
        }
    }
    for (auto& node : nodes) {
        std::cout << node.GetIdentity()
                  << " type: " << static_cast<uint32_t>(node.GetNodeType())
                  << std::endl;
        auto iterator = _nodes.emplace(node.GetIdentity(), node);
        if ((static_cast<uint32_t>(node.GetNodeType()) % 2) > 0)
            iterator.first->second.ExpandDirectory();
        else
            iterator.first->second.ExpandNodes();
    }
    return true;
}

auto WZNode::ExpandNodes() -> bool {
    _reader->SetPosition(_offset);
    auto name = _reader->TransitString(_offset);
    auto type = _reader->GetNodeTypeByString(name);
    std::cout << "PropType: " << type << std::endl;
    switch (type) {
        case WZNodeType::kProperty:
            break;
        default:
            break;
    }
    return true;
}

}  // namespace wz