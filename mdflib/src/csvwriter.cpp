/*
* Copyright 2024 Ingemar Hedvall
* SPDX-License-Identifier: MIT
 */

#include "mdf/csvwriter.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <limits>
#include <type_traits>
#include <mdf/mdflogstream.h>

using namespace std::filesystem;

namespace {
// A quote inside a quoted CSV field is written as two quotes
std::string QuotedField(const std::string& text) {
  std::string output(1, '\"');
  output.reserve(text.size() + 2);
  for (char input : text) {
    if (input == '\"') {
      output += '\"';
    }
    output += input;
  }
  output += '\"';
  return output;
}

// A float gets the shortest text that reads back as the same float. A double
// gets the 15 digits it holds, so computed values like 3 * 0.1 stay 0.3.
template <typename T>
std::string NumberText(T value) {
  // Whole numbers stay without exponent and decimals
  if (std::fabs(value) < 1e15 && value == std::trunc(value)) {
    return std::to_string(static_cast<int64_t>(value));
  }
  constexpr int max_digits = std::is_same_v<T, float>
                                 ? std::numeric_limits<float>::max_digits10
                                 : std::numeric_limits<double>::digits10;
  char text[40]{};
  for (int digits = std::numeric_limits<T>::digits10;; ++digits) {
    snprintf(text, sizeof(text), "%.*g", digits, static_cast<double>(value));
    if (digits >= max_digits ||
        static_cast<T>(std::strtod(text, nullptr)) == value) {
      break;
    }
  }
  return text;
}

// The text value is rounded to the display precision, so numbers are read as
// double. Integers without conversion and text conversions stay as text.
bool EngText(const mdf::IChannelObserver& obs, uint64_t sample,
             std::string& text) {
  using namespace mdf;
  const auto& channel = obs.Channel();
  const auto* conversion = channel.ChannelConversion();
  const auto type = channel.DataType();
  const bool is_float =
      type == ChannelDataType::FloatLe || type == ChannelDataType::FloatBe;
  bool number = is_float || (type <= ChannelDataType::SignedIntegerBe &&
                             conversion != nullptr);
  if (conversion != nullptr) {
    switch (conversion->Type()) {
      case ConversionType::ValueToText:
      case ConversionType::ValueRangeToText:
      case ConversionType::TextToTranslation:
      case ConversionType::BitfieldToText:
        number = false;
        break;
      default:
        break;
    }
  }
  if (!number) {
    return obs.GetEngValue(sample, text);
  }
  double value = 0.0;
  const bool valid = obs.GetEngValue(sample, value);
  // A 4-byte float value, also after a 1:1 conversion, gets float digits
  const auto single = static_cast<float>(value);
  text = is_float && channel.DataBytes() == 4 && single == value
             ? NumberText(single)
             : NumberText(value);
  return valid;
}
}  // namespace

namespace mdf {

CsvWriter::CsvWriter(const std::string& filename)
: dest_filename_(filename) {
  try {
    path fullname(filename);
    path parent_path = fullname.parent_path();
    parent_path_ = parent_path.string();
    name_ = fullname.filename().string();

    path temp_path = temp_directory_path();
    temp_path.append(name_);
    temp_filename_ = temp_path.string();

    if (!exists(parent_path_)) {
      create_directories(parent_path_);
    }

    is_ok_ = true;
  } catch (const std::exception& err) {
    MDF_ERROR() << "CSV file path error. Error: " << err.what();
  }
}

CsvWriter::~CsvWriter() {
  try {
    if (file_.is_open()) {
      file_.close();
    }
  } catch (const std::exception& err) {
    MDF_ERROR() << "Fail to close file: Error: " << err.what();
  }
}

bool CsvWriter::Convert(ChannelObserverList& observer_list) {
  std::vector<const IChannelObserver*> obs_list;

  // Add master channel(s) first in the list
  for (const auto& itr : observer_list) {
    const IChannelObserver* obs = itr.get();
    if (obs == nullptr) {
      continue;
    }
    if (obs->IsMaster()) {
      obs_list.emplace_back(obs);
      // Should be a break here as it should only exist one master
      // but some files has marked more channels as master as it
      // may exist in a real application.
    }
  }

  // Append non-master channels to the list
  for (const auto& itr : observer_list) {
    const IChannelObserver* obs = itr.get();
    if (obs == nullptr) {
      continue;
    }
    if (!obs->IsMaster()) {
      obs_list.emplace_back(obs);
    }
  }

  if (obs_list.empty()) {
    return false;
  }

  size_t nof_samples = observer_list[0]->NofSamples();
  if (nof_samples == 0) {
    return false;
  }

  bool saved = false;
  try {
    file_.open(temp_filename_, std::ios::out | std::ios::trunc);
    // HEADER LINE
    size_t count = 0;
    for (const auto* header : obs_list) {
      const IChannel& channel = header->Channel();
      const auto unit = channel.Unit();
      if (count > 0) {
        file_ << delimiter_;
      }
      std::string label = channel.Name();
      if (!unit.empty()) {
        label += " [" + unit + "]";
      }
      file_ << QuotedField(label);
      ++count;
    }
    file_ << std::endl;

    // ROW LINE
    for (size_t sample = 0; sample < nof_samples; ++sample) {
      count = 0;
      for (const IChannelObserver* obs : obs_list) {
        std::string value;
        bool valid = EngText(*obs, sample, value);
        if (count > 0) {
          file_ << delimiter_;
        }
        if (valid) {
          file_ << CsvText(value);
        }
        ++count;
      }
      file_ << std::endl;
    }
    file_.close();
    std::filesystem::rename(temp_filename_, dest_filename_);
    saved = true;
  } catch (const std::exception& err) {
    MDF_ERROR() << "File save error. Error: " << err.what();
  }

  try {
    if (file_.is_open()) {
      file_.close();
    }
  } catch (const std::exception& err) {
    MDF_ERROR() << "File close error. Error: " << err.what();
  }
  return saved;
}

std::string CsvWriter::CsvText(std::string& csv_text) const {
  for (char input : csv_text) {
    if (input == '\"' || input == delimiter_ || isspace(input)) {
      return QuotedField(csv_text);
    }
  }

  return csv_text;
}

} // namespace mdf