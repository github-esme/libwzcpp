#ifndef LIBWZ_WZ_KEY_H
#define LIBWZ_WZ_KEY_H

#include <boost/container/vector.hpp>
#include <boost/thread/mutex.hpp>
#include <cinttypes>

namespace wz {

namespace container = boost::container;
class WZKey {
   public:
    static const uint32_t kBlockSize = 4096;
    static const uint32_t kSizeAESKey = 32;
    static const uint32_t kSizeAESIV = 4;
    explicit WZKey() = default;
    explicit WZKey(const container::vector<uint8_t>& aeskey,
                   const container::vector<uint8_t>& aesiv);
    WZKey(const WZKey& copy) {
        _key = copy._key;
        _aesiv = copy._aesiv;
        _aeskey = copy._aeskey;
    }
    WZKey(WZKey&& copy) {
        _key = copy._key;
        _aesiv = copy._aesiv;
        _aeskey = copy._aeskey;
    }
    ~WZKey() = default;
    container::vector<uint8_t>& GetKey() { return _key; }
    container::vector<uint8_t>& GetAesKey() { return _aeskey; }
    container::vector<uint8_t>& GetAesIV() { return _aesiv; }

    uint8_t operator[](uint32_t index);

   private:
    boost::mutex _lock;
    container::vector<uint8_t> _key;
    container::vector<uint8_t> _aeskey;
    container::vector<uint8_t> _aesiv;
    void ExtendKey(size_t size);
};

}  // namespace wz

#endif