# Library MDF

## Summary

The MDF Library implements an interface against ASAM Measurement Data Files (MDF). 
It supports reading and writing of MDF files version 3 and 4. 

Release 1.0 of the project support reading of MDF files up to version 4.1 and some 4.2 version features. 
Writing is supported for version 3. Version 4 is under development but is currently on low priority as 
it is not required of other projects.

The project is under development and support up to MDF version 4.1. Working on support for version 4.2 and
writing of MDF 4 files. 

The main project is a C++ static library and its dependent applications.
- **MDF Viewer**. Simple GUI application that list the contents of an MDF file. 
- **MDF .NET (C++/CLI) Assembly**. Basic reading and writing of MDF files for .NET friends. On hold.
- **MDF gRPC Server**. Microservice for reading and writing of MDF files. Under development.
- **MDF Python Library**. Basic reading and writing for Python friends. On hold.

Note that external projects currently only need reading support. This means that the writing support
is on low priority. Please let me know if write support is required.

## Building

The project uses CMAKE for building. The following third-party libraries are used and
needs to be downloaded and pre-built.

- Util Library. Generic library that contains common functionality as logging.
- Boost Library. Set the 'Boost_ROOT' variable to the Boost root path.
- Expat Library. Set the 'EXPAT_ROOT' variable to the expat root path.
- OpenSSL Library. Set the 'OPENSSL_ROOT' variable to the OpenSSL root path.
- ZLIB Library. Set the 'ZLIB_ROOT' variable to the ZLIB root path.
- WxWidgets Library. Required if the GUI applications should be built. 
- Doxygen's application. Is required if the documentation should be built.
- Google Test Library. Is required for running and build the unit tests.

## License

The project uses the MIT license. See external LICENSE file in project root.

