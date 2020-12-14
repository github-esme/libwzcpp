#include "nx/wz2nx_serializer.h"

#include <boost/asio/streambuf.hpp>
#include <boost/range/algorithm.hpp>
#include <iostream>
#include <fstream>

namespace wz {
namespace nx {

void WZ2NXSerializer::Clear() {
    _nodes_buffer.consume(_nodes_buffer.size());
    _strings_buffer.consume(_strings_buffer.size());
    _bitmaps_buffer.consume(_bitmaps_buffer.size());
    _audios_buffer.consume(_audios_buffer.size());
}

void WZ2NXSerializer::Parse(const std::string& path_to_wz, const std::string& path_to_nx) {
    Clear();

    auto reader = boost::make_shared<wz::WZReader>(path_to_wz, wz::WZKey(wz::WZKey::kDefaultAESKey, wz::WZKey::kZeroIV));
    auto nodes = boost::container::vector<wz::WZNode>();

    if (!reader->Valid()) {
        std::cerr << "The wz file is invalid" << std::endl;
        return;
    }
    auto root = boost::make_shared<wz::WZNode>(wz::WZNode(
        wz::WZNodeType::kDirectory, "/", reader->GetPosition(), reader));

    if (!root->ExpandDirectory()) {
        std::cerr << "The wz file parse error" << std::endl;
        return;
    }
    boost::container::vector<wz::WZNode*> node_levels;
    for (auto& node : root->GetNodes()) {
        node_levels.push_back(&node.second);
    }
    std::ofstream bw(path_to_nx, std::ios::binary);
    
    std::cout << "Write Headers...";
    bw.write("PKG4", 4);
    bw.write("\x00\x00\x00\x00", 4);  // node count
    bw.write("\x00\x00\x00\x00\x00\x00\x00\x00", 8);  // node block offset
    bw.write("\x00\x00\x00\x00", 4);  // string count
    bw.write("\x00\x00\x00\x00\x00\x00\x00\x00", 8);  // string offset table offset
    bw.write("\x00\x00\x00\x00", 4);  // bitmap count
    bw.write("\x00\x00\x00\x00\x00\x00\x00\x00", 8);  // bitmap offset table offset
    bw.write("\x00\x00\x00\x00", 4);  // audio count
    bw.write("\x00\x00\x00\x00\x00\x00\x00\x00", 8);  // audio offset table offset

    std::cout << "Write Nodes...";
    EnsureMultiple(bw, 4);
    uint64_t offset_nodes = bw.tellp();
    while (!node_levels.empty())
        WriteNodeLevel(node_levels, bw);

    std::cout << "Nodes : " << _nodes.size() << std::endl;
    std::cout << "Nodes Offset : " << offset_nodes << std::endl;
    std::cout << "Strings : " << _strings.size() << std::endl;
    int x = 1;
}

void WZ2NXSerializer::EnsureMultiple(std::ostream& bw, int32_t multiple) {
    int32_t skip = (int32_t)(multiple - (bw.tellp() % multiple));
    for (auto i = 0; i < skip; i++) bw.write("\x00", 1);
}

void WZ2NXSerializer::WriteNodeLevel(boost::container::vector<wz::WZNode*>& node_levels, std::ostream& bw) {
    uint32_t next_child_id = _nodes.size();
    for (auto& level_node : node_levels) {
        std::cout << "Write " << level_node->GetIdentity() << std::endl;
        if (level_node->GetNodeType() == WZNodeType::kUOL)
            WriteUOL(level_node, bw);
        else
            WriteNode(level_node, bw, next_child_id);
        next_child_id += level_node->GetNodes().size();
    }
    boost::container::vector<wz::WZNode*> new_node_levels;
    for (auto& node : node_levels) {
        if (node->GetNodes().empty()) continue;
        boost::container::vector<wz::WZNode*> childs;
        for (auto& child : node->GetNodes()) {
            childs.push_back(&child.second);
        }
        boost::sort(childs, [this](wz::WZNode* const& a, wz::WZNode* const& b) -> bool {
            return (a->GetIdentity().compare(b->GetIdentity())) >= 0;
        });
        new_node_levels.insert(new_node_levels.end(), childs.begin(), childs.end());
    }
    node_levels.clear();
    node_levels = new_node_levels;
}

void WZ2NXSerializer::WriteUOL(wz::WZNode* node, std::ostream& bw) {
    _nodes.push_back(node);
    auto name_id = AddString(node->GetIdentity());
    bw.write((char*)&name_id, sizeof(name_id));
    _uol_nodes.emplace(std::pair<uint32_t, wz::WZNode*>(bw.tellp(), node));
    bw.write("\x00\x00\x00\x00", 4);
    bw.write("\x00\x00\x00\x00", 4);
    bw.write("\x00\x00\x00\x00", 4);
    bw.write("\x00\x00\x00\x00", 4);
}

void WZ2NXSerializer::WriteNode(wz::WZNode* node, std::ostream& bw, uint32_t next_child_id) {
    uint16_t node_type;
    _nodes.push_back(node);
   auto name_id = AddString(node->GetIdentity());
    bw.write((char*)&name_id, sizeof(name_id));
    bw.write((char*)&next_child_id, sizeof(next_child_id));
    auto node_size = (uint16_t)node->GetNodes().size();
    bw.write((char*)&node_size, sizeof(node_size));
    switch (node->GetNodeType()) {
        case WZNodeType::kDirectory:
        case WZNodeType::kDirectoryWithOffset:
        case WZNodeType::kImage:
        case WZNodeType::kImageWithOffset:
        case WZNodeType::kNone:
        case WZNodeType::kConvex:
            node_type = 0;
            break;
        case WZNodeType::kProperty:
            switch (node->GetDataType()) {
                case WZDataType::kNone:
                case WZDataType::kSub:
                    node_type = 0;
                    break;
                case WZDataType::kUInteger:
                case WZDataType::kInteger:
                case WZDataType::kUShort:
                case WZDataType::kShort:
                case WZDataType::kLong:
                    node_type = 1;
                    break;
                case WZDataType::kDouble:
                case WZDataType::kFloat:
                    node_type = 2;
                    break;
                case WZDataType::kString:
                    node_type = 3;
                    break;
                default:
                    std::cerr << "Unhandled  data type " << (int)node->GetDataType();
                    break;
            }
            break;
        case WZNodeType::kVector:
            node_type = 4;
            break;
        case WZNodeType::kCanvas:
            node_type = 5;
            break;
        case WZNodeType::kSound:
            node_type = 6;
            break;
        default:
            std::cerr << "Unhandled node type " << (int)node->GetNodeType();
            break;
    }
    bw.write((char*)&node_type, sizeof(node_type));
    switch (node->GetNodeType()) {
        case WZNodeType::kProperty:
            switch (node->GetDataType()) {
                case WZDataType::kUInteger:
                case WZDataType::kInteger:
                case WZDataType::kUShort:
                case WZDataType::kShort:
                case WZDataType::kLong:
                    bw << (uint64_t)node->GetLong();
                    break;
                case WZDataType::kDouble:
                case WZDataType::kFloat:
                    bw << (double)node->GetDouble();
                    break;
                case WZDataType::kString:
                    bw << AddString(node->GetStringValue());
                    break;
                default:
                    bw.write("\x00\x00\x00\x00", 4);
                    break;
            }
            break;
        case WZNodeType::kVector:
            bw << (int32_t)node->GetVector().x;
            bw << (int32_t)node->GetVector().y;
            break;
        case WZNodeType::kCanvas:
            node_type = 5;
            bw.write("\x00\x00\x00\x00", 4);
            break;
        case WZNodeType::kSound:
            node_type = 6;
            bw.write("\x00\x00\x00\x00", 4);
            break;
        default:
            bw.write("\x00\x00\x00\x00", 4);
            break;
    }
}

uint32_t WZ2NXSerializer::AddString(const std::string& value) {
    uint32_t id = _strings.size();
    _strings.push_back(value);
    return id;
}

}  // namespace nx
}  // namespace wz
