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

{% include_relative mdfreaderexample.cpp %}
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
