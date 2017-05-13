//----------------------------------------------------------------------------
//
//  File:           TseEnums.C
//  Created:        2/4/2004
//  Authors:
//
//  Description:    Common enums required for ATSE shopping/pricing.
//
//  Updates:
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
//----------------------------------------------------------------------------

#include "Common/TseEnums.h"

#include "Common/ErrorResponseException.h"
#include "Common/ShoppingUtil.h"

#include <iostream>

namespace tse
{

const int GlobalDirectionItemsCnt = 34;
const char* globalDirectionItems[GlobalDirectionItemsCnt] = {
  "AF", "AL", "AP", "AT", "CA", "CT", "DI", "DO", "DU", "EH", "EM", "EU",
  "FE", "IN", "ME", "NA", "NP", "PA", "PE", "PN", "PO", "PV", "RU", "RW",
  "SA", "SN", "SP", "TB", "TS", "TT", "US", "WH", "XX", ""
};

const std::string globalDirectionItemsStr[GlobalDirectionItemsCnt] = {
  "AF", "AL", "AP", "AT", "CA", "CT", "DI", "DO", "DU", "EH", "EM", "EU",
  "FE", "IN", "ME", "NA", "NP", "PA", "PE", "PN", "PO", "PV", "RU", "RW",
  "SA", "SN", "SP", "TB", "TS", "TT", "US", "WH", "XX", ""
};

bool
globalDirectionToStr(std::string& dst, const GlobalDirection src)
{
  int gdIdx = src - 1;
  if ((gdIdx < 0) || (gdIdx >= GlobalDirectionItemsCnt))
  {
    return false;
  }

  dst = globalDirectionItems[gdIdx];

  return true;
}

const std::string*
globalDirectionToStr(const GlobalDirection src)
{
  int gdIdx = src - 1;
  if (UNLIKELY((gdIdx < 0) || (gdIdx >= GlobalDirectionItemsCnt)))
  {
    return nullptr;
  }

  return &globalDirectionItemsStr[gdIdx];
}

bool
strToGlobalDirection(GlobalDirection& dst, const std::string& src)
{
  for (int gdIdx = 0; gdIdx < GlobalDirectionItemsCnt; gdIdx++)
  {
    if (src == globalDirectionItems[gdIdx])
    {
      dst = static_cast<GlobalDirection>(gdIdx + 1);
      return true;
    }
  }

  return false;
}

void
getBrandingServiceErrorMessage(StatusBrandingService status, std::string& errorMessage)
{
  switch (status)
  {
  case StatusBrandingService::BS_UNAVAILABLE:
    errorMessage = ErrorResponseException(ErrorResponseException::BRANDING_SERVICE_UNAVAILABLE).message();
    break;
  case StatusBrandingService::BS_INVALID_RESPONSE:
    errorMessage = ErrorResponseException(ErrorResponseException::BRANDING_SERVICE_INVALID_RESPONSE).message();
    break;
  default:
    errorMessage = "";
    break;
  }
}

Continent
getContinentByAllianceContinentCode(int allianceContinentCode)
{
  switch (allianceContinentCode)
  {
  case 1:
    return CONTINENT_AFRICA;
  case 2:
    return CONTINENT_ASIA;
  case 3:
    return CONTINENT_EUROPE;
  case 4:
    return CONTINENT_EUROPE_MIDDLE_EAST;
  case 5:
    return CONTINENT_NORTH_AMERICA;
  case 6:
    return CONTINENT_SOUTH_AMERICA;
  case 7:
    return CONTINENT_SOUTH_WEST_PACIFIC;
  case 8:
    return CONTINENT_SOUTH_CENTRAL_AMERICA;
  default:
    throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED,
                                 "INVALID CONTINENT CODE");
  }
}

std::ostream& operator<<(std::ostream& out, IbfErrorMessage m)
{
  out << ShoppingUtil::getIbfErrorCode(m);
  return out;
}

std::string directionToIndicator(const Direction direction)
{
  if (direction == Direction::ORIGINAL)
    return "O";
  if (direction == Direction::REVERSED)
    return "R";
  return "B";
}

std::string directionToString(const Direction direction)
{
  if (direction == Direction::ORIGINAL)
    return "ORIGINAL";
  if (direction == Direction::REVERSED)
    return "REVERSED";
  return "BOTHWAYS";
}

std::ostream& operator<<(std::ostream& out, const Direction direction)
{
  out << directionToString(direction);
  return out;
}


} // end tse namespace
