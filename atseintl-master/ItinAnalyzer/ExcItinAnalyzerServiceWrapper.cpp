///-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "ItinAnalyzer/ExcItinAnalyzerServiceWrapper.h"

#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/Itin.h"
#include "ItinAnalyzer/FareMarketBuilder.h"
#include "ItinAnalyzer/ItinAnalyzerService.h"

namespace tse
{
ExcItinAnalyzerServiceWrapper::ExcItinAnalyzerServiceWrapper(ItinAnalyzerService& itinAnalyzer)
  : ItinAnalyzerServiceWrapper(itinAnalyzer)
{
}

bool
ExcItinAnalyzerServiceWrapper::buildFareMarket(PricingTrx& trx, Itin& itin)
{
  std::vector<FareMarket*>::const_iterator fmIter = itin.fareMarket().begin();

  for (; fmIter != itin.fareMarket().end(); ++fmIter)
  {
    FareMarketBuilder::setFareCalcAmtForDummyFareFM(trx, &itin, **fmIter);
  }

  struct ItinAnalyzerService::IsPartButNotWholeDummyFare isPartButNotWholeDummyFare(
      (static_cast<ExchangePricingTrx&>(trx)).exchangeOverrides().dummyFCSegs());

  itin.fareMarket().erase(std::remove_if(itin.fareMarket().begin(),
                                         itin.fareMarket().end(),
                                         isPartButNotWholeDummyFare),
                          itin.fareMarket().end());

  return true;
}
}
