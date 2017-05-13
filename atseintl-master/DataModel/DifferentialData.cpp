//-------------------------------------------------------------------
//
//  File:        DifferentilaData.cpp
//  Created:     May 27, 2004
//  Authors:     Valentin Perov, Alexander Zagrebin
//
//  Description:
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

#include "DataModel/DifferentialData.h"

#include "Common/TseConsts.h"
#include "DBAccess/DataHandle.h"

#include <vector>

namespace tse
{
bool
DifferentialData::finalizeCarrierAndFareMarketLists(const CarrierCode& cr)
{
  if (!_carrier.size() || !_fareMarket.size() || (_carrier.size() != _fareMarket.size()))
    return false;
  std::vector<CarrierCode>::iterator i = _carrier.begin();
  std::vector<FareMarket*>::iterator j = _fareMarket.begin();
  while (i != _carrier.end())
  {
    if (*i == cr)
    {
      ++i;
      ++j;
    }
    else
    {
      i = _carrier.erase(i);
      j = _fareMarket.erase(j);
    }
  }
  return true;
}

void
DifferentialData::swap()
{
  const Loc* beDest = _origin;

  _origin = _destination;
  _destination = beDest;
  _setSwap = true;

  return;
}

DifferentialData*
DifferentialData::clone(DataHandle& dataHandle) const
{
  DifferentialData* dd = nullptr;
  dataHandle.get(dd);
  *dd = *this;
  return dd;
}
} // tse namespace
