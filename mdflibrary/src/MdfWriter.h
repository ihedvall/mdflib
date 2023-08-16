/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <mdf/mdffactory.h>
#include <mdf/mdfwriter.h>

#include "MdfDataGroup.h"
#include "MdfFile.h"
#include "MdfHeader.h"

using namespace System;

namespace MdfLibrary {
public
enum class MdfWriterType : int {
  Mdf3Basic = 0,  ///< Basic MDF version 3 writer.
  Mdf4Basic = 1   ///< Basic MDF version 4 writer.
};

public
ref class MdfWriter {
 public:
  MdfWriter(MdfWriterType writer_type);
  virtual ~MdfWriter();

  property MdfFile^ File { MdfFile^ get(); };
  property MdfHeader^ Header { MdfHeader^ get(); };

  property bool CompressData {
	void set(bool compress);
	bool get();
  }

  bool Init(String^ path_name);

  MdfDataGroup^ CreateDataGroup();

  bool InitMeasurement();
  void SaveSample(MdfChannelGroup^ group, uint64_t time);
  void StartMeasurement(uint64_t start_time);
  void StopMeasurement(uint64_t stop_time);
  bool FinalizeMeasurement();

 protected:
  !MdfWriter();
  internal :
    mdf::MdfWriter* writer_ = nullptr;
};
}  // namespace MdfLibrary
