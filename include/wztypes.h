#ifndef LIBWZ_NODE_TYPE_H
#define LIBWZ_NODE_TYPE_H

#include <boost/container/map.hpp>
#include <boost/tuple/tuple.hpp>
#include <string>
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
typedef struct WZData {
    std::string str;
    double dreal;
    int64_t ireal;
    union {
        struct {
            uint32_t width;
            uint32_t height;
            uint32_t format1;
            uint32_t format2;
            uint32_t reserved;
            uint32_t size;
            uint64_t offset_bitmap;
        } bitmap;
        struct {
            int32_t x;
            int32_t y;
        } vector;
    };
};

}  // namespace wz
#endif