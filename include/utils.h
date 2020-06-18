#ifndef LIBWZ_UTILS_H
#define LIBWZ_UTILS_H

#include <string>
#include <climits>

namespace wz {
namespace utils {
namespace string {
auto IsNumber(const std::string& s) -> bool;
}  // namespace string
namespace bits {
template <typename INT> 
INT rol(INT x, INT n) {
    return (x << n) | ((x) >> ( (sizeof(INT)* CHAR_BIT) -n));
}
}
}  // namespace utils
}  // namespace wz

#endif