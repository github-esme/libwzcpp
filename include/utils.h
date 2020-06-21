#ifndef LIBWZ_UTILS_H
#define LIBWZ_UTILS_H

#include <climits>
#include <string>
#include <boost/container/vector.hpp>

namespace wz {
namespace utils {
namespace string {
auto IsNumber(const std::string &s) -> bool;
auto Split(const std::string& str, boost::container::vector<std::string>& ret, std::string sep, bool nullallowed) -> uint32_t;
bool EndWith(std::string const &value, std::string const &ending);
auto ToUTF8(const std::u16string &utf16) -> std::string;
}  // namespace string
namespace bits {
template <typename INT>
INT rol(INT x, INT n) {
    return (x << n) | ((x) >> ((sizeof(INT) * CHAR_BIT) - n));
}
}  // namespace bits
}  // namespace utils
}  // namespace wz

#endif