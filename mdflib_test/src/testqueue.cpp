/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/

#include <gtest/gtest.h>
#include <list>
#include <deque>
#include <queue>
#include "mdf/samplerecord.h"
#include "mdf/mdfhelper.h"

using namespace mdf;


namespace mdf::test {

TEST(TestQueueSpeed, TestList) {
  SampleRecord test_record;
  test_record.timestamp = MdfHelper::NowNs();
  test_record.record_id = 1;
  test_record.record_buffer.resize(1000, 0);

  std::cout << std::endl;
  {
    const uint64_t start_time = MdfHelper::NowNs();
    std::list<SampleRecord> list;
    for (size_t sample = 0; sample < 1'000'000; ++sample) {
      list.push_back(test_record);
    }
    while (!list.empty()) {
      list.pop_front();
    }
    const uint64_t run_time = MdfHelper::NowNs() - start_time;
    std::cout << "List Run Time (ms): "
              << static_cast<double>(run_time) / 1'000'000 << std::endl;
  }

  {
    const uint64_t start_time = MdfHelper::NowNs();
    std::deque<SampleRecord> list;
    for (size_t sample = 0; sample < 1'000'000; ++sample) {
      list.push_back(test_record);
    }
    while (!list.empty()) {
      list.pop_front();
    }
    const uint64_t run_time = MdfHelper::NowNs() - start_time;
    std::cout << "Deque Run Time (ms): "
              << static_cast<double>(run_time) / 1'000'000 << std::endl;
  }

  {
    const uint64_t start_time = MdfHelper::NowNs();
    std::queue<SampleRecord> list;
    for (size_t sample = 0; sample < 1'000'000; ++sample) {
      list.push(test_record);
    }
    while (!list.empty()) {
      list.pop();
    }
    const uint64_t run_time = MdfHelper::NowNs() - start_time;
    std::cout << "Queue Run Time (ms): "
              << static_cast<double>(run_time) / 1'000'000 << std::endl;
  }
  std::cout << std::endl;
}

} // end namespace mdf::test