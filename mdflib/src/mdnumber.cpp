/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "mdf/mdnumber.h"

#include <utility>

namespace mdf {
MdNumber::MdNumber(double number, uint64_t history_index,
                   std::string language)
: MdStandardAttribute(history_index, std::move(language) ),
  number_(std::to_string(number)) {

}

MdNumber::operator double() const {
  return Number<double>();
}

bool MdNumber::IsActive() const {
  return !number_.empty();
}

void MdNumber::DataType(MdDataType type) {
  data_type_ = type;
}

MdDataType MdNumber::DataType() const {
  return data_type_;
}

void MdNumber::Triggered(bool triggered) {
  triggered_ = triggered;
}

bool MdNumber::Triggered() const { return triggered_; }

void MdNumber::ByteCount(uint64_t byte_count) {
  byte_count_ = byte_count;
}

uint64_t MdNumber::ByteCount() const {
  return byte_count_;
}

void MdNumber::BitMas(uint64_t bit_mask) {
  bit_mask_ = bit_mask;
}

uint64_t MdNumber::BitMask() const {
  return bit_mask_;
}

void MdNumber::ByteOrder(MdByteOrder byte_order) {
  byte_order_ = byte_order;
}

MdByteOrder MdNumber::ByteOrder() const {
  return byte_order_;
}

void MdNumber::Min(double min) {
  min_ = min;
}

double MdNumber::Min() const {
  return min_;
}

void MdNumber::Max(double max) {
  max_ = max;
}

double MdNumber::Max() const {
  return max_;
}

void MdNumber::Average(double average) {
  average_ = average;
}

double MdNumber::Average() const {
  return average_;
}

void MdNumber::Unit(std::string unit) {
  unit_ = std::move(unit);
}

const std::string& MdNumber::Unit() const {
  return unit_;
}

void MdNumber::UnitRef(std::string unit_ref) {
  unit_ref_ = std::move(unit_ref);
}

const std::string& MdNumber::UnitRef() const {
  return unit_ref_;
}

}  // namespace mdf