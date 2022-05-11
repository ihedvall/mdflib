/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <string>
#include "mdf/idatagroup.h"

namespace mdf {

class IHeader {
 public:
  [[nodiscard]] virtual int64_t Index() const = 0;

  virtual void Author(const std::string& author) = 0;
  [[nodiscard]] virtual std::string Author() const = 0;

  virtual void Department(const std::string& department) = 0;
  [[nodiscard]] virtual std::string Department() const = 0;

  virtual void Project(const std::string& name) = 0;
  [[nodiscard]] virtual std::string Project() const = 0;

  virtual void Subject(const std::string& subject) = 0;
  [[nodiscard]] virtual std::string Subject() const = 0;

  virtual void Description(const std::string& description) = 0;
  [[nodiscard]] virtual std::string Description() const = 0;

  virtual void MeasurementId(const std::string& uuid);
  [[nodiscard]] virtual std::string MeasurementId() const;

  virtual void RecorderId(const std::string& uuid);
  [[nodiscard]] virtual std::string RecorderId() const;

  virtual void RecorderIndex(int64_t index);
  [[nodiscard]] virtual int64_t RecorderIndex() const;

  virtual void StartTime(uint64_t ns_since_1970) = 0;
  [[nodiscard]] virtual uint64_t StartTime() const  = 0;

  virtual void MetaData(const std::string& meta_data) = 0;
  [[nodiscard]] virtual std::string MetaData() const = 0;

  [[nodiscard]] virtual std::vector<IDataGroup*> DataGroups() const = 0;

  [[nodiscard]] virtual IDataGroup* LastDataGroup() const = 0;
 protected:
  virtual ~IHeader() = default;
};


} // end namespace mdf