#ifndef LIBWZ_NODE_TYPE_H
#define LIBWZ_NODE_TYPE_H

#include <boost/container/map.hpp>
#include <boost/tuple/tuple.hpp>
#include <string>
namespace wz {

class WZNode;
enum class WZNodeType : uint32_t {
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
};
typedef boost::tuple<int32_t, int32_t> Vector2i;
typedef boost::container::map<std::string, WZNode> WZNodes;


} // namespacae wz
#endif