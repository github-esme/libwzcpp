#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cinttypes>
#include <cstdlib>
#include <vector>

#include "wzreader.h"

using namespace testing;

TEST(WZ_TEST, WZREADER_VALID) {
    wz::WZReader reader("../../wz/Base.wz");
    reader.Valid();
    auto header = reader.GetHeader();
    ASSERT_EQ(header.signature, 19280);
    ASSERT_EQ(reader.GetVersion(), 224);
}