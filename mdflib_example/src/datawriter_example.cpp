/*
 * Simple example that writes a few samples using the DataWriter API.
 * Does not depend on GTest or Boost; links only to the mdflib library.
 */

#include <iostream>
#include <filesystem>
#include <vector>
#include <cstring>

#include "mdf/mdffactory.h"
#include "mdf/mdfwriter.h"
#include "mdf/mdfreader.h"
#include "mdf/idatawriter.h"
#include "mdf/idatagroup.h"
#include "mdf/ichannelgroup.h"
#include "mdf/ichannel.h"

using namespace std::filesystem;
using namespace mdf;

int main() {
  try {
    const std::string filename = (current_path() / "datawriter_example.mf4").string();

    auto writer = MdfFactory::CreateMdfWriter(MdfWriterType::Mdf4Basic);
    if (!writer || !writer->Init(filename)) {
      std::cerr << "Failed to create or init MdfWriter\n";
      return 1;
    }

    auto* dg = writer->CreateDataGroup();
    auto* cg = MdfWriter::CreateChannelGroup(dg);

    // Master time channel
    {
      auto* master = MdfWriter::CreateChannel(cg);
      master->Name("Time");
      master->Type(ChannelType::Master);
      master->Sync(ChannelSyncType::Time);
      master->DataType(ChannelDataType::FloatLe);
      master->DataBytes(sizeof(float));
      master->Unit("s");
    }

    // Simple integer signal
    {
      auto* signal = MdfWriter::CreateChannel(cg);
      signal->Name("Value");
      signal->Type(ChannelType::FixedLength);
      signal->DataType(ChannelDataType::SignedIntegerLe);
      signal->DataBytes(sizeof(int32_t));
    }

    if (!writer->InitMeasurement()) {
      std::cerr << "Failed to init measurement\n";
      return 2;
    }

    const uint64_t start_time = 1'000'000'000ULL;
    writer->StartMeasurement(start_time);

    // Create DataWriter from the channel group
    auto data_writer = MdfFactory::CreateDataWriter(*cg);
    if (!data_writer) {
      std::cerr << "Failed to create DataWriter\n";
      return 3;
    }

    const auto layouts = data_writer->ChannelLayouts();
    // DataWriter channel layouts do not expose the master channel. The
    // master (Time) channel value is set by SaveSample() based on the
    // timestamp passed to it. Only write the signal values here.
    size_t value_offset = 0;
    for (const auto& l : layouts) {
      if (l.name == "Value") value_offset = l.byte_offset;
    }

    // Print channel offsets for debugging
    auto* master_ch = cg->GetMasterChannel();
    auto* sig_ch = cg->GetChannel("Value");
    if (master_ch) {
      std::cout << "Master ByteOffset=" << master_ch->ByteOffset()
                << " DataBytes=" << master_ch->DataBytes() << std::endl;
    }
    if (sig_ch) {
      std::cout << "Value ByteOffset=" << sig_ch->ByteOffset()
                << " DataBytes=" << sig_ch->DataBytes()
                << " DataType=" << static_cast<int>(sig_ch->DataType()) << std::endl;
    }

    const size_t samples = 100;
    for (size_t s = 0; s < samples; ++s) {
      data_writer->Reset();
      auto& buf = data_writer->Buffer();

      // Master/time channel is not written here. SaveSample() will set master
      // time from the supplied timestamp. Only write the signal value.
      auto v = static_cast<int32_t>(100 + s);
      std::memcpy(buf.data() + value_offset, &v, sizeof(v));
      if (s == 0) {
        std::cout << "value_offset=" << value_offset << " buffer_size=" << buf.size() << "\n";
        std::cout << "buffer bytes at offset: ";
        for (size_t b = 0; b < std::min<size_t>(16, buf.size()); ++b) {
          std::cout << std::hex << (int)buf[b] << " ";
        }
        std::cout << std::dec << "\n";
      }
      SampleRecord rec = data_writer->Commit();

      // Debug: verify group's internal sample buffer
      if (s == 0) {
        std::cout << "group buffer bytes at offset: ";
        for (size_t b = 0; b < std::min<size_t>(16, rec.record_buffer.size()); ++b) {
          std::cout << std::hex << (int)rec.record_buffer[b] << " ";
        }
        std::cout << std::dec << "\n";
      }

      writer->AddSample(*dg, *cg,
        start_time + static_cast<uint64_t>(s) * 1'000'000ULL,
        std::move(rec));
    }

    writer->StopMeasurement(start_time + 10'000'000ULL);
    if (!writer->FinalizeMeasurement()) {
      std::cerr << "FinalizeMeasurement failed\n";
      return 5;
    }

    std::cout << "Wrote example file: " << filename << "\n";

    // Quick read-back
    MdfReader reader(filename);
    if (!reader.ReadEverythingButData()) {
      std::cerr << "Reader failed to read metadata\n";
      return 6;
    }
    auto* read_dg = reader.GetDataGroup(0);
    if (!read_dg) {
      std::cerr << "No data group found\n";
      return 7;
    }
    auto groups = read_dg->ChannelGroups();
    if (groups.empty()) {
      std::cerr << "No channel groups found\n";
      return 8;
    }

    // Prepare channel group pointer and create channel observers BEFORE reading data
    auto* cg_read = groups[0];
    const auto* time_ch = cg_read->GetChannel("Time");
    const auto* value_ch = cg_read->GetChannel("Value");
    auto time_obs = CreateChannelObserver(*read_dg, *cg_read, *time_ch);
    auto value_obs = CreateChannelObserver(*read_dg, *cg_read, *value_ch);

    // Attach a temporary sample observer to print raw record bytes during read
    std::unique_ptr<mdf::ISampleObserver> raw_obs = std::make_unique<mdf::ISampleObserver>(*read_dg);
    raw_obs->DoOnSample = [cg_read](uint64_t sample, uint64_t record_id, const std::vector<uint8_t>& record)->bool {
      if (record_id == cg_read->RecordId()) {
        std::cout << "OnSample raw (sample=" << sample << ") bytes: ";
        for (size_t i = 0; i < std::min<size_t>(16, record.size()); ++i) {
          std::cout << std::hex << (int)record[i] << " ";
        }
        std::cout << std::dec << "\n";
        const auto* time_ch_local = cg_read->GetChannel("Time");
        const auto* value_ch_local = cg_read->GetChannel("Value");
        if (time_ch_local) {
          double tt = 0.0; bool ok = time_ch_local->GetChannelValue(record, tt);
          std::cout << "  time_ch GetChannelValue ok=" << ok << " val=" << tt << "\n";
        }
        if (value_ch_local) {
          int64_t vv = 0; bool okv = value_ch_local->GetChannelValue(record, vv);
          std::cout << "  value_ch GetChannelValue ok=" << okv << " val=" << vv << "\n";
        }
      }
      return true;
    };

    
    std::cout << "Read back data group with " << groups.size() << " channel group(s)" << std::endl;
    if (time_ch) std::cout << "READ Master ByteOffset=" << time_ch->ByteOffset() << " DataBytes=" << time_ch->DataBytes() << std::endl;
    if (value_ch) std::cout << "READ Value ByteOffset=" << value_ch->ByteOffset() << " DataBytes=" << value_ch->DataBytes() << std::endl;
    if (time_ch && value_ch) {
      auto time_obs = CreateChannelObserver(*read_dg, *cg_read, *time_ch);
      auto value_obs = CreateChannelObserver(*read_dg, *cg_read, *value_ch);
      if (time_obs && value_obs) {
        reader.ReadData(*read_dg);
        const uint64_t nof = time_obs->NofSamples();
        std::cout << "Samples: " << nof << std::endl;
        for (uint64_t i = 0; i < nof; ++i) {
          double t = 0.0;
          int64_t v = 0;
          auto time_valid = time_obs->GetChannelValue(i, t);
          auto value_valid = value_obs->GetChannelValue(i, v);

          std::cout<<"time valid:"<<time_valid<<" value valid:"<<value_valid<<std::endl;
          std::cout << "Sample " << i << ": time=" << t << " value=" << v << std::endl;
        }
      }
    }

    return 0;
  } catch (const std::exception& ex) {
    std::cerr << "Exception: " << ex.what() << std::endl;
    return -1;
  }
}
