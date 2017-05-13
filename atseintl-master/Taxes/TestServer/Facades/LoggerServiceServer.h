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

#include "AtpcoTaxes/ServiceInterfaces/LoggerService.h"

namespace tax
{

class LoggerServiceServer : public LoggerService
{
public:
  LoggerServiceServer() {}
  ~LoggerServiceServer() override {}

  void log_INFO(const char* message) const override;
  void log_TRACE(const char* message) const override;
  void log_DEBUG(const char* message) const override;
  void log_WARN(const char* message) const override;
  void log_ERROR(const char* message) const override;
};
}
