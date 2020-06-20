#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <boost/shared_ptr.hpp>
#include <cinttypes>
#include <cstdlib>
#include <vector>

#include "wzkey.h"
#include "wznode.h"
#include "wzreader.h"

using namespace testing;

TEST(WZ_TEST, WZREADER_VALID) {
    wz::WZKey wzkey(wz::WZKey::kDefaultAESKey, wz::WZKey::kZeroIV);
    wz::WZReader reader("../../wz/Base.wz", wzkey);
    reader.Valid();
    auto header = reader.GetHeader();
    ASSERT_EQ(header.signature, 19280);
    ASSERT_EQ(reader.GetVersion(), 224);
    ASSERT_EQ(reader.Valid(), true);
}

TEST(WZ_TEST, WZREADER_READSTRING) {
    wz::WZKey wzkey(wz::WZKey::kDefaultAESKey, wz::WZKey::kZeroIV);
    wz::WZReader reader("../../wz/Etc.wz", wzkey);
    reader.Valid();
    reader.GetVersion();
    reader.ReadCompressedInt();
    reader.Read<int8_t>();
    auto s = reader.ReadStringXoredWithFactor();
    ASSERT_EQ(s, "StandardPDD.img");
}

TEST(WZ_TEST, WZREADER_PARSE_ROOT) {
    wz::WZKey wzkey(wz::WZKey::kDefaultAESKey, wz::WZKey::kZeroIV);
    auto reader = boost::shared_ptr<wz::WZReader>(
        new wz::WZReader("../../wz/Etc.wz", wzkey));
    reader->Valid();
    reader->GetVersion();
    wz::WZNode root(wz::WZNodeType::kDirectory, "/", reader->GetPosition(),
                    reader);
    root.ExpandDirectory();
}
