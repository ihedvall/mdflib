/*
* Copyright 2025 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#pragma once

#include "mdf/mdcomment.h"
#include "mdf/mdstring.h"
#include "mdf/mdnumber.h"
#include "mdf/hounit.h"

namespace mdf {

enum class MdMonotony {
  MonDecrease,
  MonIncrease,
  StrictDecrease,
  StrictIncrease,
  Monotonous,
  StrictMon,
  NotMono,
};

class CnComment : public MdComment {
 public:
  CnComment();

  void LinkerName(MdString linker_name);
  [[nodiscard]] const MdString& LinkerName() const;

  void LinkerAddress(MdNumber linker_address);
  [[nodiscard]] const MdNumber& LinkerAddress() const;

  void AxisMonotony(MdMonotony axis_monotony);
  [[nodiscard]] const MdMonotony& AxisMonotony() const;

  void Raster(MdNumber raster);
  [[nodiscard]] const MdNumber& Raster() const;

  void Address(MdNumber address);
  [[nodiscard]] const MdNumber& Address() const;

  void Unit(HoUnit unit);
  [[nodiscard]] const HoUnit& Unit() const;

 private:
  MdString linker_name_;
  MdNumber linker_address_;
  MdMonotony axis_monotony_ = MdMonotony::NotMono;
  MdNumber raster_;
  MdNumber address_;
  HoUnit   unit_;


};

}  // namespace mdf


