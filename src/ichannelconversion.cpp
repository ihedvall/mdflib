/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#include <cmath>
#include "mdf/ichannelconversion.h"

namespace mdf {

void IChannelConversion::ChannelDataType(uint8_t channel_data_type) {
  channel_data_type_ = channel_data_type;
}

bool IChannelConversion::IsChannelInteger() const {
  return channel_data_type_ <= 3;
}

bool IChannelConversion::IsChannelFloat() const {
  return channel_data_type_ > 3 && channel_data_type_ <= 5;
}

bool IChannelConversion::ConvertLinear(double channel_value, double& eng_value) const {
  if (value_list_.empty()) {
    return false;
  }
  if (value_list_.size() == 1) {
    return value_list_[0]; // Constant value
  }
  eng_value = value_list_[0] + (value_list_[1]* channel_value);
  return true;
}

bool IChannelConversion::ConvertRational(double channel_value, double &eng_value) const {
  if (value_list_.size() < 6) {
    return false;
  }

  eng_value = (value_list_[0] * std::pow(channel_value,2))
      + (value_list_[1] * channel_value) + value_list_[2];
  const double div = (value_list_[3] * std::pow(channel_value,2))
      + (value_list_[4] * channel_value) + value_list_[5];
  if (div == 0.0) {
    return false;
  }
  eng_value /= div;
  return true;
}

bool IChannelConversion::ConvertPolynomial(double channel_value, double &eng_value) const {
  if (value_list_.size() < 6) {
    return false;
  }
  const double temp = channel_value - value_list_[4] - value_list_[5];
  eng_value = value_list_[1] - (value_list_[3] * temp);
  const double div = value_list_[2] * temp - value_list_[0];
  if (div == 0.0) {
    return false;
  }
  eng_value /= div;
  return true;
}

bool IChannelConversion::ConvertLogarithmic(double channel_value, double &eng_value) const {
  if (value_list_.size() < 7) {
    return false;
  }
  if (value_list_[3] == 0.0) {
    double eng_value = (channel_value - value_list_[6]) * value_list_[5] - value_list_[2];
    if (value_list_[0] == 0) {
      return false;
    }

    eng_value /= value_list_[0];
    eng_value = std::log(eng_value);
    if (value_list_[1] == 0.0) {
      return false;
    }
    eng_value /= value_list_[1];
  } else if (value_list_[0] == 0.0) {
    double eng_value = value_list_[2];
    const double temp2 = channel_value - value_list_[6];
    if (temp2 == 0) {
      return false;
    }
    eng_value /= temp2;
    eng_value -= value_list_[5];
    if (value_list_[3] == 0) {
      return false;
    }
    eng_value /= value_list_[3];
    eng_value = std::log(eng_value);
    if (value_list_[4] == 0.0) {
      return false;
    }
    eng_value /= value_list_[4];
  } else {
    return false;
  }
  return true;
}

bool IChannelConversion::ConvertExponential(double channel_value, double &eng_value) const {
  if (value_list_.size() < 7) {
    return false;
  }
  if (value_list_[3] == 0.0) {
    double eng_value = (channel_value - value_list_[6]) * value_list_[5] - value_list_[2];
    if (value_list_[0] == 0) {
      return false;
    }

    eng_value /= value_list_[0];
    eng_value = std::exp(eng_value);
    if (value_list_[1] == 0.0) {
      return false;
    }
    eng_value /= value_list_[1];
  } else if (value_list_[0] == 0.0) {
    double eng_value = value_list_[2];
    const double temp2 = channel_value - value_list_[6];
    if (temp2 == 0) {
      return false;
    }
    eng_value /= temp2;
    eng_value -= value_list_[5];
    if (value_list_[3] == 0) {
      return false;
    }
    eng_value /= value_list_[3];
    eng_value = std::exp(eng_value);
    if (value_list_[4] == 0.0) {
      return false;
    }
    eng_value /= value_list_[4];
  } else {
    return false;
  }
  return true;
}
bool IChannelConversion::ConvertAlgebraic(double channel_value, double &eng_value) const {
  // Todo (ihedvall): This requires a flex and bison formula calculator. Currently not supported.
  return false;
}

bool IChannelConversion::ConvertValueToValueInterpolate(double channel_value, double &eng_value) const {
  if (value_list_.size() < 2) {
    return false;
  }

  for (uint16_t n = 0; n < nof_values_; ++n) {
    const size_t key_index = n * 2;
    const size_t value_index = key_index + 1;
    if (value_index >= value_list_.size()) {
      break;
    }
    const double key = value_list_[key_index];
    const double value = value_list_[value_index];
    if (channel_value == key) {
      eng_value = value;
      return true;
    }
    if (channel_value < key) {
      if (n == 0) {
        eng_value = value;
        return true;
      }
      const double prev_key = value_list_[key_index - 2];
      const double prev_value = value_list_[value_index - 2];
      const double key_range = key - prev_key;
      const double value_range = value - prev_value;

      if (key_range == 0.0) {
        return false;
      }
      double x = (channel_value - prev_key) / key_range;
      eng_value = prev_value + (x * value_range);
      return true;
    }
  }
  eng_value = value_list_.back();
  return true;
}

bool IChannelConversion::ConvertValueToValue(double channel_value, double &eng_value) const {
  if (value_list_.size() < 2) {
    return false;
  }

  for (uint16_t n = 0; n < nof_values_; ++n) {
    const size_t key_index = n * 2;
    const size_t value_index = key_index + 1;
    if (value_index >= value_list_.size()) {
      break;
    }
    const double key = value_list_[key_index];
    const double value = value_list_[value_index];
    if (channel_value == key) {
      eng_value = value;
      return true;
    }

    if (channel_value < key) {
      if (n == 0) {
        eng_value = value;
        return true;
      }

      const double prev_key = value_list_[key_index - 2];
      const double prev_value = value_list_[value_index - 2];
      const double key_range = key - prev_key;
      if (key_range == 0.0) {
        return false;
      }
      double x = (channel_value - prev_key) / key_range;
      eng_value =  x <= 0.5 ? prev_value : value;
      return true;
    }
  }
  eng_value = value_list_.back();
  return true;
}

bool IChannelConversion::ConvertValueRangeToValue(double channel_value, double &eng_value) const {
  if (!value_list_.empty()) {
    return false;
  }

  for (uint16_t n = 0; n < nof_values_; ++n) {
    size_t key_min_index = n * 2;
    size_t key_max_index = key_min_index + 1;
    size_t value_index = key_min_index + 2;
    if (value_index >= value_list_.size()) {
      break;
    }
    const double key_min = value_list_[key_min_index];
    const double key_max = value_list_[key_max_index];
    const double value = value_list_[value_index];
    if (IsChannelInteger() && channel_value >= key_min && channel_value <= key_max) {
      eng_value = value;
      return true;
    }

    if (IsChannelFloat() && channel_value >= key_min && channel_value < key_max) {
      eng_value = value;
      return true;
    }
  }
  eng_value = value_list_.back();
  return true;
}

bool IChannelConversion::ConvertTextToValue(const std::string &channel_value, double &eng_value) const {
  return false;
}

bool IChannelConversion::ConvertValueToText(double channel_value, std::string &eng_value) const {
  return false;
}

bool IChannelConversion::ConvertValueRangeToText(double channel_value, std::string &eng_value) const {
  return false;
}

bool IChannelConversion::ConvertTextToTranslation(const std::string &channel_value, std::string &eng_value) const {
  return false;
}

void IChannelConversion::Parameter(size_t index, double parameter) {
  while (index + 1 < value_list_.size()) {
    value_list_.push_back(0.0);
  }
  value_list_[index] = parameter;
}


} // end namespace mdf

