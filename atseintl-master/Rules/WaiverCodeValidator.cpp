//-------------------------------------------------------------------
//
//  File:        WaiverCodeValidator.cpp
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

#include "Rules/WaiverCodeValidator.h"

#include "DBAccess/DataHandle.h"
#include "DBAccess/Waiver.h"
#include "Diagnostic/DiagCollector.h"
#include "Util/BranchPrediction.h"

namespace tse
{

void
WaiverCodeValidator::getWaiver(const VendorCode& vendor, int itemNo, const DateTime& applDate)
{
  _waivers = _dataHandle.getWaiver(vendor, itemNo, _dataHandle.ticketDate(), applDate);
}

bool
WaiverCodeValidator::validate(uint32_t itemNoR3,
                              const VendorCode& vendor,
                              int itemNo,
                              const DateTime& applDate,
                              const std::string& waiverCode)
{
  bool result = match(vendor, itemNo, applDate, waiverCode);

  if (LIKELY(!_dc))
    return result;

  if (waiverCode.empty())
    getWaiver(vendor, itemNo, applDate);

  typedef std::vector<Waiver*>::const_iterator It;

  *_dc << "WAIVER CODES: ";
  for (It i = _waivers.begin(); i != _waivers.end(); i++)
    *_dc << (*i)->waiver() << " ";
  *_dc << " " << _label << " WAIVER CODE: " << waiverCode << "\n";

  if (!result)
    *_dc << "  FAILED ITEM " << itemNoR3 << " - WAIVER CODE NOT MATCHED\n";

  return result;
}

namespace
{

class WaiverCodeEquals
{
public:
  WaiverCodeEquals(const std::string& waiverCode)
    : _waiverCode(static_cast<Waiver::WaiverCode>(std::atoi(waiverCode.c_str())))
  {
  }
  bool operator()(const Waiver* w) const { return w->waiver() == _waiverCode; }

protected:
  Waiver::WaiverCode _waiverCode;
};
}

bool
WaiverCodeValidator::match(const VendorCode& vendor,
                           int itemNo,
                           const DateTime& applDate,
                           const std::string& waiverCode)
{
  if (itemNo == 0)
    return true;
  if (waiverCode.empty())
    return false;

  getWaiver(vendor, itemNo, applDate);

  if (_waivers.empty())
  {
    LOG4CXX_ERROR(_logger, "Waiver table item No. " << itemNo << " not found");
    return false;
  }

  return std::find_if(_waivers.begin(), _waivers.end(), WaiverCodeEquals(waiverCode)) !=
         _waivers.end();
}
}
