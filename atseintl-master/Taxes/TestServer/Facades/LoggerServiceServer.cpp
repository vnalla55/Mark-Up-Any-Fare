// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "LoggerServiceServer.h"

#include <iostream>

namespace tax
{

void
LoggerServiceServer::log_INFO(const char* message) const
{
  std::cerr << "INFO: " << message << std::endl;
}

void
LoggerServiceServer::log_TRACE(const char* message) const
{
  std::cerr << "TRACE: " << message << std::endl;
}

void
LoggerServiceServer::log_DEBUG(const char* message) const
{
  std::cerr << "DEBUG: " << message << std::endl;
}

void
LoggerServiceServer::log_WARN(const char* message) const
{
  std::cerr << "WARN: " << message << std::endl;
}

void
LoggerServiceServer::log_ERROR(const char* message) const
{
  std::cerr << "ERROR: " << message << std::endl;
}

} // namespace tax
