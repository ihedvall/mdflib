/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <vector>
#include <cstdint>
namespace mdf {

class ISampleObserver {
 public:
  ISampleObserver() = default;
  virtual ~ISampleObserver() = default;
  virtual void OnSample(size_t sample, uint64_t record_id, const std::vector<uint8_t>& record) = 0;
};

}
