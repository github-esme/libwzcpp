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
}