#include "utils.h"
#include <algorithm>

namespace wz {
namespace utils {
namespace string {
auto IsNumber(const std::string& s) -> bool {
    return !s.empty() && std::find_if(s.begin(), s.end(), [](unsigned char c) {
                             return !std::isdigit(c);
                         }) == s.end();
}
}  // namespace string

}  // namespace utils

}  // namespace wz
