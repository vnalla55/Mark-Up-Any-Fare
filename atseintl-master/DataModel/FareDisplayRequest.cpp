//-------------------------------------------------------------------
//
//  File:        FareDisplayRequest.cpp
//  Created:
//  Authors:
//
//  Description:
//
//  Updates:
//          02/15/05 - Mike Carroll - file created.
//
//  Copyright Sabre 2004
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

#include "DataModel/FareDisplayRequest.h"

#include "FareDisplay/FDConsts.h"
#include "FareDisplay/InclusionCodeConsts.h"

namespace tse
{
const FDRequestType FARE_DISPLAY_REQUEST = "FQ";
const FDRequestType FARE_RULES_REQUEST = "RD";
const FDRequestType FARE_TAX_REQUEST = "FT";
const FDRequestType FARE_MILEAGE_REQUEST = "MP";
const FDRequestType FARE_BOOKINGCODE_REQUEST = "RB";
const FDRequestType TAX_CODE_REQUEST = "TX";
const FDRequestType PFC_REQUEST = "PX";
const FDRequestType ENHANCED_RD_REQUEST = "ER";

const FDOutputType FD_OUTPUT_SDS = "SDS";

bool
FareDisplayRequest::isPaxTypeRequested()
{
  return ((!_displayPassengerTypes.empty() || _requestedInclusionCode == ALL_INCLUSION_CODE) &&
          _requestedInclusionCode != TRAVELOCITY_INCL);
}

uint8_t
FareDisplayRequest::inclusionNumber(InclusionCode& inclCode) const
{
  if(inclCode == PREMIUM_FIRST_CLASS_FARES)
    return PB_PREM_FIRST;
  if(inclCode == FIRST_CLASS_FARES)
    return FB_FIRST;
  if(inclCode == PREMIUM_BUSINESS_CLASS_FARES)
    return JB_PREM_BUSINESS;
  if(inclCode == BUSINESS_CLASS_FARES)
    return BB_BUSINESS;
  if(inclCode == PREMIUM_ECONOMY_CLASS_FARES)
    return SB_PREM_ECONOMY;
  if(inclCode == ECONOMY_CLASS_FARES)
    return YB_ECONOMY;
  if(inclCode == ALL_CLASS_FARES)
    return AB_ALL;
  else
    return 0;
}

std::string
FareDisplayRequest::inclusionVerbiage(uint8_t inclusionNumber)
{
  std::string name = "";
  switch(inclusionNumber)
  {
    case PB_PREM_FIRST:
      name = "PREMIUM FIRST CABIN";
      break;
    case FB_FIRST:
      name = "FIRST CABIN";
      break;
    case JB_PREM_BUSINESS:
      name = "PREMIUM BUSINESS CABIN";
      break;
    case BB_BUSINESS:
      name = "BUSINESS CABIN";
      break;
    case SB_PREM_ECONOMY:
      name = "PREMIUM ECONOMY CABIN";
      break;
    case YB_ECONOMY:
      name = "ECONOMY CABIN";
      break;
    default:
      name = "INVALID CABIN";
      break;
  }
  return name;
}
}
