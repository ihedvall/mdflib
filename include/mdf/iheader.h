/*
 * Copyright 2021 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

/** \file iheader.h
 * \brief Interface against a HD block.
 *
 * Defines an interface against a HD block in an MDF file.
 */
#pragma once
#include <string>
#include "mdf/idatagroup.h"
#include "mdf/iattachment.h"
#include "mdf/imetadata.h"
#include "mdf/ifilehistory.h"

namespace mdf {

/** \class IHeader iheader.h "mdf/iheader.h"
 * \brief Interface class against an MDF HD block
 *
 * Interface class against a header block (HD) in a MDF file. The header
 * block is the main root that holds all other blocks except the ID block.
 */
class IHeader {
 public:
  /** \brief Returns the file position.
   *
   * Returns the file position of the block. This index is updated when
   * the block is saved onto the file. The index is also used to indicate
   * if the block has been written to the file.
   * @return File position
   */
  [[nodiscard]] virtual int64_t Index() const = 0;

  /** \brief Sets the author of the file.
   *
   * Sets the author of the files.
   * @param author Author of the file.
   */
  virtual void Author(const std::string& author) = 0;

  /** \brief Returns the author of the file.
   *
   * Returns the author of the file.
   * @return Author of the file.
   */
  [[nodiscard]] virtual std::string Author() const = 0;

  /** \brief Sets the department name.
   *
   * Sets the department name.
   * @param department Department name.
   */
  virtual void Department(const std::string& department) = 0;

  /** \brief Returns the department.
   *
   * Returns the department name.
   * @return Department name.
   */
  [[nodiscard]] virtual std::string Department() const = 0;

  /** \brief Sets the project name.
   *
   * Sets the project name.
   * @param name Project name.
   */
  virtual void Project(const std::string& name) = 0;

  /** \brief Returns the project name.
   *
   * Returns the project name.
   * @return Project name
   */
  [[nodiscard]] virtual std::string Project() const = 0;

  /** \brief Sets the subject of the file.
   *
   * Sets the subject of the file.
   * @param subject Subject of the file.
   */
   virtual void Subject(const std::string& subject) = 0;

   /** \brief Returns the subject of the file.
    *
    * Returns the subject of the file
    * @return Subject of the file.
    */
  [[nodiscard]] virtual std::string Subject() const = 0;

  /** \brief Sets the description of the file.
   *
   * Sets the description of the file.
   * @param description Description of the file.
   */
  virtual void Description(const std::string& description) = 0;

  /** \brief Returns the description of the file.
   *
   * Returns the description of the file.
   * @return Description of the file.
   */
  [[nodiscard]] virtual std::string Description() const = 0;

  /** \brief Sets a unique identifier for the measurement.
   *
   * Sets a unique identifier (UUID) for the file. This identify files
   * from the same measurement.
   * @param uuid Unique identifier (UUID).
   */
  virtual void MeasurementId(const std::string& uuid);

  /** \brief Returns the unique measurement identifier.
   *
   * Returns the unique measurement identifier of the file.
   * @return Unique identifier (UUID) of the measurement.
   */
  [[nodiscard]] virtual std::string MeasurementId() const;

  /** \brief Sets a unique identifier for the recorder.
   *
   * Sets a unique identifier (UUID) for measurements from the same
   * recorder.
   * @param uuid Unique identifier (UUID).
   */
  virtual void RecorderId(const std::string& uuid);

  /** \brief Returns the unique recorder identifier.
 *
 * Returns the unique recorder identifier of the file.
 * @return Unique identifier (UUID) of the recorder.
 */
  [[nodiscard]] virtual std::string RecorderId() const;

  /** \brief Sets the recorder order number.
   *
   * Sets the recorder order number.
   * @param index Order number
   */
  virtual void RecorderIndex(int64_t index);

  /** \brief Returns the recorder order number
   *
   * Returns the recorder order number.
   * @return Order number.
   */
  [[nodiscard]] virtual int64_t RecorderIndex() const;

  /** \brief Sets the absolute measurement time for the file.
   *
   * Sets the absolute start time for the measurement file.
   * @param ns_since_1970 Nanoseconds since 1970.
   */
  virtual void StartTime(uint64_t ns_since_1970) = 0;

  /** \brief Returns the absolute measurement time for the file.
   *
   * Returns the absolute start time for the measurement file.
   * @return Nanoseconds since 1970.
   */
  [[nodiscard]] virtual uint64_t StartTime() const  = 0;

  /** \brief Returns meta data information object.
   *
   * Returns meta data as a text string typically as an XML snippets.
   * Note that this function also creates the MD block if it is missing.
   * @return Returns meta data.
   */
  [[nodiscard]] virtual IMetaData* MetaData();

  /** \brief Returns meta data information object.
   *
   * Returns meta data as a text string typically as an XML snippets.
   * @return Returns meta data or a null pointer if no MD block.
   */
  [[nodiscard]] virtual const IMetaData* MetaData() const;

  /** \brief Create a new attachment block.
   *
   * Creates a new attachment block (AT) that reference an external file. This
   * file can be just referenced or embedded into the MDF file. If it is
   * embedded, the file may be compressed as well. Note that MDF3 doesn't
   * support attachments.
   * @return An attachment interface pointer.
   */
  [[nodiscard]] virtual IAttachment* CreateAttachment();

  /** \brief Returns a list of attachments.
   *
   * Returns a list of attachments in the measurement file.
   * @return
   */
  [[nodiscard]] virtual std::vector<IAttachment*> Attachments() const;

  /** \brief Create a new file history block.
   *
   * Creates a new file history (FH) block that reference a change/update of the MDF file.
   * @return A file history  interface pointer.
   */
  [[nodiscard]] virtual IFileHistory* CreateFileHistory();

  /** \brief Returns a list of file history blocks.
   *
   * Returns a list of file history blocks in the measurement file.
   * @return
   */
  [[nodiscard]] virtual std::vector<IFileHistory*> FileHistories() const;

  /** \brief Returns a list of measurements.
   *
   * Returns a list of measurements in the file also known as DG blocks.
   * @return List of measurements.
   */
  [[nodiscard]] virtual std::vector<IDataGroup*> DataGroups() const = 0;


  /** \brief Returns the last measurement in the file.
   *
   * Returns the last measurement in the file.
   * @return Last DG block.
   */
  [[nodiscard]] virtual IDataGroup* LastDataGroup() const = 0;

  /** \brief Sets the start angle for the measurements.
   *
   * Sets the start angle for the measurements in radians.
   * @param angle Start angle.
   */
  virtual void StartAngle(double angle);

  /** Returns the start angle.
   *
   * Returns the start angle of the measurement in radians.
   * @param valid Set tor true if the angle value is valid.
   * @return Start angle in radians.
   */
  [[nodiscard]] virtual std::optional<double> StartAngle() const;

  /** \brief Sets the start distance.
   *
   * Sets the start distance in meter for all measurements.
   * @param distance Start distance in meters.
   */
  virtual void StartDistance(double distance);

  /** \brief Returns the start distance in meters.
   *
   * Sets the start distance in meters.
   * @param valid True if the value is valid.
   * @return Start distance in meters.
   */
  [[nodiscard]] virtual std::optional<double> StartDistance() const;
 protected:
  virtual ~IHeader() = default; ///< Default constructor
};


} // end namespace mdf