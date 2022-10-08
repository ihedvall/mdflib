/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <mdf/iattachment.h>
#include <mdf/imetadata.h>

#include <cstdint>
#include <string>
#include <vector>

namespace mdf {

enum class EventType : uint8_t {
  RecordingPeriod = 0,       ///< Specifies a recording period (range).
  RecordingInterrupt = 1,    ///< The recording was interrupted.
  AcquisitionInterrupt = 2,  ///< The data acquisition was interrupted.
  StartRecording = 3,        ///< Start recording event.
  StopRecording = 4,         ///< Stop recording event.
  Trigger = 5,               ///< Generic event (no range).
  Marker = 6                 ///< Another generic event (maybe range).
};

enum class SyncType : uint8_t {
  SyncTime = 1,      ///< Sync value represent time (s).
  SyncAngle = 2,     ///< Sync value represent angle (rad).
  SyncDistance = 3,  ///< Sync value represent distance (m).
  SyncIndex = 4,     ///< Sync value represent sample index.
};

enum class RangeType : uint8_t {
  RangePoint = 0,  ///< Defines a point
  RangeStart = 1,  ///< First in a range.
  RangeEnd = 2     ///< Last in a range.
};

enum class EventCause : uint8_t {
  CauseOther = 0,   ///< Unknown source.
  CauseError = 1,   ///< An error generated this event.
  CauseTool = 2,    ///< The tool generated this event.
  CauseScript = 3,  ///< A script generated this event.
  CauseUser = 4,    ///< A user generated this event.
};

class IEvent {
 public:
  [[nodiscard]] virtual int64_t Index() const = 0;

  virtual void Name(const std::string& name) = 0;
  [[nodiscard]] virtual const std::string& Name() const = 0;

  virtual void GroupName(const std::string& group_name) = 0;
  [[nodiscard]] virtual const std::string& GroupName() const = 0;

  virtual void Type(EventType event_type) = 0;
  [[nodiscard]] virtual EventType Type() const = 0;
  [[nodiscard]] std::string TypeToString() const;

  virtual void Sync(SyncType sync_type) = 0;
  [[nodiscard]] virtual SyncType Sync() const = 0;

  virtual void Range(RangeType range_type) = 0;
  [[nodiscard]] virtual RangeType Range() const = 0;
  [[nodiscard]] std::string RangeToString() const;

  virtual void Cause(EventCause cause) = 0;
  [[nodiscard]] virtual EventCause Cause() const = 0;
  [[nodiscard]] std::string CauseToString() const;

  virtual void CreatorIndex(size_t index) = 0;
  [[nodiscard]] virtual size_t CreatorIndex() const = 0;

  virtual void SyncValue(int64_t value) = 0;
  [[nodiscard]] virtual int64_t SyncValue() const = 0;

  virtual void SyncFactor(double factor) = 0;
  [[nodiscard]] virtual double SyncFactor() const = 0;
  [[nodiscard]] std::string ValueToString() const;

  virtual void ParentEvent(const IEvent* parent) = 0;
  [[nodiscard]] virtual const IEvent* ParentEvent() const = 0;

  virtual void RangeEvent(const IEvent* range_event) = 0;
  [[nodiscard]] virtual const IEvent* RangeEvent() const = 0;

  virtual void AddScope(const void* scope) = 0;
  [[nodiscard]] virtual const std::vector<const void*>& Scopes() const = 0;

  virtual void AddAttachment(const IAttachment* attachment) = 0;
  [[nodiscard]] virtual const std::vector<const IAttachment*>& Attachments()
      const = 0;

  /** \brief Returns an interface against an MD4 block
   *
   * @return Pointer to a meta data block.
   */
  [[nodiscard]] virtual IMetaData* MetaData() = 0;

  /** \brief Returns an constant interface against a MD4 block
   *
   * @return Pointer to a meta data block.
   */
  [[nodiscard]] virtual const IMetaData* MetaData() const = 0;

  void Description(const std::string& description);

  [[nodiscard]] std::string Description() const;

  void PreTrig(double pre_trig);

  [[nodiscard]] double PreTrig() const;

  void PostTrig(double post_trig);

  [[nodiscard]] double PostTrig() const;
};

}  // namespace mdf
