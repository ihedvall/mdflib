/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

/** \file isampleobserver.h
 * \brief Interface class to a sample observer. This class is used internally.
 */
#pragma once
#include <cstdint>
#include <vector>
namespace mdf {

/** \brief Interface to a sample observer that handle incoming samples events.
 */
class ISampleObserver {
 public:
  ISampleObserver() = default; ///< Default constructor
  virtual ~ISampleObserver() = default; ///< Default destructor
  /** \brief Observer function that receives the sample record and parse out
   * a channel value.
   * @param sample Sample number.
   * @param record_id Record ID (channel group identity).
   * @param record Sample record (excluding the record ID.
   */
  virtual void OnSample(uint64_t sample, uint64_t record_id,
                        const std::vector<uint8_t>& record) = 0;
};

}  // namespace mdf
