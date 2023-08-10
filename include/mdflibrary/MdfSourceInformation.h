#pragma once
#include "MdfMetaData.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfSourceInformation {
 private:
  const mdf::ISourceInformation* sourceInformation;

 public:
  MdfSourceInformation(const mdf::ISourceInformation* sourceInformation) {
    this->sourceInformation = sourceInformation;
  }
  ~MdfSourceInformation() { this->sourceInformation = nullptr; }
  int64_t GetIndex() { return MdfSourceInformationGetIndex(sourceInformation); }
  const char* GetName() {
    return MdfSourceInformationGetName(sourceInformation);
  }
  void SetName(const char* name) {
    MdfSourceInformationSetName(sourceInformation, name);
  }
  const char* GetDescription() {
    return MdfSourceInformationGetDescription(sourceInformation);
  }
  void SetDescription(const char* desc) {
    MdfSourceInformationSetDescription(sourceInformation, desc);
  }
  const char* GetPath() {
    return MdfSourceInformationGetPath(sourceInformation);
  }
  void SetPath(const char* path) {
    MdfSourceInformationSetPath(sourceInformation, path);
  }
  SourceType GetType() {
    return MdfSourceInformationGetType(sourceInformation);
  }
  void SetType(SourceType type) {
    MdfSourceInformationSetType(sourceInformation, type);
  }
  BusType GetBus() { return MdfSourceInformationGetBus(sourceInformation); }
  void SetBus(BusType bus) {
    MdfSourceInformationSetBus(sourceInformation, bus);
  }
  uint8_t GetFlags() { return MdfSourceInformationGetFlags(sourceInformation); }
  void SetFlags(uint8_t flags) {
    MdfSourceInformationSetFlags(sourceInformation, flags);
  }
  const MdfMetaData GetMetaData() {
    return MdfMetaData(MdfSourceInformationGetMetaData(sourceInformation));
  }
  MdfMetaData CreateMetaData() {
    return MdfMetaData(MdfSourceInformationCreateMetaData(sourceInformation));
  }
};
}  // namespace MdfLibrary