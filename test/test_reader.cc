#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <boost/shared_ptr.hpp>
#include <cinttypes>
#include <cstdlib>
#include <vector>

#include "wznode.h"
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
    reader.ReadCompressedInt();
    reader.Read<int8_t>();
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
    auto reader = boost::shared_ptr<wz::WZReader>(
        new wz::WZReader("../../wz/Character.wz", wzkey));
    reader->Valid();
    reader->GetVersion();
    wz::WZNode root(wz::WZNodeType::kDirectory, "/", reader->GetPosition(),
                    reader);
    root.ExpandDirectory();
}
