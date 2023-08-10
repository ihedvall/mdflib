#include <iostream>

// Pure C example
#include <mdflibrary/MdfExport.h>
using namespace MdfLibrary;
using namespace MdfLibrary::ExportFunctions;
void c_example() {
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

// C++ example
#include <mdflibrary/MdfWriter.h>
void cpp_example() {
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
    channels[1].SetChannelValueAsFloat(i * 2);
    channels[2].SetChannelValueAsFloat(i * 3);
    channels[3].SetChannelValueAsFloat(i * 4);
    channels[4].SetChannelValueAsFloat(i * 5);
    Writer.SaveSample(cg, 100000000 + i * 1000000);
  }
  std::cout << "Stop measure" << std::endl;
  Writer.StopMeasurement(1100000000);
  Writer.FinalizeMeasurement();
}

int main() {
  c_example();
  cpp_example();
  return 0;
}
