#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <boost/make_shared.hpp>
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

TEST(WZ_TEST, WZREADER_PARSE_ETC) {
    wz::WZKey wzkey(wz::WZKey::kDefaultAESKey, wz::WZKey::kZeroIV);
    auto reader = boost::shared_ptr<wz::WZReader>(
        new wz::WZReader("../../wz/Etc.wz", wzkey));
    reader->Valid();
    reader->GetVersion();
    auto root = boost::make_shared<wz::WZNode>(wz::WZNode(
        wz::WZNodeType::kDirectory, "/", reader->GetPosition(), reader));
    root->SetIdentity("Etc.wz");
    root->ExpandDirectory();
}

TEST(WZ_TEST, WZREADER_PARSE_SOUND2) {
    wz::WZKey wzkey(wz::WZKey::kDefaultAESKey, wz::WZKey::kZeroIV);
    auto reader = boost::shared_ptr<wz::WZReader>(
        new wz::WZReader("../../wz/Sound2.wz", wzkey));
    reader->Valid();
    reader->GetVersion();
    auto root = boost::make_shared<wz::WZNode>(wz::WZNode(
        wz::WZNodeType::kDirectory, "/", reader->GetPosition(), reader));
    root->SetIdentity("Sound2.wz");
    root->ExpandDirectory();
}

TEST(WZ_TEST, WZREADER_PARSE_CHARACTER) {
    wz::WZKey wzkey(wz::WZKey::kDefaultAESKey, wz::WZKey::kZeroIV);
    auto reader = boost::shared_ptr<wz::WZReader>(
        new wz::WZReader("../../wz/Character.wz", wzkey));
    reader->Valid();
    reader->GetVersion();
    auto root = boost::make_shared<wz::WZNode>(wz::WZNode(
        wz::WZNodeType::kDirectory, "/", reader->GetPosition(), reader));
    root->SetIdentity("Character.wz");
    root->ExpandDirectory();
}
