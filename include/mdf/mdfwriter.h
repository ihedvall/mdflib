/*
 * Copyright 2022 Ingemar Hedvall
 * SPDX-License-Identifier: MIT
 */

#pragma once
#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <string>
#include <thread>

#include "mdf/mdffile.h"
#include "samplerecord.h"
/** \file mdfwriter.h
 * \brief Interface against an MDF writer object.
 */
namespace mdf {

class IChannelGroup;
class IChannel;
class IChannelConversion;
class IDataGroup;

/** \brief Interface against an MDF writer object.
 *
 * The MDF writer purpose is to create MDF files. It simplifies the writing
 * into some steps.
 *
 * The first is to create type of writer. This is done by using the
 * MdfFactory::CreateMdfWriter() function.
 *
 * The next step is to call the Init() function with a filename with valid path.
 * The Init function will create the actual MDF file object and check if it
 * exist or not. If it exist, it reads in the existing file content so it can
 * be appended.
 *
 * The next step is to prepare the file for a new measurement. This is done
 * by creating the DG/CG/CN/CC blocks that defines the measurement. Note that
 * it is the last DG block that is the target.
 *
 * Next step is to call the InitMeasurement() function. This create a thread
 * that handle the queue of samples. The function also write the configuration
 * to the file and closes it.
 *
 * The user shall know starts adding samples to the queue by first setting
 * the current channel value to each channel and then call the SaveSample()
 * function for each channel group (CG). Note that no samples are saved to the
 * file yet. The max queue size is set to the pre-trig time, see PreTrigTime().
 *
 * At some point the user shall call the StartMeasurement() function. The
 * sample queue is now saved onto the file. The save rate is actually dependent
 * if CompressData() is enabled or not. If compression is used the data is
 * saved at the 4MB size or a 10 minute max. If not using compression, the
 * samples are saved each 10 second.
 *
 * The user shall now call StopMeasurement() which flush out the remaining
 * sample queue. After stopping the queue, the user may add some extra block
 * as event (EV) and attachment (AT) blocks.
 *
 * The FinalizeMeasurement() function. Stops the thread and write all
 * unwritten blocks to the file.
 *
 */
class MdfWriter {
 public:
  MdfWriter() = default; ///< Default constructor.
  virtual ~MdfWriter();  ///< Default destructor.

  MdfWriter(const MdfWriter& writer) = delete;
  MdfWriter& operator=(const MdfWriter& writer) = delete;

  /** \brief Returns the filename without extension and path (stem). */
  [[nodiscard]] std::string Name() const;

  /** \brief Initiate the file.
   *
   * Initiate the writer by defining which file is shall work with. Note that
   * appending existing file is supported.
   * @param filename Filename with full path.
   * @return Returns true if the function was successful.
   */
  bool Init(const std::string& filename);

  /** \brief Returns true if this is a new file. */
  [[nodiscard]] bool IsFileNew() const {
    return write_state_ == WriteState::Create;
  }

  /** \brief Sets the pre-trig time (s) of the writer.
   *
   * Sets the pre-trig time (note seconds). This defines the maximum number of
   * samples the writer holds internally, before the measurement is started.
   * @param pre_trig_time Pre-trig time in seconds.
   */
  void PreTrigTime(double pre_trig_time);
  [[nodiscard]] double PreTrigTime() const; ///< Pre-trig time (s).

  /** \brief Returns start time in nano-seconds since 1970. */
  [[nodiscard]] uint64_t StartTime() const { return start_time_; }
  /** \brief Returns stop time in nano-seconds since 1970. */
  [[nodiscard]] uint64_t StopTime() const { return stop_time_; }

  /** \brief Returns the MDF file interface.
   *
   * Returns the MDF file interface. The user may change the file version and
   * the finalize status of the file. By default is the MDF 4.2 or 3.2 version
   * of the file used.
   * @return Pointer to the MDF file interface.
   */
  MdfFile* GetFile() { return mdf_file_.get(); }

  [[nodiscard]] IHeader* Header() const; ///< Returns the header block (HD).
  /** \brief Create a new data group (DG) block. */
  [[nodiscard]] IDataGroup* CreateDataGroup();
  /** \brief Create a new channel group (CG) block. */
  [[nodiscard]] static IChannelGroup* CreateChannelGroup(IDataGroup* parent);
  /** \brief Creates a new channel (CN) block. */
  [[nodiscard]] static IChannel* CreateChannel(IChannelGroup* parent);
  /** \brief Create a new channel conversion (CC) block. */
  virtual IChannelConversion* CreateChannelConversion(IChannel* parent) = 0;

  /** \brief Initialize the sample queue and write any unwritten block to the
   * file.
   */
  virtual bool InitMeasurement();
  /** \brief Saves a sample record for a channel group. */
  void SaveSample(IChannelGroup& group, uint64_t time);
  /** \brief Starts the measurement. */
  void StartMeasurement(uint64_t start_time);
  /** \brief Stops the measurement. */
  void StopMeasurement(uint64_t stop_time);
  /** \brief Stop the sample queue and write all unwritten blocks to
   * the file.*/
  bool FinalizeMeasurement();
   /** \brief If set to true, the data block will be compressed. */
  void CompressData(bool compress) {compress_data_ = compress;}
  /** \brief Returns true if the data block is compressed. */
  [[nodiscard]] bool CompressData() const { return compress_data_;}

 protected:
  /** \brief Internal state of the thread. */
  enum class WriteState : uint8_t {
    Create,     ///< Only at first measurement
    Init,       ///< Start work thread and start collecting samples
    StartMeas,  ///< Start saving samples to file
    StopMeas,   ///< Stop saving samples. OK to
    Finalize    ///< OK to add new DG and CG blocks
  };
  std::atomic<WriteState> write_state_ =
      WriteState::Create;  ///< Keeps track of the worker thread state.

  std::unique_ptr<MdfFile> mdf_file_;  ///< Holds the actual file object.
  std::string filename_;  ///< Full name of file with path and extension.

  std::atomic<uint64_t> pre_trig_time_ = 0;  ///< Nanoseconds difference.
  std::atomic<uint64_t> start_time_ = 0;     ///< Nanoseconds since 1970.
  std::atomic<uint64_t> stop_time_ = 0;      ///< Nanoseconds since 1970.

  std::thread work_thread_; ///< Sample queue thread.
  std::atomic_bool stop_thread_ = false; ///< Set to true to stop the thread.
  std::mutex locker_; ///< Mutex for thread-safe handling of the sample queue.
  std::condition_variable sample_event_; ///< Used internally.


  using SampleQueue = std::deque<SampleRecord>; ///< Sample queue
  SampleQueue sample_queue_; ///< Sample queue

  virtual void CreateMdfFile() = 0; ///< Creates an MDF file
  virtual bool PrepareForWriting() = 0; ///< Prepare for writing.
  virtual void SetDataPosition(std::FILE* file); ///< Set the data position.
  virtual bool WriteSignalData(std::FILE* file); ///< Write an SD block.

  void StopWorkThread(); ///< Stops the worker thread
  void WorkThread(); ///< Worker thread function
  void TrimQueue(); ///< Trims the sample queue.
  /** \brief Saves the queue to file. */
  virtual void SaveQueue(std::unique_lock<std::mutex>& lock);
  /** \brief Flush the sample queue. */
  virtual void CleanQueue(std::unique_lock<std::mutex>& lock);
  /** \brief Increment the sample counter. */
  void IncrementNofSamples(uint64_t record_id) const;
  /** \brief Set the last file position. */
  virtual void SetLastPosition(std::FILE* file) = 0;

 private:
  bool compress_data_ = false; ///< True if the data shall be compressed.
};

}  // namespace mdf