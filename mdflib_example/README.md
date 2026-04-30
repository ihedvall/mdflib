DataWriter example

Build this example with CMake by enabling `-DMDF_BUILD_EXAMPLES=ON` at configure time.

Notes:
- `IDataWriter::ChannelLayouts()` intentionally excludes the Master channel.
	The master (Time) value is normally calculated and written during
	`MdfWriter::SaveSample()`; bulk writers should not write the master field
	unless they explicitly disable master-time calculation on the channel.

This example writes a small MF4 file `datawriter_example.mf4` in the current
working directory and performs a quick read-back to sanity-check the write.
