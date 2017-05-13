//-------------------------------------------------------------------
//
//  File: FDAddOnGroupingStrategy.cpp
//  Author:Abu
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/FDAddOnGroupingStrategy.h"

#include "Common/Logger.h"
#include "DataModel/FareDisplayResponse.h"

#include <algorithm>
#include <map>
#include <set>
#include <vector>

namespace tse
{
static Logger
logger("atseintl.FDAddOnGroupingStrategy");

FDAddOnGroupingStrategy::FDAddOnGroupingStrategy(std::vector<FDAddOnFareInfo*>& addonFares)
  : _addonFares(addonFares)
{
  _globalGroup.insert(std::make_pair(GlobalDirection::AT, 1));
  _globalGroup.insert(std::make_pair(GlobalDirection::SA, 2));
  _globalGroup.insert(std::make_pair(GlobalDirection::PA, 3));
  _globalGroup.insert(std::make_pair(GlobalDirection::PN, 4));
  _globalGroup.insert(std::make_pair(GlobalDirection::FE, 5));
  _globalGroup.insert(std::make_pair(GlobalDirection::RU, 6));
  _globalGroup.insert(std::make_pair(GlobalDirection::TS, 7));
  _globalGroup.insert(std::make_pair(GlobalDirection::EH, 8));
  _globalGroup.insert(std::make_pair(GlobalDirection::AP, 9));
  _globalGroup.insert(std::make_pair(GlobalDirection::WH, 10));
  _globalGroup.insert(std::make_pair(GlobalDirection::ZZ, 12));
}

bool
FDAddOnGroupingStrategy::apply()
{
  std::sort(_addonFares.begin(), _addonFares.end(), LessByGlobal(_globalGroup));
  return true;
}
}
