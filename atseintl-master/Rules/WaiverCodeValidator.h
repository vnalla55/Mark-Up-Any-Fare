//-------------------------------------------------------------------
//
//  File:        WaiverCodeValidator.h
//  Created:     June 17, 2009
//
//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "Common/LoggerPtr.h"
#include "Common/TseCodeTypes.h"

#include <string>
#include <vector>

namespace tse
{
class Waiver;

class DataHandle;
class DiagCollector;
class DateTime;

class WaiverCodeValidator
{
public:
  WaiverCodeValidator(DataHandle& dataHandle,
                      DiagCollector* dc,
                      log4cxx::LoggerPtr logger,
                      const std::string& label)
    : _dataHandle(dataHandle), _dc(dc), _logger(logger), _label(label)
  {
  }

  virtual ~WaiverCodeValidator() {}

  bool validate(uint32_t itemNoR3,
                const VendorCode& vendor,
                int itemNo,
                const DateTime& applDate,
                const std::string& waiverCode);

protected:
  virtual void getWaiver(const VendorCode& vendor, int itemNo, const DateTime& applDate);

  bool match(const VendorCode& vendor,
             int itemNo,
             const DateTime& applDate,
             const std::string& waiverCode);

  DataHandle& _dataHandle;
  DiagCollector* _dc;
  log4cxx::LoggerPtr _logger;
  const std::string& _label;

  std::vector<Waiver*> _waivers;

private:
  WaiverCodeValidator(const WaiverCodeValidator&);
  WaiverCodeValidator& operator=(const WaiverCodeValidator&);

  friend class WaiverCodeValidatorTest;
};
}
