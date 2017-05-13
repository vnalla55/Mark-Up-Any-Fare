//----------------------------------------------------------------------------
//  File:        WnSnapFCUtil.h
//  Created:     2011-01-05
//
//  Description: Airline Shopping Fare Calc utility class
//
//  Updates:
//
//  Copyright Sabre 2011
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

#include "Common/TseStringTypes.h"
#include "Common/WnSnapUtil.h"
#include "DataModel/FarePath.h"
#include "Taxes/Common/AbstractTaxSplitter.h"

namespace tse
{
class Itin;
class PricingTrx;

class AirlineShoppingFCUtil
{
public:
  static void copyTaxResponsesAndSwapItinsVec(PricingTrx& trx);

  static void updateTaxResponse(PricingTrx& trx, AbstractTaxSplitter& taxSplitter);

private:
  static void updateProcessedItins(Itin* itin, std::vector<Itin*>& processedItins);

  static bool
  farePathAlreadyProcessed(Itin* itin,
                           FarePath::FarePathKey& farePathKey,
                           std::set<std::pair<Itin*, FarePath::FarePathKey> >& processedFarePaths);

  static bool
  baseItinContainsFarePath(PricingTrx& trx, Itin* itin, FarePath::FarePathKey& farePathKey);
};

} // end namespace tse

