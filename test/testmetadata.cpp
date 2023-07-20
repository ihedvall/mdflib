/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include <gtest/gtest.h>

#include <memory>

#include "md4block.h"
#include "hd4block.h"
#include "fh4block.h"

using namespace mdf::detail;

namespace mdf::test {

TEST(TestMetaData, HDComment) {
  auto md4 = std::make_unique<detail::Md4Block>();
  auto* meta_data = md4.get();
  meta_data->InitMd("HDcomment");
  std::cout << meta_data->XmlSnippet() << std::endl;
  meta_data->StringProperty("TX", "Comments");
  EXPECT_TRUE(meta_data->StringProperty("TX") == "Comments");
  EXPECT_TRUE(meta_data->StringProperty("tx") == "Comments");

  ETag olle;
  olle.Name("Olle");
  olle.Description("Olle description");
  olle.Type("float");
  olle.Value(1.23);
  olle.Unit("m");
  meta_data->CommonProperty(olle);
  std::cout << meta_data->XmlSnippet() << std::endl;

  ETag olle_list;
  olle_list.Name("olle_list");

  ETag olle1 = olle;
  olle1.Name("olle1");
  olle_list.AddTag(olle1);
  meta_data->CommonProperty(olle_list);
  std::cout << meta_data->XmlSnippet() << std::endl;

  auto olle2 = meta_data->CommonProperty("Olle");
  EXPECT_TRUE(olle2.Name() == olle.Name());
  EXPECT_TRUE(olle2.Description() == olle.Description());

  const auto olle2_list = meta_data->CommonProperties();
  EXPECT_EQ(olle2_list.size(), 2);
}

TEST(TestMetaData, Hd4Block) {
  Hd4Block hd4;
  hd4.Author("Ingemar Hedvall");
  hd4.Department("Home");

  EXPECT_STREQ(hd4.Author().c_str(), "Ingemar Hedvall");
  EXPECT_STREQ(hd4.Department().c_str(), "Home");

  const auto* md4 = hd4.MetaData();
  ASSERT_TRUE(md4 != nullptr);
  std::cout << md4->XmlSnippet() << std::endl;
}

TEST(TestMetaData, Fh4Block) {
  Fh4Block fh4;

  fh4.Time(1234);
  EXPECT_EQ(fh4.Time(), 1234);

  fh4.Description("Comments");
  fh4.ToolName("Tester");

  EXPECT_STREQ(fh4.Description().c_str(), "Comments");
  EXPECT_STREQ(fh4.ToolName().c_str(), "Tester");

  const auto* md4 = fh4.MetaData();
  ASSERT_TRUE(md4 != nullptr);
  std::cout << md4->XmlSnippet() << std::endl;
}
}  // namespace mdf::test
