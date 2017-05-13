//-------------------------------------------------------------------
//
//  File:        BrandResponseItem.h
//  Created:     Feb 03, 2008
//  Authors:     Mauricio Dantas
//
//  Description: Members and methods for Brand response XML content
//
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
//-------------------------------------------------------------------
#pragma once

#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"

#include <string>

namespace tse
{

struct BrandResponseItem
{
  std::string _campaignCode;
  BrandCode _brandCode;
  std::string _brandName;
  std::string _brandText;
  std::vector<BookingCode> _bookingCodeVec;
  std::vector<std::string> _includedFClassVec;
  std::vector<std::string> _excludedFClassVec;

  BrandResponseItem() { clear(); }

  void clear();
};
typedef std::vector<BrandResponseItem*> BrandResponseItemVec;

} // end namespace tse
// msd
