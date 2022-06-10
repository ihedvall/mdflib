/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <cstdint>
#include <string>
#include "mdf/imetadata.h"

namespace mdf {

enum class SourceType : uint8_t {
  Other = 0,
  Ecu = 1,
  Bus = 2,
  IoDevice = 3,
  Tool = 4,
  User = 5
};

enum class BusType : uint8_t {
  None = 0,
  Other = 1,
  Can = 2,
  Lin = 3,
  Most = 4,
  FlexRay = 5,
  Kline = 6,
  Ethernet = 7,
  Usb = 8
};

namespace SiFlag {
 constexpr uint8_t Simulated = 0x01;
}

class ISourceInformation {
 public:
  [[nodiscard]] virtual int64_t Index() const = 0;

  virtual void Name(const std::string& name) = 0;
  [[nodiscard]] virtual const std::string& Name() const = 0;

  virtual void Path(const std::string& path) = 0;
  [[nodiscard]] virtual const std::string& Path() const = 0;

  virtual void Description(const std::string& desc) = 0;
  [[nodiscard]] virtual std::string Description() const = 0;

  virtual void Type(SourceType type) = 0;
  [[nodiscard]] virtual SourceType Type() const = 0;

  virtual void Bus(BusType type) = 0;
  [[nodiscard]] virtual BusType Bus() const = 0;

  virtual void Flags(uint8_t flags) = 0;
  [[nodiscard]] virtual uint8_t Flags() const = 0;

  [[nodiscard]] virtual IMetaData* MetaData() = 0;
  [[nodiscard]] virtual const IMetaData* MetaData() const = 0;
};

} // end namespace mdf