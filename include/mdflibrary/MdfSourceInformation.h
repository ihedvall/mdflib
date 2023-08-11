#pragma once
#include "MdfMetaData.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfSourceInformation {
 private:
  mdf::ISourceInformation* sourceInformation;

 public:
  MdfSourceInformation(mdf::ISourceInformation* sourceInformation)
      : sourceInformation(sourceInformation) {
    if (sourceInformation == nullptr)
      throw std::runtime_error("MdfSourceInformationInit failed");
  }
  MdfSourceInformation(const mdf::ISourceInformation* sourceInformation)
      : MdfSourceInformation(
            const_cast<mdf::ISourceInformation*>(sourceInformation)) {}
  ~MdfSourceInformation() { sourceInformation = nullptr; }
  int64_t GetIndex() const {
    return MdfSourceInformationGetIndex(sourceInformation);
  }
  std::string GetName() const {
    std::string str;
    size_t size = MdfSourceInformationGetName(sourceInformation, nullptr);
    str.reserve(size + 1);
    str.resize(size);
    MdfSourceInformationGetName(sourceInformation, str.data());
    return str;
  }
  void SetName(const char* name) {
    MdfSourceInformationSetName(sourceInformation, name);
  }
  std::string GetDescription() const {
    std::string str;
    size_t size =
        MdfSourceInformationGetDescription(sourceInformation, nullptr);
    str.reserve(size + 1);
    str.resize(size);
    MdfSourceInformationGetDescription(sourceInformation, str.data());
    return str;
  }
  void SetDescription(const char* desc) {
    MdfSourceInformationSetDescription(sourceInformation, desc);
  }
  std::string GetPath() const {
    std::string str;
    size_t size = MdfSourceInformationGetPath(sourceInformation, nullptr);
    str.reserve(size + 1);
    str.resize(size);
    MdfSourceInformationGetPath(sourceInformation, str.data());
    return str;
  }
  void SetPath(const char* path) {
    MdfSourceInformationSetPath(sourceInformation, path);
  }
  SourceType GetType() const {
    return MdfSourceInformationGetType(sourceInformation);
  }
  void SetType(SourceType type) {
    MdfSourceInformationSetType(sourceInformation, type);
  }
  BusType GetBus() const {
    return MdfSourceInformationGetBus(sourceInformation);
  }
  void SetBus(BusType bus) {
    MdfSourceInformationSetBus(sourceInformation, bus);
  }
  uint8_t GetFlags() const {
    return MdfSourceInformationGetFlags(sourceInformation);
  }
  void SetFlags(uint8_t flags) {
    MdfSourceInformationSetFlags(sourceInformation, flags);
  }
  const MdfMetaData GetMetaData() const {
    return MdfMetaData(MdfSourceInformationGetMetaData(sourceInformation));
  }
  MdfMetaData CreateMetaData() {
    return MdfMetaData(MdfSourceInformationCreateMetaData(sourceInformation));
  }
};
}  // namespace MdfLibrary