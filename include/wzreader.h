#ifndef LIBWZ_WZ_READER
#define LIBWZ_WZ_READER

#include <boost/container/vector.hpp>
#include <boost/thread/mutex.hpp>
#include <cinttypes>
#include <string>

#include "wzkey.h"
#include "wztypes.h"

namespace wz {

class WZNode;
class WZReader {
   public:
    class WZHeader {
       public:
        WZHeader() : signature(0), datasize(0), size(0), copyright("") {}
        uint16_t signature;
        uint64_t datasize;
        uint32_t size;
        std::string copyright;
    };
    explicit WZReader(const std::string &path, const WZKey &wzkey);
    ~WZReader();
    //! Deletes all copy constructors
    WZReader(const WZReader &) = delete;
    WZReader(WZReader &&) = delete;
    WZReader &operator=(const WZReader &) = delete;
    WZReader &operator=(WZReader &&) = delete;

    auto Valid() -> bool;

    template <typename T>
    inline auto Read() -> T {
        auto &value = *reinterpret_cast<T const *>(_offset);
        _offset += sizeof(T);
        _position += sizeof(T);
        return value;
    }
    template <typename T>
    inline auto ReadArray(size_t length) -> boost::container::vector<T> {
        boost::container::vector<T> data;
        data.reserve(length);
        data.resize(length);
        memcpy(data.data(), _offset, length * sizeof(T));
        _offset += length * sizeof(T);
        _position += length * sizeof(T);
        return data;
    }

    auto GetHeader() -> WZHeader & {
        if (_header.signature != 0) return _header;
        LoadHeader();
        return _header;
    }

    auto ReadRawNullTerminatedString() -> std::string;
    auto ReadRawFixedSizeString(uint32_t size) -> std::string;
    auto ReadStringXoredWithFactor() -> std::string;
    auto ReadStringXored() -> std::string;
    auto TransitString(size_t offset) -> std::string;
    auto ReadCompressedInt() -> int32_t;
    auto ReadCompressedLong() -> int64_t;
    auto ReadNodeOffset() -> int32_t;

    auto SetPosition(uint64_t position) -> bool;
    auto GetPosition() -> uint64_t { return _position; }
    auto GetVersion() -> uint32_t { return _version; }
    auto GetLock() -> boost::mutex & { return _readlock; }
    auto GetNodeTypeByString(const std::string &str) -> WZNodeType;

   private:
    // For lock while reading nodes
    boost::mutex _readlock;
    // Current position of file
    uint64_t _position;
    // Linux  file mapping related
    int32_t _file;
    // Size of wz file
    uint64_t _filesize;
    // Base cusor of memory mapped wz file
    uint8_t *_base;
    // Current cursor of memory mapped wz file
    uint8_t *_offset;
    // Header of wz file
    WZHeader _header;
    // Version number of wz file
    uint32_t _version;
    uint32_t _version_factor;
    // Key for wz decryption
    WZKey _key;
    auto LoadHeader() -> void;
    auto LoadVersion() -> void;
    auto CalculateVersionHash(std::string version) -> uint16_t;
    auto CalculateVersionFactor(std::string version) -> uint16_t;
    auto DecryptString(uint8_t *buffer, uint8_t *key1, size_t size, bool wide)
        -> void;
    auto Xor(uint8_t *buffer, size_t size) -> void;
    auto DecryptUnicodeString(uint8_t *orignal, size_t size) -> std::string;
    auto DecryptASCIIString(uint8_t *orignal, size_t size) -> std::string;
};  // namespace wz

}  // namespace wz

#endif