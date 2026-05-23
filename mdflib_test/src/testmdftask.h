/*
* Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <gtest/gtest.h>

namespace mdf::test {

class TestMdfTask: public testing::Test {
public:
  static void SetUpTestSuite();
  static void TearDownTestSuite();
};

}  // namespace mdf::test


