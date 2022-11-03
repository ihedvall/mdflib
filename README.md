# Library MDF

## Summary

The MDF library repository implements an interface against the ASAM Measurement Data File (MDF). 
It supports reading of MDF files version 3 and 4. The writing support is not finalized.  

Release 1.0 of the project support reading of MDF files up to version 4.1 and some 4.2 version features. 
Writing is supported for version 3. Version 4 is under development but is currently on low priority as 
it is not required of other projects.

The project support up to MDF version 4.1. Support for version 4.2 is limited. 

The main project is a C++ static library and its used applications.
- **MDF Viewer**. Simple GUI application that list the contents of an MDF file. 
- **MDF .NET (C++/CLI) Assembly**. Basic reading of MDF files for .NET friends. 
- **MDF gRPC Server**. Microservice for reading and writing of MDF files. On hold.
- **MDF Python Library**. Basic reading and writing for Python friends. On hold.

The library and its applications, compiles/runs on Windows, Linux and Mac operating system. 
The .NET library only runs on Windows operating system.

Note that external projects currently only needing read support. This means that the writing support
is on low priority. Please let me know if write support is required.

## Building

The project uses CMAKE for building. The following third-party libraries are used and
needs to be downloaded and pre-built. Note that the .NET library requires Visual Studio 
and a

- ZLIB Library. Set the 'ZLIB_ROOT' variable to the ZLIB root path.
- WxWidgets Library. Required if the GUI applications should be built. 
- Doxygen's application. Is required if the documentation should be built.
- Google Test Library. Is required for running and build the unit tests.

## License

The project uses the MIT license. See external LICENSE file in project root.

