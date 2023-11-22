/*
* Copyright 2022 Ingemar Hedvall
* SPDX-License-Identifier: MIT
*/

#include "mdfbuslogger.h"

namespace mdf::detail {
MdfBusLogger::~MdfBusLogger() { StopWorkThread();}
}  // namespace mdf