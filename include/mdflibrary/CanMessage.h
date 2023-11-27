/*
 * Copyright 2023 Simplxs
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <stdexcept>
#include <vector>

#include "MdfExport.h"

using namespace MdfLibrary::ExportFunctions;

namespace MdfLibrary {
class CanMessage {
 private:
  mdf::CanMessage* can;

 public:
  CanMessage(mdf::CanMessage* can) : can(can) {
    if (can == nullptr) throw std::runtime_error("MdfAttachmentInit failed");
  }
  ~CanMessage() { can = nullptr; };

  uint32_t GetMessageId() const { return CanMessageGetMessageId(can); };
  void SetMessageId(uint32_t msgId) { CanMessageSetMessageId(can, msgId); };
  uint32_t GetCanId() const { return CanMessageGetCanId(can); };
  void SetExtendedId(bool extendedId) {
    CanMessageSetExtendedId(can, extendedId);
  };
  bool GetExtendedId() const { return CanMessageGetExtendedId(can); };
  uint8_t GetDlc() const { return CanMessageGetDlc(can); };
  void SetDlc(uint8_t dlc) { CanMessageSetDlc(can, dlc); };

  uint32_t GetDataLength() const { return CanMessageGetDataLength(can); };
  void SetDataLength(uint32_t dataLength) {
    CanMessageSetDataLength(can, dataLength);
  };
  std::vector<uint8_t> GetDataBytes() const {
    size_t count = CanMessageGetDataBytes(can, nullptr);
    if (count <= 0) return std::vector<uint8_t>();
    auto dataList = std::vector<uint8_t>(count);
    CanMessageGetDataBytes(can, dataList.data());
    return dataList;
  };
  void SetDataBytes(const std::vector<uint8_t>& dataList) {
    CanMessageSetDataBytes(can, dataList.data(), dataList.size());
  };
  void SetDataBytes(const uint8_t* dataList, const size_t size) {
    CanMessageSetDataBytes(can, dataList, size);
  };
  uint64_t GetDataIndex() const { return CanMessageGetDataIndex(can); };
  void SetDataIndex(const uint64_t index) {
    CanMessageSetDataIndex(can, index);
  };
  bool GetDir() const { return CanMessageGetDir(can); };
  void SetDir(const bool transmit) { CanMessageSetDir(can, transmit); };
  bool GetSrr() const { return CanMessageGetSrr(can); };
  void SetSrr(const bool srr) { CanMessageSetSrr(can, srr); };
  bool GetEdl() const { return CanMessageGetEdl(can); };
  void SetEdl(const bool edl) { CanMessageSetEdl(can, edl); };
  bool GetBrs() const { return CanMessageGetBrs(can); };
  void SetBrs(const bool brs) { CanMessageSetBrs(can, brs); };
  bool GetEsi() const { return CanMessageGetEsi(can); };
  void SetEsi(const bool esi) { CanMessageSetEsi(can, esi); };
  bool GetRtr() const { return CanMessageGetRtr(can); };
  void SetRtr(const bool rtr) { CanMessageSetRtr(can, rtr); };
  bool GetWakeUp() const { return CanMessageGetWakeUp(can); };
  void SetWakeUp(const bool wakeUp) { CanMessageSetWakeUp(can, wakeUp); };
  bool GetSingleWire() const { return CanMessageGetSingleWire(can); };
  void SetSingleWire(const bool singleWire) {
    CanMessageSetSingleWire(can, singleWire);
  };
  uint8_t GetBusChannel() const { return CanMessageGetBusChannel(can); };
  void SetBusChannel(const uint8_t channel) {
    CanMessageSetBusChannel(can, channel);
  };
  uint8_t GetBitPosition() const { return CanMessageGetBitPosition(can); };
  void SetBitPosition(const uint8_t position) {
    CanMessageSetBitPosition(can, position);
  };
  CanErrorType GetErrorType() const { return CanMessageGetErrorType(can); };
  void SetErrorType(const CanErrorType type) {
    CanMessageSetErrorType(can, type);
  };
  void Reset();
};
}  // namespace MdfLibrary