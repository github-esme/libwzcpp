#include "wznode.h"

#include <boost/make_shared.hpp>
#include <fstream>
#include <iostream>

#include "utils.h"
namespace wz {

auto WZNode::ExpandDirectory() -> bool {
    boost::unique_lock<boost::mutex> locked(_reader->GetLock());
    return ExpandDirectory(_offset);
}

auto WZNode::ExpandDirectory(uint32_t offset) -> bool {
    if (_node_type != WZNodeType::kDirectory) return false;
    boost::container::vector<WZNode> nodes;
    _reader->SetPosition(offset);
    auto nodes_count = _reader->ReadCompressed<int32_t>();
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
                _reader->SetPosition(_reader->GetHeader().size + offset_jmp);
                node_type = static_cast<WZNodeType>(_reader->Read<int8_t>());
                assert(node_type == WZNodeType::kDirectory ||
                       node_type == WZNodeType::kImage);
                _reader->ReadString(identity);
            }
            case WZNodeType::kDirectory:
            case WZNodeType::kImage:
                _reader->ReadString(identity);
                current_offset = _reader->GetPosition();
                break;
            default:
                return false;
        }
        _reader->SetPosition(current_offset);
        block_size = _reader->ReadCompressed<int32_t>();
        block_checksum = _reader->ReadCompressed<int32_t>();
        block_offset = _reader->ReadNodeOffset();
        auto& node =
            nodes.emplace_back(WZNode(node_type, identity, block_size,
                                      block_checksum, block_offset, _reader));
        node._parent = this;
    }
    for (auto& node : nodes) {
        auto iterator = _nodes.emplace(node.GetIdentity(), node);
        // std::cout << node.GetFullPath() << std::endl;
        _reader->SetPosition(node.GetOffset());
        if (node.GetNodeType() == WZNodeType::kDirectory)
            iterator.first->second.ExpandDirectory(node.GetOffset());
        else
            iterator.first->second.ExpandNodes(node.GetOffset());
    }
    return true;
}

auto WZNode::ExpandNodes() -> bool {
    boost::unique_lock<boost::mutex> locked(_reader->GetLock());
    return ExpandNodes(_offset);
}

auto WZNode::ExpandNodes(uint32_t offset_image) -> bool {
    std::string propname;
    if (utils::string::EndWith(propname, ".lua")) {
        _reader->TransitString(propname, offset_image, false);
    } else {
        _reader->TransitString(propname, offset_image);
    }
    auto type = GetNodeTypeByString(propname);
    switch (type) {
        case WZNodeType::kProperty:
            return ExpandProperty(offset_image);
        case WZNodeType::kCanvas:
            return ExpandCanvas(offset_image);
        case WZNodeType::kVector:
            return ExpandShape2dVector2d(offset_image);
        case WZNodeType::kConvex:
            return ExpandShape2dConvex2d(offset_image);
        case WZNodeType::kUOL:
            return ExpandUol(offset_image);
        case WZNodeType::kSound:
            return ExpandSound(offset_image);
        case WZNodeType::kLua:
            _data_node = true;
            _data.str = propname;
            _node_type = WZNodeType::kLua;
            break;
        default:
            break;
    }
    return true;
}

auto WZNode::ExpandProperty(uint32_t image_offset) -> bool {
    auto resrved = _reader->Read<int16_t>();
    // assert(resrved == 0);
    auto count = _reader->ReadCompressed<int32_t>();
    for (int i = 0; i < count; i++) {
        std::string identity = "";
        _reader->TransitString(identity, image_offset);
        auto data_type = _reader->Read<WZDataType>();
        auto& node =
            _nodes
                .emplace(identity, WZNode(WZNodeType::kProperty, identity,
                                          _reader->GetPosition(), _reader))
                .first->second;
        node._parent = this;
        node._data_type = data_type;
        node._reader = _reader;
        node._data_node = true;
        // std::cout << node.GetFullPath() << std::endl;
        switch (data_type) {
            case WZDataType::kNone:
                break;
            case WZDataType::kShort:
            case WZDataType::kUShort:
                node._data.ireal = _reader->Read<uint16_t>();
                break;
            case WZDataType::kInteger:
            case WZDataType::kUInteger:
                node._data.ireal = _reader->ReadCompressed<int32_t>();
                break;
            case WZDataType::kFloat:
                node._data.dreal = _reader->ReadCompressed<float>();
                break;
            case WZDataType::kDouble:
                node._data.dreal = _reader->Read<double>();
                break;
            case WZDataType::kString:
                _reader->TransitString(node._data.str, image_offset);
                break;
            case WZDataType::kLong:
                node._data.ireal = _reader->ReadCompressed<uint64_t>();
                break;
            case WZDataType::kSub: {
                node._data_node = false;
                auto size = _reader->Read<uint32_t>();
                uint64_t block_end = _reader->GetPosition() + size;
                node.ExpandNodes(image_offset);
                _reader->SetPosition(block_end);
                break;
            }
            default:
                printf("error data type");
                return false;
                break;
        }
    }
    return true;
}

auto WZNode::ExpandCanvas(uint32_t image_offset) -> bool {
    _node_type = WZNodeType::kCanvas;

    auto unknown = _reader->Read<uint8_t>();
    auto has_nodes = _reader->Read<uint8_t>();
    if (has_nodes)
        if (!ExpandProperty(image_offset)) return false;

    auto width = _reader->ReadCompressed<int32_t>();
    auto height = _reader->ReadCompressed<int32_t>();
    auto format1 = _reader->ReadCompressed<int32_t>();
    auto format2 = _reader->Read<uint8_t>();
    auto reserved = _reader->Read<uint32_t>();
    auto size = _reader->Read<uint32_t>();
    size_t offset_bitmap = _reader->GetPosition();
    _data_node = true;
    _data.bitmap.width = width;
    _data.bitmap.height = height;
    _data.bitmap.format1 = format1;
    _data.bitmap.format2 = format2;
    _data.bitmap.reserved = reserved;
    _data.bitmap.size = size;
    _data.bitmap.offset_bitmap = offset_bitmap;
    auto offset_current = _reader->GetPosition();
    _reader->SetPosition(offset_current + size);
    return true;
}

auto WZNode::ExpandShape2dVector2d(uint32_t image_offset) -> bool {
    auto x = _reader->ReadCompressed<int32_t>();
    auto y = _reader->ReadCompressed<int32_t>();
    _node_type = WZNodeType::kVector;
    _data_node = true;
    _data.vector.x = x;
    _data.vector.y = y;
    return true;
}

auto WZNode::ExpandShape2dConvex2d(uint32_t image_offset) -> bool {
    _node_type = WZNodeType::kConvex;
    auto size = _reader->ReadCompressed<int32_t>();
    for (auto i = 0; i < size; i++) {
        if (!ExpandNodes(image_offset)) return false;
    }
    return true;
}

auto WZNode::ExpandUol(uint32_t image_offset) -> bool {
    auto v = _reader->Read<int8_t>();
    std::string path;
    _reader->TransitString(path, image_offset);
    _data_node = true;
    _node_type = WZNodeType::kUOL;
    _data.str = path;
    if (_parent != nullptr) {
        boost::container::vector<std::string> parts;
        utils::string::Split(path, parts, "/", true);
        WZNode* n = _parent;
        for (auto& part : parts) {
            if (part == "..") {
                n = n->_parent;
            } else {
                break;
            }
        }
        _data.ireal = n->_offset;
    }
    return true;
}

auto WZNode::GetFullPath() -> std::string {
    std::string path = "";
    auto iterator = this;
    while (iterator) {
        path = "/" + iterator->_identity + path;
        iterator = iterator->_parent;
    }
    return path;
}

auto WZNode::ExpandSound(uint32_t image_offset) -> bool {
    _node_type = WZNodeType::kSound;
    // std::cout << this->GetFullPath() << std::endl;
    auto unknown = _reader->Read<uint8_t>();
    // assert(unknown == 0);
    // sound buffer size
    auto size_mp3 = _reader->ReadCompressed<int32_t>();
    // millesecond of audio
    auto length_audio = _reader->ReadCompressed<int32_t>();
    auto offset_header_start = _reader->GetPosition();
    const auto kSoundHeaderSize = 51;
    auto offset_sound_header = _reader->GetPosition();
    // skipped sound header buffer ( 51 bytes )
    _reader->SetPosition(_reader->GetPosition() + kSoundHeaderSize);
    auto size_wav_header = _reader->Read<uint8_t>();
    auto offset_wav_header = _reader->GetPosition();
    _data.audio.offset_sound_header = offset_sound_header;
    _data.audio.size_wav_header = size_wav_header;
    _data.audio.offset_wav_header = offset_wav_header;
    _data.audio.size_mp3 = size_mp3;
    _data.audio.length_audio = length_audio;
    _data.audio.offset_sound_header = offset_header_start;
    _data.audio.wav_header = _reader->Read<wav::WavFormat>();
    _data.audio.offset_mp3 = _reader->GetPosition();
    // to check wave fomrat header size is correct
    _data.audio.encrpyted_header =
        sizeof(wav::WavFormat) + _data.audio.wav_header.extra_size !=
        _data.audio.size_wav_header;

    if (_data.audio.encrpyted_header) {
        auto header_ptr = reinterpret_cast<char*>(&_data.audio.wav_header);
        for (auto i = 0; i < _data.audio.size_wav_header; i++) {
            header_ptr[i] ^= _reader->GetKey()[i];
        }
        auto recheck_header =
            sizeof(wav::WavFormat) + _data.audio.wav_header.extra_size !=
            _data.audio.size_wav_header;
        if (!recheck_header) {
            printf("decode wave header error.");
        }
    }
    _reader->SetPosition(_reader->GetPosition() + size_mp3);
    return true;
}

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
        return WZNodeType::kCanvas;
    } else {
        return WZNodeType::kLua;
    }
};

auto WZNode::GetInteger() -> int32_t {
    switch (_data_type) {
        case WZDataType::kFloat:
        case WZDataType::kDouble:
            return static_cast<int32_t>(_data.dreal);
            break;
        case WZDataType::kInteger:
        case WZDataType::kUInteger:
        case WZDataType::kShort:
        case WZDataType::kUShort:
        case WZDataType::kLong:
            return static_cast<int32_t>(_data.ireal);
        case WZDataType::kString:
            if (!_data.str.empty() &&
                std::all_of(_data.str.begin(), _data.str.end(), ::isdigit)) {
                return std::atoi(_data.str.c_str());
            }
            return 0;
        default:
            return 0;
            break;
    }
}

auto WZNode::GetUInteger() -> uint32_t {
    switch (_data_type) {
        case WZDataType::kFloat:
        case WZDataType::kDouble:
            return static_cast<uint32_t>(_data.dreal);
            break;
        case WZDataType::kInteger:
        case WZDataType::kUInteger:
        case WZDataType::kShort:
        case WZDataType::kUShort:
        case WZDataType::kLong:
            return static_cast<uint32_t>(_data.ireal);
        case WZDataType::kString:
            if (!_data.str.empty() &&
                std::all_of(_data.str.begin(), _data.str.end(), ::isdigit)) {
                return static_cast<uint32_t>(std::atoll(_data.str.c_str()));
            }
            return 0U;
        default:
            return 0U;
            break;
    }
}

auto WZNode::GetLong() -> int64_t {
    switch (_data_type) {
        case WZDataType::kFloat:
        case WZDataType::kDouble:
            return static_cast<int64_t>(_data.dreal);
            break;
        case WZDataType::kInteger:
        case WZDataType::kUInteger:
        case WZDataType::kShort:
        case WZDataType::kUShort:
        case WZDataType::kLong:
            return static_cast<int64_t>(_data.ireal);
        case WZDataType::kString:
            if (!_data.str.empty() &&
                std::all_of(_data.str.begin(), _data.str.end(), ::isdigit)) {
                return std::atoll(_data.str.c_str());
            }
            return 0L;
        default:
            return 0L;
            break;
    }
}

auto WZNode::GetULong() -> uint64_t {
    switch (_data_type) {
        case WZDataType::kFloat:
        case WZDataType::kDouble:
            return static_cast<uint64_t>(_data.dreal);
            break;
        case WZDataType::kInteger:
        case WZDataType::kUInteger:
        case WZDataType::kShort:
        case WZDataType::kUShort:
        case WZDataType::kLong:
            return static_cast<uint64_t>(_data.ireal);
        case WZDataType::kString:
            if (!_data.str.empty() &&
                std::all_of(_data.str.begin(), _data.str.end(), ::isdigit)) {
                return std::atoll(_data.str.c_str());
            }
            return 0UL;
        default:
            return 0UL;
            break;
    }
}

auto WZNode::GetStringValue() -> const std::string& {
    switch (_data_type) {
        case WZDataType::kFloat:
        case WZDataType::kDouble:
            if (_data.str.empty()) {
                _data.str = std::to_string(_data.dreal);
            }
            break;
        case WZDataType::kInteger:
        case WZDataType::kUInteger:
        case WZDataType::kShort:
        case WZDataType::kUShort:
        case WZDataType::kLong:
            if (_data.str.empty()) {
                _data.str = std::to_string(_data.ireal);
            }
            break;
        default:
            break;
    }
    return _data.str;
}

auto WZNode::GetSoundValue() -> const boost::container::vector<uint8_t>& {
    if (_node_type != WZNodeType::kSound || !_data.buffer.empty()) {
        return _data.buffer;
    }
    auto offset_current = _reader->GetPosition();
    _reader->SetPosition(_data.audio.offset_mp3);
    _reader->ReadArray(_data.buffer, _data.audio.size_mp3);
    _reader->SetPosition(offset_current);
    return _data.buffer;
}

}  // namespace wz