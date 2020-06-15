#include "wzreader.h"

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

#include "wzerrors.h"

namespace wz {

WZReader::WZReader(const std::string &path)
    : _position(0), _file(0), _filesize(0), _base(nullptr), _offset(nullptr) {
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
    try {
        boost::shared_lock<boost::shared_mutex> locked(_readlock);
        SetPosition(0);
        auto signature = Read<uint32_t>();
        auto datasize = Read<uint64_t>();
        auto headersize = Read<uint32_t>();
        auto copyright =
            ReadRawFixedSizeString(headersize - sizeof(uint32_t) -
                                   sizeof(uint64_t) - sizeof(uint32_t));

        std::cout << copyright;
    } catch (...) {
        return false;
    }
    return true;
}

auto WZReader::SetPosition(uint64_t position) -> bool {
    if (position >= _filesize) return false;
    _offset = _base + position;
    _position = position;
    return true;
}

}  // namespace wz
