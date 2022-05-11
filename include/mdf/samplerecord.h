/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */
/** \file samplerecord.h
 * Structure that is used when saving samples to a file.
 */
#pragma once
#include <cstdint>
#include <vector>
namespace mdf {

/** \struct SampleRecord samplerecord.h "mdf/samplerecord.h"
 * \brief Simple record buffer structure.
 *
 * The sample record structure is used when writing samples to an MDF file. The
 * struct has a timestamp, record ID and the raw record bytes.
 */
struct SampleRecord {
  uint64_t timestamp = 0; ///< Nanosecond since midnight 1970-01-01 UTC.
  uint64_t record_id = 0; ///< Unique record ID within the data group.
  std::vector<uint8_t> record_buffer; ///< Raw sample array.
};

}
