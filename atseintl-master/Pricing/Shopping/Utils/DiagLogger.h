
//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "DataModel/Trx.h"
#include "Diagnostic/DiagManager.h"
#include "Pricing/Shopping/Utils/BaseLogger.h"

#include <iostream>

namespace tse
{

namespace utils
{

class DiagLogger : public BaseLogger
{
public:
  DiagLogger(Trx& trx,
             DiagnosticTypes diagType,
             bool onDetailsOnly = true,
             LOGGER_LEVEL level = LOGGER_LEVEL::INFO)
    : _trx(trx), _diagType(diagType), _onDetailsOnly(onDetailsOnly), _level(level)
  {
  }

  bool getOnDetailsOnly() const { return _onDetailsOnly; }

  LOGGER_LEVEL getLevel() const { return _level; }

  void setName(const std::string& name) { _name = name; }

  std::string getName() const { return _name; }

  void message(LOGGER_LEVEL level, const std::string& msg) override
  {
    if (enabled(level))
    {
      DiagManager diagManager(_trx, _diagType);
      if (!_name.empty())
      {
        diagManager << _name << ": ";
      }
      diagManager << msg << "\n";
    }
  }

  // Enabled only if level >= logger's level
  // Enabled only if diagnostic is active.
  // In addition, if onDetailsOnly is set, only
  // enabled on DD=DETAILS for diagnostic.
  bool enabled(LOGGER_LEVEL level) const override
  {
    if (level < _level)
    {
      return false;
    }

    DiagManager diagManager(_trx, _diagType);
    if (!diagManager.isActive())
    {
      return false;
    }
    if (getOnDetailsOnly() && (!areDiagDetailsEnabled()))
    {
      return false;
    }
    return true;
  }

private:
  bool areDiagDetailsEnabled() const
  {
    return _trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "DETAILS";
  }

  Trx& _trx;
  DiagnosticTypes _diagType;

  // If onDetailsOnly flag is set, logs are
  // emitted to a diagnostic only if DD=DETAILS is set
  bool _onDetailsOnly = true;
  std::string _name;
  LOGGER_LEVEL _level = LOGGER_LEVEL::INFO;
};

} // namespace utils

} // namespace tse

