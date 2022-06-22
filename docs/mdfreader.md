---
layout: default
title: MDF Reader
---
# MDF Reader

The MDF reader simply read and parse a Measurement Data File (MDF). The file format is
specified by the ASAM standard organisation and an overview of the file format can be found
here: [ASAM MDF Wiki](https://www.asam.net/standards/detail/mdf/wiki/).

The reader object handles all versions of MDF. The object is either created through the MDFFactory class or simply by 
declaring the reader. Example of usage below:

~~~~c++
#include <mdf/mdfreader.h>
#using namespace mdf;

MdfReader reader("c:/mdf/example.mf4");  // Note the file is now open.

// Read all blocks but not the raw data and attachments.
// This reads in the block information into memory.
reader.ReadEverythingButData();          
                                         
const auto* mdf_file = reader.GetFile(); // Get the file interface.
DataGroupList dg_list;                   // Get all measurements.
mdf_file.DataGroups(dg_list);            

// In this example, we read in all sample data and fetch all values.
for (const auto* dg4 : dg_list) {
  // Subscribers holds the sample data for a channel.
  // You should normally only subscribe on some channels. 
  // We need a list to hold them.
  ChannelObserverList subscriber_list;       
  const auto cg_list = dg4->ChannelGroups(); 
  for (const auto* cg4 : cg_list ) {
    const auto cn_list = cg4->Channels(); 
    for (const auto* cn4 : cn_list) {
      // Create a subscriber and add it to the temporary list
      auto sub = reader.CreateChannelObserver(*dg4, *cg4, *cn4);
      subscriber_list.push_back(std::move(sub));
    }
  }
  
  // Now it is time to read in all samples 
  reader.ReadData(*dg4); // Read raw data from file
  double channel_value = 0.0; Channel value (no scaling)
  double eng_value = 0.0; // Engineering value
  for (auto& obs : subscriber_list) {
    for (size_t sample = 0; sample < obs->NofSamples(); ++sample) {
      const auto channel_valid = obs->GetChannelValue(csample, hannel_value);
      const auto eng_valid = sub->GetEngValue(sample, eng_value);
      // You should do something with data here
    }
  }
  
  // Not needed in this example as we delete the subscribers,
  // but it is good practise to remove samples data from memory
  // when it is no longer needed.
  dg4->ResetSample();   
}
reader.Close(); // Close the file
~~~~

## Reading from the File

When the reader object is created, the file is open and normally the application 
should do an initial read. Reading from disc may take some time. Therefor the 
user may choose from 3 types of initial reads.

- **ReadHeader()**. This function reads in the basic header and basic file information as file history and attachments. This
read is fast but no information about the measurements are read.
- **ReadMeasurementInfo()**. The function do the above and also the measurement (data groups) and its 
sub-measurements (channel groups), but no information about the channels.
- **ReadEverythingButData()**. Does the above but reads in everything but not raw sample and attachment data.
This function is the normal to use as you will get all channel and event information. 

After the initial read the file may be closed and opened later when reading sample or attachment data.

## Attachment Data

Embedded attachment data can be extracted by the ExportAttachmentData(). Supply the attachment object and
a destination file name.

## Reading Sample Data

Reading sample data is somewhat more complicated as it both time and memory consuming. Therefore, the 
user needs to select which channels are needed. This is done by creating a data subscriber for each 
channel and the call the ReadData() function for each measurement (data group).