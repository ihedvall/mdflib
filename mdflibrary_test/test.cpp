#include <mdflibrary/MdfExport.h>
int main() {
  auto* Writer = MdfWriterInit(mdf::MdfWriterType::Mdf4Basic, "test.mf4");
  auto* Header = MdfWriterGetHeader(Writer);
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
  auto* cg = MdfDataGroupCreateChannelGroup(dg);
  MdfChannelGroupSetName(cg, "Test");
  MdfChannelGroupSetDescription(cg, "Test channel group");

  auto* si = MdfChannelGroupCreateSourceInformation(cg);
  MdfSourceInformationSetName(si, "SI-Name");
  MdfSourceInformationSetPath(si, "SI-Path");
  MdfSourceInformationSetDescription(si, "SI-Desc");
  MdfSourceInformationSetType(si, mdf::SourceType::Bus);
  MdfSourceInformationSetBus(si, mdf::BusType::Can);

  {
    auto* cn = MdfChannelGroupCreateChannel(cg);
    MdfChannelSetName(cn, "Time");
    MdfChannelSetDescription(cn, "Time channel");
    MdfChannelSetType(cn, mdf::ChannelType::Master);
    MdfChannelSetSync(cn, mdf::ChannelSyncType::Time);
    MdfChannelSetDataType(cn, mdf::ChannelDataType::FloatLe);
    MdfChannelSetDataBytes(cn, 4);
    MdfChannelSetUnit(cn, "s");
    MdfChannelSetRange(cn, 0, 100);
  }
  MdfWriterInitMeasurement(Writer);
  MdfWriterStartMeasurement(Writer, 1000000);
  for (size_t i = 0; i < 50; i++) {
    mdf::IChannel** pChannel;
    MdfChannelGroupGetChannels(cg, pChannel);
    MdfChannelSetChannelValueAsFloat(pChannel[0], i);
  }
  MdfWriterStopMeasurement(Writer, 1000000);
  MdfWriterFinalizeMeasurement(Writer);
}