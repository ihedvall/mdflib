#pragma once
#include "MdfFile.h"
#include "MdfHeader.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfWriter {
 private:
  mdf::MdfWriter* writer;

 public:
  MdfWriter(MdfWriterType type, const char* filename) {
    writer = MdfWriterInit(type, filename);
  }
  ~MdfWriter() { MdfWriterUnInit(writer); }
  MdfFile GetFile() { return MdfFile(MdfWriterGetFile(writer)); }
  MdfHeader GetHeader() { return MdfHeader(MdfWriterGetHeader(writer)); }
  bool GetCompressData() { return MdfWriterGetCompressData(writer); }
  void SetCompressData(bool compress) {
    MdfWriterSetCompressData(writer, compress);
  }
  MdfDataGroup CreateDataGroup() {
    return MdfDataGroup(MdfWriterCreateDataGroup(writer));
  }
  bool InitMeasurement() { return MdfWriterInitMeasurement(writer); }
  void SaveSample(MdfChannelGroup group, uint64_t time) {
    MdfWriterSaveSample(writer, group.GetGroup(), time);
  }
  void StartMeasurement(uint64_t start_time) {
    MdfWriterStartMeasurement(writer, start_time);
  }
  void StopMeasurement(uint64_t stop_time) {
    MdfWriterStopMeasurement(writer, stop_time);
  }
  bool FinalizeMeasurement() { return MdfWriterFinalizeMeasurement(writer); }
};
}  // namespace MdfLibrary