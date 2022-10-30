using namespace System;
using namespace Reflection;
using namespace Runtime::CompilerServices;
using namespace Runtime::InteropServices;
using namespace Security::Permissions;

[assembly:AssemblyTitleAttribute(L"MdfLibrary")];
[assembly:AssemblyDescriptionAttribute(
  L"Simple .NET assembly that reads and parses MDF files.")];
[assembly:AssemblyConfigurationAttribute(L"")];
[assembly:AssemblyCompanyAttribute(L"Open Source")];
[assembly:AssemblyProductAttribute(L"MdfLibrary")];
[assembly:AssemblyCopyrightAttribute(L"Copyright (c) 2022 Ingemar Hedvall")];
[assembly:AssemblyTrademarkAttribute(L"")];
[assembly:AssemblyCultureAttribute(L"")];

[assembly:AssemblyVersionAttribute(L"1.0.*")];

[assembly:ComVisible(false)];
