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
#include "mdf/idatagroup.h"
namespace mdf {

/** \brief Interface to a sample observer that handle incoming samples events.
 */
class ISampleObserver {
 public:
  ISampleObserver() = delete;
  virtual ~ISampleObserver() = default; ///< Default destructor

  /** \brief Attach the observer to an observer list (publisher).
   *
   * This function is normally called in the constructor of an observer,
   * so there is normally no need to call it.
   */
  virtual void AttachObserver();

  /** \brief Detach the observer from an observer list
   *
   * This function detach the observer from the observer list. This function
   * shall be called when OnSample() function is not needed anymore. This
   * function is needed to handle dangling pointers. For example it is
   * called after the ReadData() function in a reader.
   */
  virtual void DetachObserver();

  /** \brief Observer function that receives the sample record and parse out
   * a channel value.
   * @param sample Sample number.
   * @param record_id Record ID (channel group identity).
   * @param record Sample record (excluding the record ID.
   */
  virtual void OnSample(uint64_t sample, uint64_t record_id,
                        const std::vector<uint8_t>& record);

 protected:
  explicit ISampleObserver(const IDataGroup& data_group);

  const IDataGroup& data_group_;  ///< Reference to the data group (DG) block.
 private:
  bool attached_ = false;
};



}  // namespace mdf
