#ifndef LIBWZ_WZ_READER
#define LIBWZ_WZ_READER

#include <boost/thread/shared_mutex.hpp>
#include <cinttypes>
#include <string>

namespace wz {
class WZReader {
   public:
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
        boost::shared_lock<boost::shared_mutex> locked(_readlock);
        auto &value = *reinterpret_cast<T const *>(_offset);
        _offset += sizeof(T);
        _position += sizeof(T);
        return value;
    }

    auto ReadRawNullTerminatedString() -> std::string;
    auto ReadRawFixedSizeString(uint32_t size) -> std::string;
    auto ReadString() -> std::string;
    auto TransitString(uint64_t offset) -> std::string;

    auto SetPosition(uint64_t position) -> bool;

   public:
   private:
    boost::shared_mutex _readlock;
    uint64_t _position;
    // linux  file mapping descriptor
    int32_t _file;
    uint64_t _filesize;
    uint8_t *_base;
    uint8_t *_offset;
};

}  // namespace wz

#endif