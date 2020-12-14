#ifndef LIBWZ_NODE_TYPE_H
#define LIBWZ_NODE_TYPE_H

#include <boost/shared_ptr.hpp>
#include <boost/container/map.hpp>
#include <boost/tuple/tuple.hpp>

#include <string>

#include "wav.h"

namespace wz {

class WZNode;
enum class WZNodeType : uint8_t {
    kNone,
    kDirectoryWithOffset,
    kImageWithOffset,
    kDirectory,
    kImage,
    kProperty,
    kCanvas,
    kVector,
    kConvex,
    kSound,
    kUOL,
    kLua
};

enum class WZDataType : uint8_t {
    kNone = 0,
    kLua = 1,
    kShort = 2,
    kInteger = 3,
    kFloat = 4,
    kDouble = 5,
    kString = 8,
    kSub = 9,
    kUShort = 11,
    kUInteger = 19,
    kLong = 20,
};

typedef boost::tuple<int32_t, int32_t> Vector2i;
typedef boost::container::map<std::string, WZNode> WZNodes;
struct Vector{
    int32_t x;
    int32_t y;
};
struct WZData {
    public:
    WZData() {}
    std::string str = "";
    double dreal = 0.0;
    int64_t ireal = 0;
    boost::container::vector<uint8_t> buffer;
    union  {
        struct {
            uint32_t width = 0;
            uint32_t height = 0;
            uint32_t format1 = 0;
            uint32_t format2 = 0;
            uint32_t reserved = 0;
            uint32_t size = 0;
            uint64_t offset_bitmap = 0;
        } bitmap;
        Vector vector;
        struct {
            uint32_t size_mp3 = 0;
            uint32_t length_audio = 0;
            size_t offset_sound_header = 0;
            uint8_t size_wav_header = 0;
            size_t offset_wav_header = 0;
            size_t offset_mp3 = 0;
            wav::WavFormat wav_header;
            bool encrpyted_header = false;
        } audio;
    };
};

}  // namespace wz
#endif