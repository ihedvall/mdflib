#pragma once

#include "mdf/itimestamp.h"
#include "mdfblock.h"

namespace mdf::detail {
class IMdfTimeStamp : public MdfBlock {
 public:
  virtual void SetTime(uint64_t time) = 0;
  virtual void SetTime(ITimestamp &timestamp) = 0;
  [[nodiscard]] virtual uint64_t GetTime() const = 0;
  
  
};
}  // namespace mdf::detail