# Library MDF version 2.0

## Summary

The MDF library repository implements an interface against the ASAM Measurement Data File (MDF). 
It supports reading and writing of MDF files version 3 and 4. 
The release 2.0 of the project support reading and writing of MDF files up to version 4.2. 

- **MDF lib** is a C++ static library.
- **MDF lib test** is a C++ unit tests application for mdflib.
- **MDF library** is a C++ shared library, and it contains .NET (C++/CLI) Assembly(need to be built with msvc>=19.36).
- **MDF library example** is a C++ application that use MDF library as dll/so.
- **MDF library test net** is a C# unit tests application that use the .NET Assembly of MDF library.
- **MDF Viewer**. Simple GUI application that list the contents of an MDF file.
- **MDF gRPC Server**. Microservice for reading and writing of MDF files. On request.
- **MDF Python Library**. Basic reading and writing for Python friends. On request or use the ASAMMDF library.

The library and its applications, compiles/runs on Windows, Linux and Mac operating system.

## Installation

[Windows Install (exe)](https://github.com/ihedvall/mdflib/releases/download/v2.0.%2C0/mdflib.exe)

## Building

The project uses CMAKE for building. But mdflibrary_test_net is a Visual Studio build.

The following third-party libraries are used and needs to be downloaded and pre-built.

- ZLIB Library. Set the 'ZLIB_ROOT' variable to the ZLIB root path.
- EXPAT Library. Set the 'EXPAT_ROOT' variable to the ZLIB root path.
- Boost Library. Required if the GUI applications should be built.
- WxWidgets Library. Required if the GUI applications should be built.
- Google Test Library. Is required for running and build the unit tests.
- Doxygen's application. Is required if the documentation should be built.

Also, you can use [vcpkg](https://github.com/microsoft/vcpkg) to import the dependencies.

## License

The project uses the MIT license. See external LICENSE file in project root.

