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

#include "Taxes/AtpcoTaxes/ServiceInterfaces/LoggerService.h"

namespace tse
{

class LoggerServiceV2 : public tax::LoggerService
{
public:
  LoggerServiceV2() {}
  LoggerServiceV2(const LoggerServiceV2&) = delete;
  LoggerServiceV2& operator=(const LoggerServiceV2&) = delete;
  ~LoggerServiceV2() override {}

  void log_INFO(const char* message) const override;
  void log_TRACE(const char* message) const override;
  void log_DEBUG(const char* message) const override;
  void log_WARN(const char* message) const override;
  void log_ERROR(const char* message) const override;
};

} // namespace tse

