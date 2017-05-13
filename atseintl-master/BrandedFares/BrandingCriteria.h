//-------------------------------------------------------------------
//
//  File:        BrandingCriteria.h
//  Created:     March 2013
//  Authors:
//
//  Description:
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

#include "Common/DateTime.h"
#include "Common/TseStringTypes.h"

#include <boost/noncopyable.hpp>

#include <vector>

namespace tse
{
class MarketRequest;

class BrandingCriteria : boost::noncopyable
{
public:
  std::vector<MarketRequest*>& marketRequest() { return _marketRequest; }
  const std::vector<MarketRequest*>& marketRequest() const { return _marketRequest; }

  std::vector<std::string>& accountCodes() { return _accountCodes; }
  const std::vector<std::string>& accountCodes() const { return _accountCodes; }

  // need to pass in YYYY-MM-DD format
  DateTime& salesDate() { return _salesDate; }
  const DateTime& salesDate() const { return _salesDate; }

private:
  std::vector<MarketRequest*> _marketRequest;
  std::vector<std::string> _accountCodes;
  DateTime _salesDate;
};
} // tse

