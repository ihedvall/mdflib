#pragma once

#include "mdf/imdftimestamp.h"
namespace mdf::detail {

class Mdf3Timestamp : public IMdfTimeStamp, public MdfBlock {
 public:
  void GetBlockProperty(detail::BlockPropertyList &dest) const override;
  size_t Read(std::FILE *file) override;
  size_t Write(std::FILE *file) override;

  void SetTime(uint64_t time) override;
  void SetTime(ITimestamp &timestamp) override;
  [[nodiscard]] uint64_t GetTimeNs() const override;
  [[nodiscard]] uint16_t GetTzOffsetMin() const override;
  [[nodiscard]] uint16_t GetDstOffsetMin() const override;

  std::string date_ = "01:01:1970";
  std::string time_ = "00:00:00";
  uint64_t local_timestamp_ =
      0;  ///< Nanosecond since 1 Jan 1970 with DST (local time)
  int16_t utc_offset_ = 0;     ///< UTC offset in hours
  uint16_t time_quality_ = 0;  ///< Default local PC time
  std::string timer_id_ = "Local PC Reference Time";
};

}  // namespace mdf::detail