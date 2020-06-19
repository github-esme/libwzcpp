#include "wzreader.h"

#include <string.h>
#include <wmmintrin.h>

#include <cstdio>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#include "utils.h"
#include "wzerrors.h"
#include "wznode.h"

namespace wz {

WZReader::WZReader(const std::string &path, const WZKey &wzkey)
    : _position(0),
      _file(0),
      _filesize(0),
      _base(nullptr),
      _offset(nullptr),
      _version(0),
      _key(wzkey) {
#ifdef _WIN32
#else
    _file = open(path.c_str(), O_RDONLY);
    if (_file == -1)
        throw WZReadError()
            << err_no(-1) << err_str("Create mapping failed. path: " + path);

    struct stat finfo;
    if (fstat(_file, &finfo) == -1)
        throw WZReadError()
            << err_no(-1)
            << err_str("Retrieve file stats failed. path: " + path);

    _filesize = finfo.st_size;
    _base = _offset = static_cast<uint8_t *>(
        mmap(nullptr, _filesize, PROT_READ, MAP_SHARED, _file, 0));
#endif
}

WZReader::~WZReader() {
    munmap(static_cast<void *>(_base), _filesize);
    close(_file);
}

auto WZReader::ReadRawNullTerminatedString() -> std::string {
    char c;
    std::string str;
    while ((c = Read<char>() != 0)) str += c;
    return str;
}

auto WZReader::ReadRawFixedSizeString(uint32_t size) -> std::string {
    std::string str = "";
    for (auto i = 0u; i < size; i++) str += Read<char>();
    return str;
}

auto WZReader::Valid() -> bool {
    boost::unique_lock<boost::mutex> locked(_readlock);
    try {
        LoadHeader();
        LoadVersion();
        return _header.size != 0 && _version != 0;
    } catch (...) {
    }
    return false;
}

auto WZReader::SetPosition(uint64_t position) -> bool {
    if (position >= _filesize) return false;
    _offset = _base + position;
    _position = position;
    return true;
}

auto WZReader::ReadCompressedInt() -> int32_t {
    auto value = Read<int8_t>();
    if (value == SCHAR_MIN) {
        return Read<int32_t>();
    }
    return value;
}

auto WZReader::ReadCompressedLong() -> int64_t {
    auto value = Read<int8_t>();
    if (value == SCHAR_MIN) {
        return Read<int64_t>();
    }
    return value;
}

auto WZReader::ReadNodeOffset() -> int32_t {
    auto factor = _version_factor;
    uint32_t offset =
        static_cast<uint32_t>(GetPosition() - _header.size) ^ UINT_MAX;
    offset *= factor;
    offset -= 0x581c3f6d;
    offset = wz::utils::bits::rol<uint32_t>(offset, offset & 0x1f);
    auto value = Read<uint32_t>();
    offset ^= value;
    offset += _header.size * 2;
    return offset;
}

auto WZReader::TransitString(size_t offset) -> std::string {
    auto type = Read<uint8_t>();
    auto offset2 = 0;
    switch (type) {
        case 0x0:
        case 0x73:
            return ReadStringXoredWithFactor();
            break;
        case 1:
        case 0x1B:
            return ReadStringXored();
        default:
            printf("[Error] Unknown Transit String Type: %d\n", type);
            return "";
            break;
    }
}

auto WZReader::ReadStringXoredWithFactor() -> std::string {
    auto size = Read<int8_t>();
    if (size > 0) {
        // Unicode String
        size = size == SCHAR_MAX ? Read<uint32_t>() : size;
        if (size >= USHRT_MAX) return "";

    } else {
        size = size == SCHAR_MAX ? Read<uint32_t>() : -size;
        auto array = ReadArray<uint8_t>(size);
        return DecryptASCIIString(array.data(), size);
    }
}

auto WZReader::ReadStringXored() -> std::string {
    std::string str;
    auto size = ReadCompressedInt();
    if (size >= str.max_size()) return "";
    auto buffer = ReadArray<uint8_t>(size);
    _key[size - 1];
    Xor(buffer.data(), size);
    str.append(buffer.data(), buffer.data() + size);
    auto x = 1;
    return str;
}

auto WZReader::DecryptUnicodeString(uint8_t *orignal, size_t size)
    -> std::string {
    std::u16string str;
    if (size / 2 >= str.max_size()) return "";
    // For multithreaded
    thread_local boost::container::vector<uint8_t> buffer;
    buffer.reserve(size << 1);
    buffer.resize(size << 1);
    memcpy(buffer.data(), orignal, size << 1);
    DecryptString(buffer.data(), orignal, size << 1, true);
    str.append(buffer.data(), buffer.data() + size);
    return "";
}

auto WZReader::DecryptASCIIString(uint8_t *orignal, size_t size)
    -> std::string {
    std::string str;
    if (size / 2 >= str.max_size()) return "";
    // For multithreaded
    thread_local boost::container::vector<uint8_t> buffer;
    buffer.reserve(size);
    buffer.resize(size);
    DecryptString(buffer.data(), orignal, size, false);
    str.append(buffer.data(), buffer.data() + size);
    return str;
}

auto WZReader::LoadHeader() -> void {
    auto cur_position = GetPosition();
    SetPosition(0);
    _header.signature = Read<uint32_t>();
    _header.datasize = Read<uint64_t>();
    _header.size = Read<uint32_t>();
    _header.copyright = ReadRawFixedSizeString(
        _header.size - sizeof(uint32_t) - sizeof(uint64_t) - sizeof(uint32_t));
    if (cur_position > 0) SetPosition(cur_position);
}

auto WZReader::LoadVersion() -> void {
    SetPosition(_header.size);
    auto version_hash = Read<uint16_t>();
    for (auto version = 0; version < 1000; version++) {
        std::string str_version = std::to_string(version);
        if (version_hash == CalculateVersionHash(str_version)) {
            _version = version;
            _version_factor = CalculateVersionFactor(str_version);
            break;
        }
    }
}

auto WZReader::CalculateVersionFactor(std::string version) -> uint16_t {
    if (!utils::string::IsNumber(version)) return 0;
    uint16_t factor = 0u;
    for (auto i = 0u; i < version.length(); i++) {
        factor = ((factor * 0x20) + static_cast<uint16_t>(version[i])) + 1;
    }
    return factor;
}

auto WZReader::CalculateVersionHash(std::string version) -> uint16_t {
    if (!utils::string::IsNumber(version)) return 0;
    uint16_t factor = CalculateVersionFactor(version);
    uint16_t n1 = (factor >> 0x18) & 0xff;
    uint16_t n2 = (factor >> 0x10) & 0xff;
    uint16_t n3 = (factor >> 0x8) & 0xff;
    uint16_t n4 = factor & 0xff;
    return ~(n1 ^ n2 ^ n3 ^ n4) & 0xff;
}

auto WZReader::DecryptString(uint8_t *buffer, uint8_t *origin, size_t size,
                             bool wide) -> void {
    auto i = 0u;
#ifdef __SSE__
    __m128i amask =
        _mm_setr_epi8(0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2,
                      0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9);
    __m128i aplus = _mm_set1_epi8(0x10);

    __m128i wmask = _mm_setr_epi16(0xAAAA, 0xAAAB, 0xAAAC, 0xAAAD, 0xAAAE,
                                   0xAAAF, 0xAAB0, 0xAAB1);
    __m128i wplus = _mm_set1_epi16(0x0008);
    auto m1 = reinterpret_cast<__m128i *>(buffer);
    auto m2 = reinterpret_cast<const __m128i *>(origin);
    _key[16];
    auto m3 = reinterpret_cast<__m128i *>(_key.GetKey().data());
    auto m4 = wide ? wmask : amask;
    for (i = 0u; i<size>> 4; ++i) {
        _key[i * 16];
        m3 = reinterpret_cast<__m128i *>(_key.GetKey().data());
        _mm_storeu_si128(m1 + i, _mm_xor_si128(_mm_loadu_si128(m2 + i),
                                               _mm_loadu_si128(m3 + i)));
        _mm_storeu_si128(m1 + i, _mm_xor_si128(_mm_loadu_si128(m1 + i), m4));
        amask = _mm_add_epi8(amask, aplus);
        wmask = _mm_add_epi8(wmask, wplus);
    }
#endif
    i *= 16;
    if (wide) {
        uint16_t factor = 0xAAAA + (i / 2);
        auto b1 = reinterpret_cast<uint16_t *>(buffer);
        auto b2 = reinterpret_cast<uint16_t *>(origin);
        _key[i];
        auto b3 = reinterpret_cast<uint16_t *>(_key.GetKey().data());
        for (; i < size; i += 2) {
            _key[i];
            b3 = reinterpret_cast<uint16_t *>(_key.GetKey().data());
            b1[i] = b2[i] ^ b3[3] ^ factor;
        }
    } else {
        uint8_t factor = (0xAA + i);
        for (; i < size; i += 1) {
            buffer[i] = origin[i] ^ _key[i] ^ factor++;
        }
    }
}

auto WZReader::GetNodeTypeByString(const std::string &str) -> WZNodeType {
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
        return WZNodeType::kNone;
    }
};

auto WZReader::Xor(uint8_t *buffer, size_t size) -> void {
    auto i = 0u;
#ifdef __SSE__
    auto m1 = reinterpret_cast<__m128i *>(buffer);
    auto m2 = reinterpret_cast<__m128i *>(WZKey::kLuaKey.GetAesKey().data());
    for (i = 0u; i<size>> 4; ++i) {
        WZKey::kLuaKey[(i + 1) * 16];
        m2 = reinterpret_cast<__m128i *>(WZKey::kLuaKey.GetKey().data());
        _mm_storeu_si128(m1 + i, _mm_xor_si128(_mm_loadu_si128(m1 + i),
                                               _mm_loadu_si128(m2 + i)));
    }
#endif
    i *= 16;
    for (; i < size; i += 1) {
        buffer[i] = buffer[i] ^ WZKey::kLuaKey[i];
    }
}

}  // namespace wz
