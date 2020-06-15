#ifndef LIBWZ_WZ_ERRORS
#define LIBWZ_WZ_ERRORS

#include <exception>
#include <boost/exception/all.hpp>

namespace wz
{

    struct WZReadError : virtual std::exception,
                         virtual boost::exception
    {
    };
    typedef boost::error_info<struct tag_err_str, std::string> err_str;
    typedef boost::error_info<struct tag_err_no, int> err_no;

} // namespace wz

#endif