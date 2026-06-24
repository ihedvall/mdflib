/*
* Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <gtest/gtest.h>
#include "mdf/samplerecordobserver.h"

namespace mdf::test {

class TestSampleRecordObserver : public SampleRecordObserver {
 public:
  TestSampleRecordObserver() = delete;
  TestSampleRecordObserver(const IDataGroup& data_group,
                           const IChannelGroup& channel_group,
                           uint64_t base_time);
  void OnSampleRecord() override;
private:
  uint64_t nof_samples_ = 0;
};

class TestMdfTask: public testing::Test {
public:
  static void SetUpTestSuite();
  static void TearDownTestSuite();
};

}  // namespace mdf::test


