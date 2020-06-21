#include "utils.h"

#include <iconv.h>
#include <memory.h>
#include <unicode/uniset.h>
#include <unicode/uobject.h>
#include <unicode/uscript.h>

#include <algorithm>
#include <boost/locale/encoding.hpp>
#include <codecvt>
#include <sstream>
#include <vector>

namespace wz {
namespace utils {
namespace string {
auto IsNumber(const std::string& s) -> bool {
    return !s.empty() && std::find_if(s.begin(), s.end(), [](unsigned char c) {
                             return !std::isdigit(c);
                         }) == s.end();
}
bool EndWith(std::string const& value, std::string const& ending) {
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}
auto ToUTF8(const std::u16string& utf16) -> std::string {
    std::string out = "";
    char* src = (char*)utf16.c_str();
    size_t srclen = utf16.length() * 2;
    out.reserve(srclen * 2);
    out.resize(srclen * 2);
    char* dst = (char*)out.data();
    memset(dst, 0, srclen);
    size_t dstlen = out.size();
    iconv_t conv = iconv_open("UTF8", "UTF-16LE");
    if ((size_t)conv == -1) return out;
    auto e = iconv(conv, &src, &srclen, &dst, &dstlen);
    iconv_close(conv);
    return out;
}

auto Split(const std::string& str, boost::container::vector<std::string>& ret,
           std::string sep, bool nullallowed) -> uint32_t {
    if (str.empty()) return 0;
    ret.clear();
    std::string tmp;
    std::string::size_type pos_begin = str.find_first_not_of(sep);
    std::string::size_type comma_pos = 0;
    while (pos_begin != std::string::npos) {
        comma_pos = str.find(sep, pos_begin);
        if (comma_pos != std::string::npos) {
            tmp = str.substr(pos_begin, comma_pos - pos_begin);
            pos_begin = comma_pos + sep.length();
        } else {
            tmp = str.substr(pos_begin);
            pos_begin = comma_pos;
        }
        if (!tmp.empty()) {
            if (tmp == "" && !nullallowed) {
                ret.push_back(tmp);
            } else {
                ret.push_back(tmp);
            }
            tmp.clear();
        }
    }
    return ret.size();
}

}  // namespace string
}  // namespace utils
}  // namespace wz
