#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cinttypes>
#include <cstdlib>
#include <vector>

#include "wzreader.h"

using namespace testing;

TEST(WZ_TEST, WZREADER_VALID) {
    boost::container::vector<uint8_t> key = {
        0x13, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00,
        0x00, 0xb4, 0x00, 0x00, 0x00, 0x1b, 0x00, 0x00, 0x00, 0x0f, 0x00,
        0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x52, 0x00, 0x00, 0x00};
    boost::container::vector<uint8_t> iv = {0, 0, 0, 0};
    wz::WZKey wzkey(key, iv);
    wz::WZReader reader("../../wz/Base.wz", wzkey);
    reader.Valid();
    auto header = reader.GetHeader();
    ASSERT_EQ(header.signature, 19280);
    ASSERT_EQ(reader.GetVersion(), 224);
    ASSERT_EQ(reader.Valid(), true);
}

TEST(WZ_TEST, WZREADER_READSTRING) {
    boost::container::vector<uint8_t> key = {
        0x13, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00,
        0x00, 0xb4, 0x00, 0x00, 0x00, 0x1b, 0x00, 0x00, 0x00, 0x0f, 0x00,
        0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x52, 0x00, 0x00, 0x00};
    boost::container::vector<uint8_t> iv = {0, 0, 0, 0};
    wz::WZKey wzkey(key, iv);
    wz::WZReader reader("../../wz/Base.wz", wzkey);
    reader.Valid();
    reader.GetVersion();
    auto value1 = reader.ReadCompressedInt();
    auto value2 = reader.Read<int8_t>();
    auto s = reader.ReadDecryptString();
    ASSERT_EQ(s, "StandardPDD.img");
}

TEST(WZ_TEST, WZREADER_PARSE_ROOT) {
    boost::container::vector<uint8_t> key = {
        0x13, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00,
        0x00, 0xb4, 0x00, 0x00, 0x00, 0x1b, 0x00, 0x00, 0x00, 0x0f, 0x00,
        0x00, 0x00, 0x33, 0x00, 0x00, 0x00, 0x52, 0x00, 0x00, 0x00};
    boost::container::vector<uint8_t> iv = {0, 0, 0, 0};
    wz::WZKey wzkey(key, iv);
    wz::WZReader reader("../../wz/Effect.wz", wzkey);
    reader.Valid();
    reader.GetVersion();
    //  wzdirectory
    auto entry_count = reader.ReadCompressedInt();
    for (auto i = 0u; i < entry_count; i++) {
        auto type = reader.Read<uint8_t>();
        std::string identity = "";
        switch (type) {
            case 1:
            case 2:
                identity = reader.ReadDecryptStringAt(
                    reader.GetHeader().size + 1 + reader.Read<int32_t>());
                break;
            case 3:
            case 4:
                identity = reader.ReadDecryptString();
                break;
            default:
                FAIL();
        }
        auto size = reader.ReadCompressedInt();
        auto sum32 = reader.ReadCompressedInt();
        auto offset = reader.ReadNodeOffset();
        printf("identity:%s, type:%d, size: %d, sum32: %d, offset:%d\n",
               identity.c_str(), type, size, sum32, offset);
    }
}
