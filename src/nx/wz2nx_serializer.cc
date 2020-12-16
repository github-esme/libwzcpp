#include "nx/wz2nx_serializer.h"

#include <lz4.h>
#include <lz4hc.h>
#include <nx/color.h>
#include <squish.h>
#include <zlib.h>

#include <boost/asio/streambuf.hpp>
#include <boost/range/algorithm.hpp>
#include <cinttypes>
#include <fstream>
#include <iomanip>
#include <iostream>

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
    uint64_t zero_value = 0uL;
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
    node_levels.push_back(root.get());
    std::ofstream bw(path_to_nx, std::ios::binary);

    std::cout << "Write Headers...";
    bw.write("PKG4", 4);
    bw.write((char*)&zero_value, sizeof(uint32_t));  // node count
    bw.write((char*)&zero_value, sizeof(uint64_t));  // node block offset
    bw.write((char*)&zero_value, sizeof(uint32_t));  // string count
    bw.write((char*)&zero_value, sizeof(uint64_t));  // string offset table offset
    bw.write((char*)&zero_value, sizeof(uint32_t));  // bitmap count
    bw.write((char*)&zero_value, sizeof(uint64_t));  // bitmap offset table offset
    bw.write((char*)&zero_value, sizeof(uint32_t));  // audio count
    bw.write((char*)&zero_value, sizeof(uint64_t));  // audio offset table offset

    std::cout << "Write Nodes..." << std::endl;
    auto x = bw.tellp();
    EnsureMultiple(4, bw);
    uint64_t nodes_offset = bw.tellp();
    while (!node_levels.empty())
        WriteNodeLevel(node_levels, bw);

    boost::container::vector<uint64_t> offsets;

    std::cout << "Write Strings..." << std::endl;
    uint32_t strings_count = _strings.size();
    offsets.resize(strings_count, 0);
    for (auto idx = 0u; idx < strings_count; idx++) {
        EnsureMultiple(2, bw);
        offsets[idx] = bw.tellp();
        WriteString(_strings[idx], bw);
    }
    EnsureMultiple(8, bw);
    uint64_t strings_offset = bw.tellp();
    for (auto idx = 0u; idx < strings_count; idx++) {
        bw.write((char*)&offsets[idx], sizeof(uint32_t));
    }

    std::cout << "Write Bitmaps..." << std::endl;
    uint32_t bitmaps_count = _bitmaps_nodes.size();
    uint64_t bitmaps_offset = 0;
    offsets.resize(bitmaps_count, 0);
    for (auto idx = 0u; idx < bitmaps_count; idx++) {
        EnsureMultiple(8, bw);
        auto node = _bitmaps_nodes[idx];
        offsets[idx] = bw.tellp();
        WriteBitmap(node, bw);
    }
    EnsureMultiple(8, bw);
    bitmaps_offset = bw.tellp();
    for (auto idx = 0u; idx < bitmaps_count; idx++) {
        bw.write((char*)&offsets[idx], sizeof(uint32_t));
    }

    std::cout << "Write Sound..." << std::endl;
    uint32_t sounds_count = _sounds_nodes.size();
    uint64_t sounds_offset = 0;
    offsets.resize(sounds_count, 0);
    for (auto idx = 0u; idx < sounds_count; idx++) {
        EnsureMultiple(8, bw);
        auto node = _sounds_nodes[idx];
        offsets[idx] = bw.tellp();
        WriteMP3(node, bw);
    }
    EnsureMultiple(8, bw);
    sounds_offset = bw.tellp();
    for (auto idx = 0u; idx < sounds_count; idx++) {
        bw.write((char*)&offsets[idx], sizeof(uint32_t));
    }

    uint32_t nodes_count = _nodes.size();

    bw.seekp(4, std::ios::beg);
    bw.write((char*)&nodes_count, sizeof(uint32_t));
    bw.write((char*)&nodes_offset, sizeof(uint64_t));
    bw.write((char*)&strings_count, sizeof(uint32_t));
    bw.write((char*)&strings_offset, sizeof(uint64_t));
    bw.write((char*)&bitmaps_count, sizeof(uint32_t));
    bw.write((char*)&bitmaps_offset, sizeof(uint64_t));
    bw.write((char*)&sounds_count, sizeof(uint32_t));
    bw.write((char*)&sounds_offset, sizeof(uint64_t));

    bw.close();

    std::cout << "Nodes : " << _nodes.size() << std::endl;
    std::cout << "Nodes Offset : " << nodes_offset << std::endl;
    std::cout << "Strings : " << _strings.size() << std::endl;
}

void WZ2NXSerializer::EnsureMultiple(int32_t multiple, std::ostream& bw) {
    int32_t skip = (int32_t)(multiple - (bw.tellp() % multiple));
    if (skip == multiple) return;
    for (auto i = 0; i < skip; i++) bw.write("\x00", 1);
}

void WZ2NXSerializer::WriteNodeLevel(boost::container::vector<wz::WZNode*>& node_levels, std::ostream& bw) {
    uint32_t next_child_id = _nodes.size() + node_levels.size();
    for (auto& level_node : node_levels) {
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
    uint32_t zero_value = 0u;
    uint32_t name_id = AddString(node->GetIdentity());
    bw.write((char*)&name_id, sizeof(uint32_t));
    _uol_nodes.emplace(std::pair<uint32_t, wz::WZNode*>(bw.tellp(), node));
    bw.write((char*)&zero_value, sizeof(uint32_t));
    bw.write((char*)&zero_value, sizeof(uint32_t));
    bw.write((char*)&zero_value, sizeof(uint16_t));
    bw.write((char*)&zero_value, sizeof(uint16_t));
    bw.write((char*)&zero_value, sizeof(uint64_t));
}

void WZ2NXSerializer::WriteNode(wz::WZNode* node, std::ostream& bw, uint32_t next_child_id) {
    _nodes.push_back(node);
    uint16_t node_type;
    uint32_t zero_value = 0u;
    uint32_t node_size = (uint16_t)node->GetNodes().size();
    uint32_t name_id = AddString(node->GetIdentity());

    bw.write((char*)&name_id, sizeof(uint32_t));
    bw.write((char*)&next_child_id, sizeof(uint32_t));
    bw.write((char*)&node_size, sizeof(uint16_t));

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
    bw.write((char*)&node_type, sizeof(uint16_t));
    switch (node->GetNodeType()) {
        case WZNodeType::kProperty:
            switch (node->GetDataType()) {
                case WZDataType::kUInteger:
                case WZDataType::kInteger:
                case WZDataType::kUShort:
                case WZDataType::kShort:
                case WZDataType::kLong: {
                    auto value = node->GetULong();
                    bw.write((char*)&value, sizeof(value));
                    break;
                }
                case WZDataType::kDouble:
                case WZDataType::kFloat: {
                    auto value = (double)node->GetDouble();
                    bw.write((char*)&value, sizeof(double));
                    break;
                }
                case WZDataType::kString: {
                    auto string_id = AddString(node->GetStringValue());
                    bw.write((char*)&string_id, sizeof(uint32_t));
                    bw.write((char*)&zero_value, sizeof(uint32_t));
                    break;
                }
                default:
                    bw.write((char*)&zero_value, sizeof(uint32_t));
                    bw.write((char*)&zero_value, sizeof(uint32_t));
                    break;
            }
            break;
        case WZNodeType::kVector: {
            auto vector = node->GetVector();
            bw.write((char*)&vector.x, sizeof(int32_t));
            bw.write((char*)&vector.y, sizeof(int32_t));
            break;
        }
        case WZNodeType::kCanvas: {
            auto bitmap_id = _bitmaps_nodes.size();
            bw.write((char*)&bitmap_id, sizeof(uint32_t));
            _bitmaps_nodes.push_back(node);
            bw.write((char*)&node->GetImageMeta().width, sizeof(uint32_t));
            bw.write((char*)&node->GetImageMeta().height, sizeof(uint32_t));
            break;
        }
        case WZNodeType::kSound: {
            auto audio_id = _bitmaps_nodes.size();
            bw.write((char*)&audio_id, sizeof(uint32_t));
            _sounds_nodes.push_back(node);
            bw.write((char*)&node->GetSoundMeta().size_mp3, sizeof(uint16_t));
            bw.write((char*)&zero_value, sizeof(uint16_t));
            break;
        }
        default:
            bw.write((char*)&zero_value, sizeof(uint32_t));
            bw.write((char*)&zero_value, sizeof(uint32_t));
            break;
    }
}

uint32_t WZ2NXSerializer::AddString(const std::string& value) {
    uint32_t id = _strings.size();
    _strings.push_back(value);
    return id;
}

void WZ2NXSerializer::WriteString(const std::string& value, std::ostream& bw) {
    bool has_ctrl = std::count_if(value.begin(), value.end(),
                                  [](unsigned char c) { return std::iscntrl(c); }  // correct
                                  ) > 0;
    uint32_t string_length = value.length();

    // if (has_ctrl)
    //     std::cerr << "Warning; control character in string. Perhaps toggle /wzn?" << std::endl;

    bw.write((char*)&string_length, sizeof(uint16_t));
    bw.write(value.c_str(), string_length);
}

void WZ2NXSerializer::WriteMP3(wz::WZNode* node, std::ostream& bw) {
    auto& buffer = node->GetSoundValue();
    bw.write((char*)buffer.data(), buffer.size());
}

void WZ2NXSerializer::WriteBitmap(wz::WZNode* node, std::ostream& bw) {
    auto& meta = node->GetImageMeta();
    auto buffer = node->GetImageValue();
    auto buffer_in = boost::container::vector<uint8_t>();
    auto buffer_out = boost::container::vector<uint8_t>();
    auto width = meta.width;
    auto height = meta.height;
    auto format1 = meta.format1;
    auto format2 = meta.format2;

    if (width < 0 || height < 0) {
        std::cerr << "Invalid image size: " << std::dec << width << ", " << height << std::endl;
        throw std::runtime_error{"Invalid image size"};
    }
    auto bitmap_size = width * height * 4;
    auto biggest = std::max(static_cast<uint32_t>(bitmap_size), (uint32_t)meta.size);
    buffer_in.resize(biggest, 0);
    buffer_out.resize(biggest, 0);
    auto decompressed = 0u;
    auto decompress = [&] {
        z_stream strm = {};
        strm.next_in = buffer_in.data();
        strm.avail_in = meta.size;
        inflateInit(&strm);
        strm.next_out = buffer_out.data();
        strm.avail_out = static_cast<unsigned>(buffer_out.size());
        auto err = inflate(&strm, Z_FINISH);
        if (err != Z_BUF_ERROR) {
            if (err != Z_DATA_ERROR) {
                std::cerr << "zlib error of " << std::dec << err << std::endl;
            }
            return false;
        }
        decompressed = static_cast<int>(strm.total_out);
        inflateEnd(&strm);
        return true;
    };
    std::copy(buffer.begin(), buffer.begin() + buffer.size(), buffer_in.begin());
    if (!decompress()) {
        std::cerr << "decompress bitmap failed" << std::endl;
    }
    buffer_in.swap(buffer_out);
    struct color4444 {
        uint8_t b : 4;
        uint8_t g : 4;
        uint8_t r : 4;
        uint8_t a : 4;
    };
    static_assert(sizeof(color4444) == 2, "Your bitpacking sucks");
    struct color8888 {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t a;
    };
    static_assert(sizeof(color8888) == 4, "Your bitpacking sucks");
    struct color565 {
        uint16_t b : 5;
        uint16_t g : 6;
        uint16_t r : 5;
    };
    static_assert(sizeof(color565) == 2, "Your bitpacking sucks");
    auto pixels4444 = reinterpret_cast<color4444*>(buffer_in.data());
    auto pixels565 = reinterpret_cast<color565*>(buffer_in.data());
    auto pixelsout = reinterpret_cast<color8888*>(buffer_out.data());
    //Sanity check the sizes
    auto check = decompressed;
    switch (format1) {
        case 1:
            check *= 2;
            break;
        case 2:
            break;
        case 513:
            check *= 2;
            break;
        case 1026:
            check *= 4;
            break;
        case 2050:
            check *= 4;
            break;
        default:
            std::cerr << "Unknown image format1 of" << std::dec << format1 << std::endl;
            throw std::runtime_error("Unknown image type!");
    }
    auto pixels = width * height;
    switch (format2) {
        case 0:
            break;
        case 4:
            pixels /= 256;
            break;
        default:
            std::cerr << "Unknown image format2 of" << std::dec << static_cast<unsigned>(format2) << std::endl;
            throw std::runtime_error("Unknown image type!");
    }
    if (check != pixels * 4u) {
        std::cerr << "Size mismatch: " << std::dec << width << "," << height << "," << decompressed << "," << format1 << "," << format2 << std::endl;
        throw std::runtime_error("halp!");
    }
    switch (format1) {
        case 1:
            for (auto i = 0u; i < pixels; ++i) {
                auto p = pixels4444[i];
                pixelsout[i] = {table4[p.b], table4[p.g], table4[p.r], table4[p.a]};
            }
            buffer_in.swap(buffer_out);
            break;
        case 2:
            // Do nothing
            break;
        case 513:
            for (auto i = 0u; i < pixels; ++i) {
                auto p = pixels565[i];
                pixelsout[i] = {table5[p.b], table6[p.g], table5[p.r], 255};
            }
            buffer_in.swap(buffer_out);
            break;
        case 1026:
            squish::DecompressImage(buffer_out.data(), width, height, buffer_in.data(), squish::kDxt3);
            buffer_in.swap(buffer_out);
            break;
        case 2050:
            squish::DecompressImage(buffer_out.data(), width, height, buffer_in.data(), squish::kDxt5);
            buffer_in.swap(buffer_out);
            break;
    }
    switch (format2) {
        case 0:
            // Do nothing
            break;
        case 4:
            std::cerr << "Format2 of 4" << std::endl;
            scale<16>(buffer_in, buffer_out, width, height);
            buffer_in.swap(buffer_out);
            break;
    }
    buffer_out.resize(static_cast<size_t>(LZ4_compressBound(bitmap_size)));
    uint32_t final_size;
    if (true) {
        final_size = static_cast<uint32_t>(
            LZ4_compressHC(reinterpret_cast<char const*>(buffer_in.data()),
                           reinterpret_cast<char*>(buffer_out.data()), bitmap_size));
    } else {
        final_size = static_cast<uint32_t>(
            LZ4_compress(reinterpret_cast<char const*>(buffer_in.data()),
                         reinterpret_cast<char*>(buffer_out.data()), bitmap_size));
    }
    // bitmap_offset += final_size + 4;
    bw.write(reinterpret_cast<char const*>(&final_size), sizeof(uint32_t));
    bw.write(reinterpret_cast<char const*>(buffer_out.data()), final_size);
}

}  // namespace nx
}  // namespace wz
