/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "mdf/cncomment.h"

#include <utility>

namespace mdf {
CnComment::CnComment()
: MdComment("CN") {

}

void CnComment::LinkerName(MdString linker_name) {
  linker_name_ = std::move(linker_name);
}

const MdString& CnComment::LinkerName() const {
  return linker_name_;
}

void CnComment::LinkerAddress(MdNumber linker_address) {
  linker_address_ = std::move(linker_address);
}

const MdNumber& CnComment::LinkerAddress() const {
  return linker_address_;
}

void CnComment::AxisMonotony(MdMonotony axis_monotony) {
  axis_monotony_ = axis_monotony;
}

const MdMonotony& CnComment::AxisMonotony() const {
  return axis_monotony_;
}

void CnComment::Raster(MdNumber raster) {
  raster_ = std::move(raster);
}

const MdNumber& CnComment::Raster() const {
  return raster_;
}

void CnComment::Address(MdNumber address) {
  address_ = std::move(address);
}

const MdNumber& CnComment::Address() const {
  return address_;
}

void CnComment::Unit(HoUnit unit) {
  unit_ = std::move(unit);
}

const HoUnit& CnComment::Unit() const {
  return unit_;
}

}  // namespace mdf