#ifndef LIBWZ_WZ_READER
#define LIBWZ_WZ_READER

#include <boost/thread/mutex.hpp>
#include <cinttypes>
#include <string>

namespace wz {
class WZReader {
   public:
    class WZHeader {
       public:
        WZHeader() : signature(0), datasize(0), headersize(0), copyright("") {}
        uint16_t signature;
        uint64_t datasize;
        uint32_t headersize;
        std::string copyright;
    };
    explicit WZReader();
    explicit WZReader(const std::string &path);
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

    auto GetHeader() -> WZHeader & {
        if (_header.signature != 0) return _header;
        LoadHeader();
        return _header;
    }

    auto ReadRawNullTerminatedString() -> std::string;
    auto ReadRawFixedSizeString(uint32_t size) -> std::string;
    auto ReadString() -> std::string;
    auto TransitString(uint64_t offset) -> std::string;

    auto SetPosition(uint64_t position) -> bool;
    auto GetPosition() -> uint64_t { return _position; }
    auto GetVersion() -> uint32_t { return _version; }

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

    auto LoadHeader() -> void;
    auto LoadVersion() -> void;
    auto CalculateVersionHash(std::string version) -> uint16_t;
};

}  // namespace wz

#endif