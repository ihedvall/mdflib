/*
 * Copyright 2023 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#include <gtest/gtest.h>
#include "littlebuffer.h"
#include "bigbuffer.h"
namespace mdf::test {

TEST(TestConversion, LittleBuffer) {
  LittleBuffer<uint8_t> val8(12);
  EXPECT_EQ(val8.data()[0],12);
  EXPECT_EQ(val8.value(), 12);

  LittleBuffer<uint16_t> val16(12);
  EXPECT_EQ(val16.data()[0], 12);
  EXPECT_EQ(val16.data()[1],0);
  EXPECT_EQ(val16.value(), 12);

  std::vector<uint8_t> buf32 = {0,0,0,0,66,0,0,0};
  LittleBuffer<uint32_t> val32(buf32,4);
  EXPECT_EQ(val32.data()[0], 66);
  EXPECT_EQ(val32.data()[1],0);
  EXPECT_EQ(val32.value(), 66);
}

TEST(TestConversion, BigBuffer) {
  BigBuffer<uint8_t> val8(12);
  EXPECT_EQ(val8.data()[0],12);
  EXPECT_EQ(val8.value(), 12);

  BigBuffer<uint16_t> val16(12);
  EXPECT_EQ(val16.data()[0], 0);
  EXPECT_EQ(val16.data()[1],12);
  auto* val1 = val16.data();
  const auto* val2 = val16.data();
  EXPECT_EQ(*val1, *val2);
  EXPECT_EQ(val16.value(), 12);

  std::vector<uint8_t> buf32 = {0,0,0,0,0,0,0,66};
  BigBuffer<uint32_t> val32(buf32,4);
  EXPECT_EQ(val32.data()[2], 0);
  EXPECT_EQ(val32.data()[3],66);
  EXPECT_EQ(val32.value(), 66);


}
} // end namespace