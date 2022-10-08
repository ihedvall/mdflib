/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <cstdint>

#include "mdf/ichannel.h"
#include "mdf/ichannelgroup.h"
#include "mdf/idatagroup.h"

namespace mdf {

struct ElementLink {
  const IDataGroup* data_group = nullptr;
  const IChannelGroup* channel_group = nullptr;
  const IChannel* channel = nullptr;
};

enum class ChType : uint8_t {
  Group = 0,
  Function = 1,
  Structure = 2,
  MapList = 3,
  InputVariable = 4,
  OutputVariable = 5,
  LocalVariable = 6,
  CalibrationDefinition = 7,
  CalibrationObject = 8
};

class IChannelHierarchy {
 public:
  [[nodiscard]] virtual int64_t Index() const = 0;

  virtual void Name(const std::string& name) = 0;
  [[nodiscard]] virtual const std::string& Name() const = 0;

  virtual void Type(ChType type) = 0;
  [[nodiscard]] virtual ChType Type() const = 0;
  [[nodiscard]] std::string TypeToString() const;

  virtual void Description(const std::string& description) = 0;
  [[nodiscard]] virtual std::string Description() const = 0;

  /** \brief Returns an interface against an MD4 block
   *
   * @return Pointer to a meta data block.
   */
  [[nodiscard]] virtual IMetaData* MetaData() = 0;

  /** \brief Returns an constant interface against a MD4 block
   *
   * @return Pointer to a meta data block.
   */
  [[nodiscard]] virtual const IMetaData* MetaData() const = 0;

  virtual void AddElementLink(const ElementLink& element) = 0;
  [[nodiscard]] virtual const std::vector<ElementLink>& ElementLinks()
      const = 0;

  [[nodiscard]] virtual IChannelHierarchy* CreateChannelHierarchy() = 0;
  [[nodiscard]] virtual std::vector<IChannelHierarchy*> ChannelHierarchies()
      const = 0;
};
}  // namespace mdf