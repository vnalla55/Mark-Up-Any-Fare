// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include <boost/ptr_container/ptr_vector.hpp>
#include "Common/LocZone.h"
#include "DataModel/Services/SubCache.h"

namespace tax
{

struct ServiceFeeSecurityItem
{
  ServiceFeeSecurityItem()
    : travelAgencyIndicator(type::TravelAgencyIndicator::Blank),
      carrierGdsCode(""),
      dutyFunctionCode(""),
      location(),
      codeType(type::CodeType::Blank),
      code(""),
      viewBookTktInd(type::ViewBookTktInd::ViewBookTkt)
  {
  }

  type::TravelAgencyIndicator travelAgencyIndicator;
  type::CarrierGdsCode carrierGdsCode;
  type::DutyFunctionCode dutyFunctionCode;
  LocZone location;
  type::CodeType codeType;
  std::string code;
  type::ViewBookTktInd viewBookTktInd;
};

typedef boost::ptr_vector<ServiceFeeSecurityItem> ServiceFeeSecurityItems;
typedef CacheItem<ServiceFeeSecurityItem> ServiceFeeSecurity;

}

