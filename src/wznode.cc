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
            auto current_offset = 0;
            switch (node_type) {
                case WZNodeType::kDirectoryWithOffset:
                case WZNodeType::kImageWithOffset: {
                    auto offset_jmp = _reader->Read<uint32_t>();
                    current_offset = _reader->GetPosition();
                    _reader->SetPosition(_reader->GetHeader().size +
                                         offset_jmp);
                    node_type =
                        static_cast<WZNodeType>(_reader->Read<int8_t>());
                    assert(node_type == WZNodeType::kDirectory ||
                           node_type == WZNodeType::kImage);
                    identity = _reader->ReadStringXoredWithFactor();
                }
                case WZNodeType::kDirectory:
                case WZNodeType::kImage:
                    identity = _reader->ReadStringXoredWithFactor();
                    current_offset = _reader->GetPosition();
                    break;
                default:
                    return false;
            }
            _reader->SetPosition(current_offset);
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
    auto type = GetNodeTypeByString(propname);
    std::cout << "propname = " << propname << std::endl;
    switch (type) {
        case WZNodeType::kProperty:
            ///return ExpandProperty();
            break;
        case WZNodeType::kLua:
            break;
        default:
            break;
    }
    return true;
}

auto WZNode::ExpandProperty() -> bool {
    assert(_reader->Read<uint16_t>() == 0);
    auto count = _reader->ReadCompressedInt();
    for (int i = 0; i < count; i++) {
        auto identity = _reader->TransitString(_offset);
        auto type = _reader->Read<int8_t>();
        int x = 1;
    }
}

// auto WZNode::AddLuaProperty(std::string content, uint32_t offset) -> void {}

auto WZNode::GetNodeTypeByString(const std::string& str) -> WZNodeType {
    if (str == "Property") {
        return WZNodeType::kProperty;
    } else if (str == "Shape2D#Convex2D") {
        return WZNodeType::kConvex;
    } else if (str == "Shape2D#Vector2D") {
        return WZNodeType::kVector;
    } else if (str == "Sound_DX8") {
        return WZNodeType::kSound;
    } else if (str == "UOL") {
        return WZNodeType::kUOL;
    } else if (str == "Canvas") {
        return WZNodeType::kConvex;
    } else {
        return WZNodeType::kLua;
    }
};

}  // namespace wz