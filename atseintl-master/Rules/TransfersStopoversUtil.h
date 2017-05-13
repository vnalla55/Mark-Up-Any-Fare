//----------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-----------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include <string>
#include "Util/BranchPrediction.h"

namespace tse
{
class Loc;
class LocKey;
class FareMarket;
class PricingTrx;

namespace TransStopUtil
{
LocCode
getCity(const PricingTrx& trx, const FareMarket* fm, const Loc& loc);

std::string
getLocOfType(const PricingTrx& trx,
             const FareMarket* fm,
             const VendorCode& vendor,
             const Loc& loc,
             const LocTypeCode& locType);

bool
isInLoc(const PricingTrx& trx,
        const VendorCode& vendor,
        const Loc& loc,
        const LocKey& containingLoc);
}

class RecurringSegContext
{
public:
  RecurringSegContext() : _ignoreCharge1(false), _ignoreCharge2(false) {}

  bool processNextSegment(Indicator chargeInd)
  {
    if (UNLIKELY(_ignoreCharge1 && chargeInd == '1'))
      return false;
    if (UNLIKELY(_ignoreCharge2 && chargeInd == '2'))
      return false;
    return true;
  }

  void setIgnoreChargeGroup(Indicator chargeInd)
  {
    _ignoreCharge1 |= (chargeInd == '1');
    _ignoreCharge2 |= (chargeInd == '2');
  }

private:
  bool _ignoreCharge1;
  bool _ignoreCharge2;
};
}

