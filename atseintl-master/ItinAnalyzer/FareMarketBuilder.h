//----------------------------------------------------------------------------
//  Copyright Sabre 2006
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

#pragma once

#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"


#include <set>
#include <vector>

namespace tse
{
class PricingTrx;
class FareMarket;
class Itin;

/**
 * @class FareMarketBuilder
 *
 * This class is to build all fare markets for an itin.
 *
 */

namespace FareMarketBuilder
{
  // will move code from ItinAnalyzerService to here...
void
setBreakIndicator(FareMarket* fareMarket, Itin* itin, PricingTrx& trx);
void
setBreakIndByCxrOverride(const std::vector<CarrierCode>& govCxrOverrides,
                         const std::set<CarrierCode>& participatingCarriers,
                         FareMarket& fareMarket,
                         bool yyOverride);

bool
setFareCalcAmtForDummyFareFM(const PricingTrx& trx, const Itin* const& itin, FareMarket& fm);
}

class CollectStopOverTravelSeg : public std::unary_function<FareMarket, void>
{
public:
  CollectStopOverTravelSeg(const Itin& itin) : _itin(itin) {}
  void operator()(FareMarket* fm);

private:
  const Itin& _itin;
};
}
