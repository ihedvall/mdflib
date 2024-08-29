#pragma once
#include <string>

namespace mdf {

class ITimestamp {
 public:
  virtual ~ITimestamp() = default;
  [[nodiscard]] virtual uint64_t GetTime() const = 0;
  [[nodiscard]] virtual int16_t GetTimezone() const = 0;
  [[nodiscard]] virtual int16_t GetDst() const = 0;
};

class UtcTimeStamp : public ITimestamp {
 public:
  explicit UtcTimeStamp(uint64_t utc_timestamp);
  [[nodiscard]] uint64_t GetTime() const override;
  [[nodiscard]] int16_t GetTimezone() const override;
  [[nodiscard]] int16_t GetDst() const override;
  
 private:
  uint64_t utc_timestamp_;
};

class LocalTimeStamp : public ITimestamp {
 public:
  explicit LocalTimeStamp(uint64_t local_timestamp);
  [[nodiscard]] uint64_t GetTime() const override;
  [[nodiscard]] int16_t GetTimezone() const override;
  [[nodiscard]] int16_t GetDst() const override;

 private:
  uint64_t local_timestamp_;
};

class TimezoneTimeStamp : public ITimestamp {
 public:
  TimezoneTimeStamp(uint64_t utc_timestamp, int16_t timezone_offset_min,
                    int16_t dst_offset_min);
  [[nodiscard]] uint64_t GetTime() const override;
  [[nodiscard]] int16_t GetTimezone() const override;
  [[nodiscard]] int16_t GetDst() const override;

 private:
  uint64_t utc_timestamp_;
  int16_t timezone_offset_min_;
  int16_t dst_offset_min_;
};
}  // namespace mdf