---
layout: default
title: MDF Writer
---

# MDF Writer

The MDF Writer object creates Measurement Data Files. The file format is
specified by the ASAM standard organisation and an overview of the file format can be found
here: [ASAM MDF Wiki](https://www.asam.net/standards/detail/mdf/wiki/).

Version 3 of the MDF file, had a simpler design than the version 4. Therefore, it exists several types of
writer. The following writers exists.

- **BasicMdf3Writer**. This writer creates an MDF file according to the version 3 of the standard (Original Bosch). It is 
useful when producing files with just measurement data and most external tools can handle this format.
- **BasicMdf4Writer**. Produces an MDF file according to version 4. The user may add both metadata information and 
file attachments. This interface is flexible but the user need to understand the MDF file format.

Note that bus streamers that uses A2L or DBC file for the configuration can be made upon request. Also gRPC 
microservices are possible to do upon request.

## Creating a File

Creating an MDF file require some knowledge of how the file is formatted and which blocks exist. 

First create the preferred type of writer. Then initialize the file by assigning the writer object a file name. 
If the file exist, it read in the existing blocks in the file, so it is possible to append the file.

Assuming the file is created, the writer file object now consist of an ID and HD (header) block. The user shall
now add at least a file history block, the metadata and configure the active measurement with channels and channel 
groups. Note that this requires some knowledge about the MDF file structure,

The file is now prepare for receiving samples. Set the pre-trig time and initialize the measurement. Now start
adding samples. This is done by setting engineering values to each channel and then adding the channel group
to the save sample function. 

Note that nothing is stored into a file yet. The internal cache stores all samples in an internal cache which size 
is set by the pre-trig time. The user now select to start the measurement and later stopping the measurement. 
Note that between initializing and stopping the measurement, only data samples are stored onto the disc.

After stopping the measurement, the user may add some more information as events and attachments. The 
measurement is finished by calling finalize measurement function.