/*
* Copyright 2026 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include "validatingtask.h"

#include <exception>
#include <filesystem>

#include "mdf/mdflogstream.h"

using namespace std::filesystem;

namespace mdf {

void ValidatingTask::Run() {
  try {
    CheckSourceFile();
    CreateSourceTempFile();
    const bool all_valid = ValidateAllValues();
    if (all_valid && IsMdf4File()) {
      AddValidationResult();
      CopyTempFile();
    }
    DeleteTempFile();
    Error(false);
    Result(all_valid);
  } catch (const std::exception& err) {
    std::ostringstream oss;
    oss << "Failed to run the validation task. Error: " << err.what();
    Error(true);
    ErrorMessage(oss.str());
  }
}

}  // namespace mdf