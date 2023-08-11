#include <filesystem>
#include <iostream>

// Pure C example
#include <mdflibrary/MdfExport.h>
using namespace MdfLibrary;
using namespace MdfLibrary::ExportFunctions;
void c_example() {
  std::cout << "C example" << std::endl;

  if (std::filesystem::exists("test_c.mf4"))
    std::filesystem::remove("test_c.mf4");

  {
    std::cout << "Write" << std::endl;
    auto* Writer = MdfWriterInit(MdfWriterType::Mdf4Basic, "test_c.mf4");
    std::cout << "Writer: " << Writer << std::endl;
    auto* Header = MdfWriterGetHeader(Writer);
    std::cout << "Header: " << Header << std::endl;
    MdfHeaderSetAuthor(Header, "Caller");
    MdfHeaderSetDepartment(Header, "Home Alone");
    MdfHeaderSetDescription(Header, "Testing i");
    MdfHeaderSetProject(Header, "Mdf3WriteHD");
    MdfHeaderSetStartTime(Header, 1000);
    MdfHeaderSetSubject(Header, "PXY");
    auto* History = MdfHeaderCreateFileHistory(Header);
    MdfFileHistorySetTime(History, 1000000);
    MdfFileHistorySetDescription(History, "Initial stuff");
    MdfFileHistorySetToolName(History, "Unit Test");
    MdfFileHistorySetToolVendor(History, "ACME");
    MdfFileHistorySetToolVersion(History, "2.3");
    MdfFileHistorySetUserName(History, "Ducky");

    auto* dg = MdfWriterCreateDataGroup(Writer);
    std::cout << "DataGroup: " << dg << std::endl;
    auto* cg = MdfDataGroupCreateChannelGroup(dg);
    std::cout << "ChannelGroup: " << cg << std::endl;
    MdfChannelGroupSetName(cg, "Test");
    MdfChannelGroupSetDescription(cg, "Test channel group");

    auto* si = MdfChannelGroupCreateSourceInformation(cg);
    std::cout << "SourceInformation: " << si << std::endl;
    MdfSourceInformationSetName(si, "SI-Name");
    MdfSourceInformationSetPath(si, "SI-Path");
    MdfSourceInformationSetDescription(si, "SI-Desc");
    MdfSourceInformationSetType(si, SourceType::Bus);
    MdfSourceInformationSetBus(si, BusType::Can);

    {
      auto* cn = MdfChannelGroupCreateChannel(cg);
      std::cout << "Channel: " << cn << std::endl;
      MdfChannelSetName(cn, "Time");
      MdfChannelSetDescription(cn, "Time channel");
      MdfChannelSetType(cn, ChannelType::Master);
      MdfChannelSetSync(cn, ChannelSyncType::Time);
      MdfChannelSetDataType(cn, ChannelDataType::FloatLe);
      MdfChannelSetDataBytes(cn, 4);
      MdfChannelSetUnit(cn, "s");
      MdfChannelSetRange(cn, 0, 100);
    }

    {
      auto* cn = MdfChannelGroupCreateChannel(cg);
      MdfChannelSetName(cn, "SignedLe");
      MdfChannelSetDescription(cn, "int32_t");
      MdfChannelSetType(cn, ChannelType::FixedLength);
      MdfChannelSetDataType(cn, ChannelDataType::SignedIntegerLe);
      MdfChannelSetDataBytes(cn, sizeof(int32_t));
    }
    {
      auto* cn = MdfChannelGroupCreateChannel(cg);
      MdfChannelSetName(cn, "SignedBe");
      MdfChannelSetDescription(cn, "int8_t");
      MdfChannelSetType(cn, ChannelType::FixedLength);
      MdfChannelSetDataType(cn, ChannelDataType::SignedIntegerLe);
      MdfChannelSetDataBytes(cn, sizeof(int8_t));
    }
    {
      auto* cn = MdfChannelGroupCreateChannel(cg);
      MdfChannelSetName(cn, "FloatLe");
      MdfChannelSetDescription(cn, "float");
      MdfChannelSetType(cn, ChannelType::FixedLength);
      MdfChannelSetDataType(cn, ChannelDataType::FloatLe);
      MdfChannelSetDataBytes(cn, sizeof(float));
    }
    {
      auto* cn = MdfChannelGroupCreateChannel(cg);
      MdfChannelSetName(cn, "FloatBe");
      MdfChannelSetDescription(cn, "double");
      MdfChannelSetType(cn, ChannelType::FixedLength);
      MdfChannelSetDataType(cn, ChannelDataType::FloatBe);
      MdfChannelSetDataBytes(cn, sizeof(double));
    }

    size_t size = MdfChannelGroupGetChannels(cg, nullptr);
    std::cout << "ChannelGroupGetChannels: " << size << std::endl;
    auto pChannel = new mdf::IChannel*[size];
    size = MdfChannelGroupGetChannels(cg, pChannel);
    for (size_t i = 0; i < size; i++) {
      std::cout << "Channel" << i << ": " << pChannel[i] << std::endl;
    }

    MdfWriterInitMeasurement(Writer);
    MdfWriterStartMeasurement(Writer, 100000000);
    std::cout << "Start measure" << std::endl;
    for (size_t i = 0; i < 50; i++) {
      MdfChannelSetChannelValueAsFloat(pChannel[1], i * 2);
      MdfChannelSetChannelValueAsFloat(pChannel[2], i * 3);
      MdfChannelSetChannelValueAsFloat(pChannel[3], i * 4);
      MdfChannelSetChannelValueAsFloat(pChannel[4], i * 5);
      MdfWriterSaveSample(Writer, cg, 100000000 + i * 1000000);
    }
    std::cout << "Stop measure" << std::endl;
    MdfWriterStopMeasurement(Writer, 1100000000);
    MdfWriterFinalizeMeasurement(Writer);
  }
}

// C++ example
#include <mdflibrary/MdfChannelObserver.h>
#include <mdflibrary/MdfReader.h>
#include <mdflibrary/MdfWriter.h>
void cpp_example() {
  std::cout << "C++ example" << std::endl;

  if (std::filesystem::exists("test_cpp.mf4"))
    std::filesystem::remove("test_cpp.mf4");

  {
    std::cout << "Write" << std::endl;
    MdfWriter Writer(MdfWriterType::Mdf4Basic, "test_cpp.mf4");
    MdfHeader Header = Writer.GetHeader();
    Header.SetAuthor("Caller");
    Header.SetDepartment("Home Alone");
    Header.SetDescription("Testing i");
    Header.SetProject("Mdf3WriteHD");
    Header.SetStartTime(1000);
    Header.SetSubject("PXY");
    auto History = Header.CreateFileHistory();
    History.SetTime(1000000);
    History.SetDescription("Initial stuff");
    History.SetToolName("Unit Test");
    History.SetToolVendor("ACME");
    History.SetToolVersion("2.3");
    History.SetUserName("Ducky");

    auto dg = Writer.CreateDataGroup();
    auto cg = dg.CreateChannelGroup();
    cg.SetName("Test");
    cg.SetDescription("Test channel group");

    auto si = cg.CreateSourceInformation();
    si.SetName("SI-Name");
    si.SetPath("SI-Path");
    si.SetDescription("SI-Desc");
    si.SetType(SourceType::Bus);
    si.SetBus(BusType::Can);

    {
      auto cn = cg.CreateChannel();
      cn.SetName("Time");
      cn.SetDescription("Time channel");
      cn.SetType(ChannelType::Master);
      cn.SetSync(ChannelSyncType::Time);
      cn.SetDataType(ChannelDataType::FloatLe);
      cn.SetDataBytes(4);
      cn.SetUnit("s");
      cn.SetRange(0, 100);
    }

    {
      auto cn = cg.CreateChannel();
      cn.SetName("SignedLe");
      cn.SetDescription("int32_t");
      cn.SetType(ChannelType::FixedLength);
      cn.SetDataType(ChannelDataType::SignedIntegerLe);
      cn.SetDataBytes(sizeof(int32_t));
    }
    {
      auto cn = cg.CreateChannel();
      cn.SetName("SignedBe");
      cn.SetDescription("int8_t");
      cn.SetType(ChannelType::FixedLength);
      cn.SetDataType(ChannelDataType::SignedIntegerLe);
      cn.SetDataBytes(sizeof(int8_t));
    }
    {
      auto cn = cg.CreateChannel();
      cn.SetName("FloatLe");
      cn.SetDescription("float");
      cn.SetType(ChannelType::FixedLength);
      cn.SetDataType(ChannelDataType::FloatLe);
      cn.SetDataBytes(sizeof(float));
    }
    {
      auto cn = cg.CreateChannel();
      cn.SetName("FloatBe");
      cn.SetDescription("double");
      cn.SetType(ChannelType::FixedLength);
      cn.SetDataType(ChannelDataType::FloatBe);
      cn.SetDataBytes(sizeof(double));
    }

    auto channels = cg.GetChannels();
    std::cout << "ChannelGroupGetChannels: " << channels.size() << std::endl;

    Writer.InitMeasurement();
    Writer.StartMeasurement(100000000);
    std::cout << "Start measure" << std::endl;
    for (size_t i = 0; i < 50; i++) {
      channels[1].SetChannelValue(i * 2);
      channels[2].SetChannelValue(i * 3);
      channels[3].SetChannelValue(i * 4);
      channels[4].SetChannelValue(i * 5);
      Writer.SaveSample(cg, 100000000 + i * 1000000);
      std::cout << "Save sample " << i << std::endl;
    }
    std::cout << "Stop measure" << std::endl;
    Writer.StopMeasurement(1100000000);
    Writer.FinalizeMeasurement();
  }

  {
    std::cout << "Read" << std::endl;
    MdfReader Reader("test_cpp.mf4");
    Reader.ReadEverythingButData();
    auto Header = Reader.GetHeader();
    std::cout << "Author: " << Header.GetAuthor().c_str() << std::endl;
    std::cout << "Department: " << Header.GetDepartment() << std::endl;
    std::cout << "Description: " << Header.GetDescription() << std::endl;
    std::cout << "Project: " << Header.GetProject() << std::endl;
    std::cout << "StartTime: " << Header.GetStartTime() << std::endl;
    std::cout << "Subject: " << Header.GetSubject() << std::endl;

    auto Historys = Header.GetFileHistorys();
    std::cout << "History: " << Historys.size() << std::endl;
    for (const auto& Histroy : Historys) {
      std::cout << "Time: " << Histroy.GetTime() << std::endl;
      std::cout << "Description: " << Histroy.GetDescription() << std::endl;
      std::cout << "ToolName: " << Histroy.GetToolName() << std::endl;
      std::cout << "ToolVendor: " << Histroy.GetToolVendor() << std::endl;
      std::cout << "ToolVersion: " << Histroy.GetToolVersion() << std::endl;
      std::cout << "UserName: " << Histroy.GetUserName() << std::endl;
      std::cout << std::endl;
    }

    auto DataGroups = Header.GetDataGroups();
    std::cout << "DataGroups: " << DataGroups.size() << std::endl;
    for (const auto& DataGroup : DataGroups) {
      auto ChannelGroups = DataGroup.GetChannelGroups();
      std::cout << "ChannelGroups: " << ChannelGroups.size() << std::endl;
      for (const auto& ChannelGroup : ChannelGroups) {
        std::cout << "Name: " << ChannelGroup.GetName() << std::endl;
        std::cout << "Description: " << ChannelGroup.GetDescription()
                  << std::endl;

        auto SourceInformation = ChannelGroup.GetSourceInformation();
        std::cout << "SI Name: " << SourceInformation.GetName() << std::endl;
        std::cout << "SI Path: " << SourceInformation.GetPath() << std::endl;
        std::cout << "SI Description: " << SourceInformation.GetDescription()
                  << std::endl;

        std::cout << "Nof Samples: " << ChannelGroup.GetNofSamples()
                  << std::endl;

        auto Channels = ChannelGroup.GetChannels();
        std::cout << "Channels: " << Channels.size() << std::endl;
        std::vector<MdfLibrary::MdfChannelObserver> Observers;
        for (const auto& Channel : Channels) {
          std::cout << "Name: " << Channel.GetName() << std::endl;
          std::cout << "Description: " << Channel.GetDescription() << std::endl;
          std::cout << "Type: " << static_cast<int>(Channel.GetType())
                    << std::endl;
          std::cout << "Sync: " << static_cast<int>(Channel.GetSync())
                    << std::endl;
          std::cout << "DataType: " << static_cast<int>(Channel.GetDataType())
                    << std::endl;
          std::cout << "DataBytes: " << Channel.GetDataBytes() << std::endl;
          std::cout << "Unit: " << Channel.GetUnit() << std::endl;
          std::cout << std::endl;

          Observers.push_back(
              MdfChannelObserver(DataGroup, ChannelGroup, Channel));
        }

        Reader.ReadData(DataGroup);

        for (size_t i = 0; i < ChannelGroup.GetNofSamples(); i++) {
          std::cout << "Sample: " << i << std::endl;
          for (const auto& Observer : Observers) {
            switch (Observer.GetChannel().GetDataType()) {
              case ChannelDataType::CanOpenDate:
              case ChannelDataType::CanOpenTime: {
                uint64_t channel_value, eng_value;
                Observer.GetChannelValue(i, channel_value);
                Observer.GetEngValue(i, eng_value);
                std::cout << "Channel: " << channel_value
                          << ", Eng: " << eng_value << std::endl;
                break;
              }
              case ChannelDataType::UnsignedIntegerLe:
              case ChannelDataType::UnsignedIntegerBe: {
                uint64_t channel_value, eng_value;
                Observer.GetChannelValue(i, channel_value);
                Observer.GetEngValue(i, eng_value);
                std::cout << "Channel: " << channel_value
                          << ", Eng: " << eng_value << std::endl;
                break;
              }
              case ChannelDataType::SignedIntegerLe:
              case ChannelDataType::SignedIntegerBe: {
                int64_t channel_value, eng_value;
                Observer.GetChannelValue(i, channel_value);
                Observer.GetEngValue(i, eng_value);
                std::cout << "Channel: " << channel_value
                          << ", Eng: " << eng_value << std::endl;
                break;
              }
              case ChannelDataType::FloatLe:
              case ChannelDataType::FloatBe: {
                double channel_value, eng_value;
                Observer.GetChannelValue(i, channel_value);
                Observer.GetEngValue(i, eng_value);
                std::cout << "Channel: " << channel_value
                          << ", Eng: " << eng_value << std::endl;
                break;
              }
              case ChannelDataType::StringAscii:
              case ChannelDataType::StringUTF8:
              case ChannelDataType::StringUTF16Le:
              case ChannelDataType::StringUTF16Be: {
                std::string channel_value, eng_value;
                Observer.GetChannelValue(i, channel_value);
                Observer.GetEngValue(i, eng_value);
                std::cout << "Channel: " << channel_value
                          << ", Eng: " << eng_value << std::endl;
                break;
              }
              case ChannelDataType::MimeStream:
              case ChannelDataType::MimeSample:
              case ChannelDataType::ByteArray: {
                std::vector<uint8_t> channel_value, eng_value;
                Observer.GetChannelValue(i, channel_value);
                Observer.GetEngValue(i, eng_value);
                std::cout << "Channel: " << channel_value.size()
                          << ", Eng: " << eng_value.size() << std::endl;
                break;
              }
              default:
                break;
            }
          }
          std::cout << std::endl;
        }
        std::cout << std::endl;
      }
      std::cout << std::endl;
    }
  }
}

int main() {
  c_example();
  cpp_example();
  return 0;
}
