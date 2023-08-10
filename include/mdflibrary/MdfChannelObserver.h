#pragma once
#include "MdfChannel.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfChannelObserver {
 private:
  mdf::IChannelObserver* observer;

 public:
  MdfChannelObserver(mdf::IChannelObserver* observer) {
    this->observer = observer;
  }
  ~MdfChannelObserver() { this->observer = nullptr; }
  int64_t GetNofSamples() { return MdfChannelObserverGetNofSamples(observer); }
  const char* GetName() { return MdfChannelObserverGetName(observer); }
  const char* GetUnit() { return MdfChannelObserverGetUnit(observer); }
  const MdfChannel GetChannel() {
    return MdfChannel(MdfChannelObserverGetChannel(observer));
  }
  bool IsMaster() { return MdfChannelObserverIsMaster(observer); }
  bool GetChannelValueAsSigned(uint64_t sample, int64_t& value) {
    return MdfChannelObserverGetChannelValueAsSigned(observer, sample, value);
  }
  bool GetChannelValueAsUnSigned(uint64_t sample, uint64_t& value) {
    return MdfChannelObserverGetChannelValueAsUnSigned(observer, sample, value);
  }
  bool GetChannelValueAsFloat(uint64_t sample, double& value) {
    return MdfChannelObserverGetChannelValueAsFloat(observer, sample, value);
  }
  bool GetChannelValueAsString(uint64_t sample, char*& value) {
    return MdfChannelObserverGetChannelValueAsString(observer, sample, value);
  }
  bool GetChannelValueAsArray(uint64_t sample, uint8_t*& value, size_t& size) {
    return MdfChannelObserverGetChannelValueAsArray(observer, sample, value,
                                                    size);
  }
  bool GetEngValueAsSigned(uint64_t sample, int64_t& value) {
    return MdfChannelObserverGetEngValueAsSigned(observer, sample, value);
  }
  bool GetEngValueAsUnSigned(uint64_t sample, uint64_t& value) {
    return MdfChannelObserverGetEngValueAsUnSigned(observer, sample, value);
  }
  bool GetEngValueAsFloat(uint64_t sample, double& value) {
    return MdfChannelObserverGetEngValueAsFloat(observer, sample, value);
  }
  bool GetEngValueAsString(uint64_t sample, char*& value) {
    return MdfChannelObserverGetEngValueAsString(observer, sample, value);
  }
  bool GetEngValueAsArray(uint64_t sample, uint8_t*& value, size_t& size) {
    return MdfChannelObserverGetEngValueAsArray(observer, sample, value, size);
  }
};
}  // namespace MdfLibrary