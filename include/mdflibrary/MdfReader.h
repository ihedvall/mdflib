#pragma once
#include "MdfFile.h"
#include "MdfHeader.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class MdfReader {
 private:
  mdf::MdfReader* reader;

 public:
  MdfReader(const char* filename) { reader = MdfReaderInit(filename); }
  ~MdfReader() { this->reader = nullptr; }
  int64_t GetIndex() { return MdfReaderGetIndex(reader); }
  bool IsOk() { return MdfReaderIsOk(reader); }
  const MdfFile GetFile() { return MdfFile(MdfReaderGetFile(reader)); }
  const MdfHeader GetHeader() { return MdfHeader(MdfReaderGetHeader(reader)); }
  const MdfDataGroup GetDataGroup(size_t index) {
    return MdfDataGroup(MdfReaderGetDataGroup(reader, index));
  }
  bool Open() { return MdfReaderOpen(reader); }
  void Close() { MdfReaderClose(reader); }
  bool ReadHeader() { return MdfReaderReadHeader(reader); }
  bool ReadMeasurementInfo() { return MdfReaderReadMeasurementInfo(reader); }
  bool ReadEverythingButData() {
    return MdfReaderReadEverythingButData(reader);
  }
  bool ReadData(MdfDataGroup group) {
    return MdfReaderReadData(reader, group.GetDataGroup());
  }
};
}  // namespace MdfLibrary