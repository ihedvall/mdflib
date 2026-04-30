/*
 * Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <memory>

namespace mdf {
class IDataWriter;
class IChannelGroup;
}

namespace mdf::detail {

[[nodiscard]] std::unique_ptr<IDataWriter> CreateGroupDataWriter(
    const IChannelGroup& group);

}  // namespace mdf::detail
