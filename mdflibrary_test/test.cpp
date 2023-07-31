#include <mdflibrary/MdfExport.h>

#include <iostream>

int main() {
  auto* Writer = MdfWriterInit(mdf::MdfWriterType::Mdf4Basic, "test.mf4");
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
  MdfSourceInformationSetType(si, mdf::SourceType::Bus);
  MdfSourceInformationSetBus(si, mdf::BusType::Can);

  {
    auto* cn = MdfChannelGroupCreateChannel(cg);
    std::cout << "Channel: " << cn << std::endl;
    MdfChannelSetName(cn, "Time");
    MdfChannelSetDescription(cn, "Time channel");
    MdfChannelSetType(cn, mdf::ChannelType::Master);
    MdfChannelSetSync(cn, mdf::ChannelSyncType::Time);
    MdfChannelSetDataType(cn, mdf::ChannelDataType::FloatLe);
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

  MdfWriterInitMeasurement(Writer);
  MdfWriterStartMeasurement(Writer, 100000000);

  mdf::IChannel** pChannel = nullptr;
  size_t size = MdfChannelGroupGetChannels(cg, pChannel);
  pChannel = new mdf::IChannel*[size];
  std::cout << "ChannelGroupGetChannels: " << size << std::endl;
  size = MdfChannelGroupGetChannels(cg, pChannel);
  std::cout << "Channel0: " << pChannel[0] << std::endl;

  for (size_t i = 0; i < 50; i++) {
    MdfChannelSetChannelValueAsFloat(pChannel[1], i * 2);
    MdfChannelSetChannelValueAsFloat(pChannel[2], i * 3);
    MdfChannelSetChannelValueAsFloat(pChannel[3], i * 4);
    MdfChannelSetChannelValueAsFloat(pChannel[4], i * 5);
  }
  MdfWriterStopMeasurement(Writer, 1100000000);
  MdfWriterFinalizeMeasurement(Writer);
}