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

#pragma once

namespace tax
{

class LoggerService
{
public:
  LoggerService() {}
  virtual ~LoggerService() {}

  virtual void log_INFO(const char* message) const = 0;
  virtual void log_TRACE(const char* message) const = 0;
  virtual void log_DEBUG(const char* message) const = 0;
  virtual void log_WARN(const char* message) const = 0;
  virtual void log_ERROR(const char* message) const = 0;
};
}

