/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "MdfFile.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfWriter {
 private:
  mdf::MdfWriter* writer;

 public:
  MdfWriter(MdfWriterType type, const char* filename)
      : writer(MdfWriterInit(type, filename)) {
    if (writer == nullptr) throw std::runtime_error("MdfWriterInit failed");
  }
  ~MdfWriter() {
    if (writer == nullptr) return;
    MdfWriterUnInit(writer);
    writer = nullptr;
  }
  MdfWriter(const MdfWriter&) = delete;
  MdfFile GetFile() const { return MdfFile(MdfWriterGetFile(writer)); }
  MdfHeader GetHeader() const { return MdfHeader(MdfWriterGetHeader(writer)); }
  bool GetCompressData() const { return MdfWriterGetCompressData(writer); }
  void SetCompressData(bool compress) {
    MdfWriterSetCompressData(writer, compress);
  }
  MdfDataGroup CreateDataGroup() {
    return MdfDataGroup(MdfWriterCreateDataGroup(writer));
  }
  bool InitMeasurement() { return MdfWriterInitMeasurement(writer); }
  void SaveSample(MdfChannelGroup group, uint64_t time) {
    MdfWriterSaveSample(writer, group.GetChannelGroup(), time);
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