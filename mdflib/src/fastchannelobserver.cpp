//
// Created by 宋健明 on 2023/12/28.
//
#include "fastchannelobserver.h"
namespace mdf::detail {
template <>
bool FastChannelObserver<std::vector<uint8_t>>::GetSampleUnsigned(
    uint64_t sample, uint64_t &value, uint64_t array_index) const {
  return false;
}
template <>
bool FastChannelObserver<std::string>::GetSampleUnsigned(
    uint64_t sample, uint64_t &value, uint64_t array_index) const {
  if (sample != current_index) {
    return false;
  }
  value = std::stoul(value_);
  return valid_;
}
template <>
bool FastChannelObserver<std::vector<uint8_t>>::GetSampleSigned(
    uint64_t sample, int64_t &value, uint64_t array_index) const {
  return false;
}
template <>
bool FastChannelObserver<std::string>::GetSampleSigned(
    uint64_t sample, int64_t &value, uint64_t array_index) const {
  if(sample!=current_index){
    return false;
  }
  value = std::stoll(value_);
  return valid_;
}
template<>
bool FastChannelObserver<std::vector<uint8_t>>::GetSampleFloat(uint64_t sample, double &value, uint64_t array_index) const {
  return  false;
}
template<>
bool FastChannelObserver<std::string>::GetSampleFloat(uint64_t sample, double &value, uint64_t array_index) const {
  if(sample!=current_index){
    return false;
  }
  value = std::stod(value_);
  return valid_;
}
template<>
bool FastChannelObserver<std::vector<uint8_t>>::GetSampleText(uint64_t sample, std::string &value, uint64_t array_index) const {
  if(sample!=current_index){
    return false;
  }
  std::ostringstream s;
  for(const auto byte:value_){
    s<<std::setfill('0')<<std::setw(2)<<std::uppercase<<std::hex<<static_cast<uint16_t>(byte);
  }
  value = s.str();
  return valid_;
}
template<>
bool FastChannelObserver<std::vector<uint8_t >>::GetSampleByteArray(uint64_t sample, std::vector<uint8_t> &value) const {
  if(sample!=current_index){
    return false;
  }
  value = value_;
  return valid_;
}
}  // namespace mdf::detail