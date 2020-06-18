#include "wzkey.h"

#include <openssl/crypto.h>
#include <openssl/evp.h>

#include "wzerrors.h"

namespace wz {

WZKey::WZKey(const container::vector<uint8_t>& aeskey,
             const container::vector<uint8_t>& aesiv)
    : _aeskey(aeskey), _aesiv(aesiv) {
    if (_aeskey.size() != kSizeAESKey)
        throw WZKeyError() << err_no(-1) << err_str("Wrong key size");
    if (_aesiv.size() != kSizeAESIV)
        throw WZKeyError() << err_no(-1) << err_str("Wrong iv size");
}

uint8_t WZKey::operator[](uint32_t index) {
    if (index < _key.size()) return _key[index];
    boost::unique_lock<boost::mutex> locked(_lock);
    if (index < _key.size()) return _key[index];
    ExtendKey(index + 1);
    return _key[index];
}

void WZKey::ExtendKey(size_t size) {
    auto iv_ptr = reinterpret_cast<uint32_t*>(_aesiv.data());
    auto key_size =
        static_cast<size_t>(ceil(1.0 * size / kBlockSize) * kBlockSize);
    auto cipher_size = 0;
    auto total_cipher_size = 0;
    auto start_index = _key.size();
    // This function only reserve capacity
    _key.reserve(key_size);
    // This line is important, it set vector to given size
    _key.resize(key_size);
    // If there is no iv (All bytes in iv was zero), just retun zero keys
    if (!*iv_ptr) return;
    EVP_CIPHER_CTX* ctx;
    if (!(ctx = EVP_CIPHER_CTX_new())) {
        EVP_CIPHER_CTX_free(ctx);
        return;
    }
    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, _aeskey.data(),
                                _aesiv.data())) {
        EVP_CIPHER_CTX_free(ctx);
        return;
    }
    EVP_CIPHER_CTX_set_padding(ctx, 0);
    for (auto i = start_index; i < key_size; i += 16) {
        if (i == 0) {
            // The first block encrypts from iv
            uint8_t block[16];
            for (auto j = 0u; j < 16; j++) block[j] = _aesiv[j % 4];
            EVP_EncryptUpdate(ctx, _key.data() + start_index, &cipher_size,
                              block, 16);
        } else {
            // The last blocks encrypts the previous block of keys.
            EVP_EncryptUpdate(ctx, _key.data() + i, &cipher_size,
                              _key.data() + i - 16, 16);
        }
        total_cipher_size += cipher_size;
    }
    if (1 != EVP_EncryptFinal_ex(ctx, _key.data() + total_cipher_size,
                                 &cipher_size)) {
        EVP_CIPHER_CTX_free(ctx);
        return;
    }
    EVP_CIPHER_CTX_free(ctx);
    return;
}

}  // namespace wz
