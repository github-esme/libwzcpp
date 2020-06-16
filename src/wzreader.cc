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

namespace wz {

WZReader::WZReader(const std::string &path)
    : _position(0),
      _file(0),
      _filesize(0),
      _base(nullptr),
      _offset(nullptr),
      _version(0) {
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
        return _header.headersize != 0 && _version != 0;
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

auto WZReader::LoadHeader() -> void {
    auto cur_position = GetPosition();
    SetPosition(0);
    _header.signature = Read<uint32_t>();
    _header.datasize = Read<uint64_t>();
    _header.headersize = Read<uint32_t>();
    _header.copyright =
        ReadRawFixedSizeString(_header.headersize - sizeof(uint32_t) -
                               sizeof(uint64_t) - sizeof(uint32_t));
    SetPosition(cur_position);
}

auto WZReader::LoadVersion() -> void {
    if (_header.headersize == 0) LoadHeader();
    SetPosition(_header.headersize);
    auto version_hash = Read<uint16_t>();
    for (auto version = 0; version < 1000; version++) {
        std::string str_version = std::to_string(version);
        if (version_hash == CalculateVersionHash(str_version)) {
            _version = version;
            break;
        }
    }
}

auto WZReader::CalculateVersionHash(std::string version) -> uint16_t {
    if (!utils::string::IsNumber(version)) return 0;
    uint16_t factor = 0u;
    for (auto i = 0u; i < version.length(); i++) {
        factor = ((factor * 0x20) + static_cast<uint16_t>(version[i])) + 1;
    }
    uint16_t n1 = (factor >> 0x18) & 0xff;
    uint16_t n2 = (factor >> 0x10) & 0xff;
    uint16_t n3 = (factor >> 0x8) & 0xff;
    uint16_t n4 = factor & 0xff;
    return ~(n1 ^ n2 ^ n3 ^ n4) & 0xff;
}

auto WZReader::FastTripleXor(uint8_t *buffer, uint8_t *key1, uint8_t *key2,
                             size_t size) -> void {
#ifdef __SSE__
    auto m1 = reinterpret_cast<__m128i *>(buffer);
    auto m2 = reinterpret_cast<const __m128i *>(key1);
    auto m3 = reinterpret_cast<__m128i *>(key2);
    for (int i = 0; i <= size >> 4; ++i) {
        _mm_storeu_si128(m1 + i, _mm_xor_si128(_mm_loadu_si128(m2 + i),
                                               _mm_loadu_si128(m3 + i)));
    }
#elif defined(__ARM_NEON__)
    auto m1 = reinterpret_cast<__m128i *>(ns);
    auto m2 = reinterpret_cast<const __m128i *>(original);
    auto m3 = reinterpret_cast<__m128i *>(WzKey::emsWzNormalKey);

#if defined(__arm64__) || defined(__aarch64__)  // NEON64

    for (int i = 0; i <= len >> 4; ++i) {
        vst1q_s64(
            (int64_t *)(m1 + i),
            veorq_s64(
                vreinterpretq_m128i_s64(vld1q_s64((const int64_t *)(m2 + i))),
                vreinterpretq_m128i_s64(vld1q_s64((const int64_t *)(m3 + i)))));
    }
#else                                           // NEON

    for (int i = 0; i <= len >> 4; ++i) {
        vst1q_s32(
            (int32_t *)(m1 + i),
            veorq_s32(
                vreinterpretq_m128i_s32(vld1q_s32((const int32_t *)(m2 + i))),
                vreinterpretq_m128i_s32(vld1q_s32((const int32_t *)(m3 + i)))));
    }
#endif
#else
    for (auto i = 0u; i < size; i++) buffer[i] = buffer[i] ^ key1[i] ^ key2[i];
#endif
}

}  // namespace wz
